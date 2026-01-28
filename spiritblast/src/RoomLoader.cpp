#include "Variables.h"
#include "Functions.h"

#include "RoomLoader.h"
#include <fstream>

static TRoutine room_goto_original;

void RoomGotoHook(RValue &result, CInstance *self, CInstance *other, int argCount, RValue *args)
{
	RValue rm = args[0];
	if (g_interface->CallBuiltin("is_string", {rm}).ToBoolean())
	{
		RoomLoader::GoToRoom(RoomLoader::levelName, rm.ToString());
		return;
	}

	room_goto_original(result, self, other, argCount, args);
}

void PlayCustomLevelHook(RValue &result, CInstance *self, CInstance *other, int argCount, RValue *args)
{
	string level = args[0].ToString();
	Print(RValue("Playing level " + level));

	RoomLoader::firstRoom = "room";
	RoomLoader::ClearInstanceMap();
	RoomLoader::GoToRoom(level, "room");
}

namespace RoomLoader
{
	json roomData;
	string currentRoom;
	string firstRoom;
	string levelName;

	map<string, vector<string>> instanceMap;

	string GetInstanceIDString(string obj, float x, float y)
	{
		string instString = obj + "_" + to_string(x) + "_" + to_string(y);
		return instString;
	}

	void ClearInstanceMap()
	{
		Print("Clearing room loader instance map...");

		for (auto &[key, value] : instanceMap)
		{
			value.clear();
			instanceMap.erase(key);
		}

		instanceMap.clear();
	}

	void CheckInstancesInRoomList()
	{
		if (!instanceMap.contains(currentRoom))
			return;

		RValue instance_count = RValue();
		g_interface->GetBuiltin("instance_count", nullptr, NULL_INDEX, instance_count);

		vector<string> &instances = instanceMap[currentRoom];
		for (int i = 0; i < instances.size(); i++)
		{
			bool exists = false;
			string instString = instances[i];

			// Not a good way to do this, but it'll do
			for (int j = 0; j < instance_count.ToInt32(); j++)
			{
				RValue instID = g_interface->CallBuiltin("instance_id_get", {j});
				RValue object_index = GetInstanceVariable(instID, "object_index");

				string name = g_interface->CallBuiltin("object_get_name", {object_index}).ToString();
				float x = GetInstanceVariable(instID, "x").ToDouble();
				float y = GetInstanceVariable(instID, "y").ToDouble();

				string istr = GetInstanceIDString(name, x, y);
				if (istr == instString)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				instances.erase(instances.begin() + i);
				i--;
			}
		}
	}

	map<string, RValue> CreateStageData()
	{
		RValue ob_stageManager = GetAsset("ob_stageManager");
		RValue stages = GetInstanceVariable(ob_stageManager, "stages");
		RValue baseStage = g_interface->CallBuiltin("struct_get", {stages, "cinnamonSprings"});
		RValue clonedStage = g_interface->CallBuiltin("variable_clone", {baseStage});

		// Makes a copy of the data for Cinnamon Springs
		// that will be used as the base for the custom stage
		map<string, RValue> stage = clonedStage.ToMap();

		// main info for the stage
		stage["name"] = "customStage";
		stage["initialMusic"] = GetAsset("mu_jamLayerA");
		stage["firstRoom"] = GetAsset("rm_template_room");

		// Add the custom stage data to the stages struct
		// since the game needs it to be there
		g_interface->CallBuiltin("struct_set", {stages, "customStage", stage});
		return stage;
	}

	json LoadRoomData(string path)
	{
		ifstream file(path);
		if (file.fail())
		{
			Print("Failed to load room data");
			return nullptr;
		}

		json data;
		try
		{
			data = json::parse(file, nullptr, true, true);
		}
		catch (const json::parse_error &e)
		{
			Print("Failed to parse json file!");
			Print(e.what());
			return nullptr;
		}

		file.close();
		return data;
	}

	void GoToRoom(string level, string name)
	{
		roomData = LoadRoomData("levels/" + level + "/" + name + ".rfrm");
		if (roomData != nullptr)
		{
			int rmVersion = roomData["rf_roomversion"];
			if (rmVersion != ROOM_VERSION)
			{
				Print("Incorrect room version!");
				Print(RValue("Expected " + to_string(ROOM_VERSION) + ", got " + to_string(rmVersion)));
				return;
			}

			CheckInstancesInRoomList();
			currentRoom = name;
			levelName = level;

			RValue templateRoom = GetAsset("rm_template_room");
			if (templateRoom.ToInt32() != GM_INVALID)
			{
				Print("Preparing room");
				int width = roomData["roomInfo"]["width"];
				int height = roomData["roomInfo"]["height"];

				g_interface->CallBuiltin("room_set_width", {templateRoom, RValue(width)});
				g_interface->CallBuiltin("room_set_height", {templateRoom, RValue(height)});

				Print("Go to template room");
				g_interface->CallBuiltin("room_goto", {templateRoom});
			}
		}
	}

	void InitializeRoom()
	{
		bool firstTime = false;
		if (!instanceMap.contains(currentRoom))
		{
			Print(RValue("Setting up " + currentRoom + " for the first time"));
			firstTime = true;
		}

		vector<string> &instanceList = instanceMap[currentRoom];

		// Create all layers and objects for the room
		vector<json> layers = roomData["layers"];
		for (int i = 0; i < layers.size(); i++)
		{
			json lay = layers[i];
			string layName = lay["name"];
			int layDepth = lay["depth"];

			if (!g_interface->CallBuiltin("layer_exists", {RValue(layName)}).ToBoolean())
				g_interface->CallBuiltin("layer_create", {layDepth, RValue(layName)});

			RValue layID = g_interface->CallBuiltin("layer_get_id", {RValue(layName)});
			vector<json> instances = lay["instances"];
			for (int j = 0; j < instances.size(); j++)
			{
				json instdata = instances[j];

				string id = instdata["id"];
				float x = instdata["x"];
				float y = instdata["y"];
				float xscale = instdata["xscale"];
				float yscale = instdata["yscale"];

				RValue obj = GetAsset(id);
				if (obj.ToInt32() == GM_INVALID)
					continue;

				RValue inst = g_interface->CallBuiltin("instance_create_layer", {x, y, layID, obj});
				string instString = GetInstanceIDString(id, x, y);

				if (firstTime)
					instanceList.push_back(instString);
				else
				{
					bool destroy = true;
					// find the instance and destroy it if it doesn't exist
					for (int k = 0; k < instanceList.size(); k++)
					{
						string lstr = instanceList[k];
						if (instString == lstr)
						{
							destroy = false;
							break;
						}
					}

					if (destroy)
					{
						g_interface->CallBuiltin("instance_destroy", {inst, false});
						continue;
					}
				}

				SetInstanceVariable(inst, "image_xscale", xscale);
				SetInstanceVariable(inst, "image_yscale", yscale);

				vector<json> instvars = instdata["variables"];
				for (int k = 0; k < instvars.size(); k++)
				{
					vector<json> var = instvars[k];

					string varname = var[0];
					json varvalue = var[1];
					string vartype = var[2];

					if (vartype == "default")
						continue;

					switch (varvalue.type())
					{
						case json::value_t::boolean:
							SetInstanceVariable(inst, varname, varvalue.get<bool>());
							break;

						case json::value_t::number_float:
						case json::value_t::number_unsigned:
						case json::value_t::number_integer:
							SetInstanceVariable(inst, varname, varvalue.get<float>());
							break;

						case json::value_t::string:
							SetInstanceVariable(inst, varname, RValue(varvalue.get<string>()));
							break;
					}
				}
			}
		}

		RValue ob_player = GetAsset("ob_player");
		if (!g_interface->CallBuiltin("instance_exists", {ob_player}).ToBoolean())
		{
			RValue ob_stageManager = GetAsset("ob_stageManager");

			Print("Setting current stage data");
			map<string, RValue> stageData = CreateStageData();
			SetInstanceVariable(ob_stageManager, "currentStage", stageData);

			Print("Doing stage intro");
			// Starts the stage intro, requires stage data first or else it crashes
			RValue defaultStageInit = GetInstanceVariable(ob_stageManager, "defaultStageInit");
			CallMethod(defaultStageInit);
		}
	}

	void InitializeLoaderHooks()
	{
		room_goto_original = CreateBuiltinHook(g_module, "room_goto", "room_goto_hook", RoomGotoHook);
		CreateBuiltinHook(g_module, "analytics_event", "analytics_event_hook", PlayCustomLevelHook);
	}
};
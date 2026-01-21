#include "main.h"
#include "Variables.h"
#include "Functions.h"

#include "RoomLoader.h"
#include <fstream>

json g_roomData;

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

void GoToRoomLoaderRoom()
{
	g_roomData = LoadRoomData("room.rfrm");
	if (g_roomData != nullptr)
	{
		int rmVersion = g_roomData["rf_roomversion"];
		if (rmVersion != ROOM_VERSION)
		{
			Print("Incorrect room version!");
			Print(RValue("Expected " + to_string(ROOM_VERSION) + ", got " + to_string(rmVersion)));
			return;
		}

		RValue templateRoom = GetAsset("rm_template_room");
		if (templateRoom.ToInt32() != GM_INVALID)
		{
			Print("Preparing room");
			int width = g_roomData["roomInfo"]["width"];
			int height = g_roomData["roomInfo"]["height"];

			g_interface->CallBuiltin("room_set_width", {templateRoom, RValue(width)});
			g_interface->CallBuiltin("room_set_height", {templateRoom, RValue(height)});

			Print("Go to template room");
			g_interface->CallBuiltin("room_goto", {templateRoom});
		}
	}
}

void InitializeRoomLoaderRoom()
{
	// Create all layers and objects for the room
	vector<json> layers = g_roomData["layers"];
	for (int i = 0; i < layers.size(); i++)
	{
		json lay = layers[i];
		string layName = lay["name"];
		int layDepth = lay["depth"];

		if (!g_interface->CallBuiltin("layer_exists", {RValue(layName)}))
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
			SetInstanceVariable(inst, "image_xscale", xscale);
			SetInstanceVariable(inst, "image_yscale", yscale);

			vector<json> instvars = instdata["variables"];
			for (int k = 0; k < instvars.size(); k++)
			{
				vector<json> var = instvars[k];

				string varname = var[0];
				json varvalue = var[1];

				// hacky workaround for this one issue...
				if ((id == "ob_spring" || id == "ob_spring_BG") && (varname == "targetX" || varname == "targetY"))
				{
					int targetval = varvalue.get<int>();
					if (targetval == -1)
						continue;
				}
				
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

	RValue ob_stageManager = GetAsset("ob_stageManager");

	Print("Setting current stage data");
	map<string, RValue> stageData = CreateStageData();
	SetInstanceVariable(ob_stageManager, "currentStage", stageData);

	Print("Doing stage intro");
	// Starts the stage intro, requires stage data first or else it crashes
	RValue defaultStageInit = GetInstanceVariable(ob_stageManager, "defaultStageInit");
	CallMethod(defaultStageInit);
}
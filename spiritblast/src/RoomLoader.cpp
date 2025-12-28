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
	g_roomData = LoadRoomData("testrm.json");
	if (g_roomData != nullptr)
	{
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
	vector<json> objectData = g_roomData["objects"];
	for (int i = 0; i < objectData.size(); i++)
	{
		json obj = objectData[i];

		string oid = obj["id"];
		int ox = obj["x"];
		int oy = obj["y"];
		int oxscale = obj["xscale"];
		int oyscale = obj["yscale"];

		RValue oasset = GetAsset(oid);
		if (oasset.ToInt32() == GM_INVALID)
			continue;

		RValue oinst = g_interface->CallBuiltin("instance_create_layer", {ox, oy, "InstancesFG", oasset});
		SetInstanceVariable(oinst, "image_xscale", oxscale);
		SetInstanceVariable(oinst, "image_yscale", oyscale);
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
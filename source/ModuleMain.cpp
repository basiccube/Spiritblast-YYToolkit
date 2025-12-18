#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

#include <vector>
using namespace std;

#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

static YYTKInterface *g_interface = nullptr;

struct SpriteData
{
	RValue sprite;
	RValue x;
	RValue y;
	RValue xscale;
	RValue yscale;
	RValue angle;
};

vector<SpriteData> g_debugCollisionData;
vector<SpriteData> g_debugCollisionDataBG;
static bool g_showDebugCollision = false;

void Print(const char* str)
{
	g_interface->CallBuiltin("show_debug_message", {RValue(str)});
}

void DrawDebugCollisionDataBG()
{
	// Override depth so that it doesn't draw in front of everything
	//RValue prevDepth = g_interface->CallBuiltin("gpu_get_depth", {});
	//g_interface->CallBuiltin("gpu_set_depth", {RValue(2500)});

	RValue camID = RValue();
	g_interface->GetBuiltin("view_camera", nullptr, 0, camID);

	RValue camX = g_interface->CallBuiltin("camera_get_view_x", {camID});

	for (int i = 0; i < g_debugCollisionDataBG.size(); i++)
	{
		SpriteData &data = g_debugCollisionDataBG[i];
		g_interface->CallBuiltin("draw_sprite_ext", {data.sprite, RValue(0), RValue(data.x.ToInt32() + 25 + (camX.ToInt32() * 0.5)), data.y, data.xscale, data.yscale, data.angle, RValue(16777215), RValue(1)});
	}

	//g_interface->CallBuiltin("gpu_set_depth", {prevDepth});
}

void DrawDebugCollisionData()
{
	for (int i = 0; i < g_debugCollisionData.size(); i++)
	{
		SpriteData &data = g_debugCollisionData[i];
		g_interface->CallBuiltin("draw_sprite_ext", {data.sprite, RValue(0), data.x, data.y, data.xscale, data.yscale, data.angle, RValue(16777215), RValue(1)});
	}
}

void GetDebugCollisionData(const char* name, int gameplayLayer = 0)
{
	RValue obj = g_interface->CallBuiltin("asset_get_index", {RValue(name)});
	RValue instNum = g_interface->CallBuiltin("instance_number", {obj});

	for (int i = 0; i < instNum.ToInt32(); i++)
	{
		RValue inst = g_interface->CallBuiltin("instance_find", {obj, RValue(i)});
		RValue obj = g_interface->CallBuiltin("variable_instance_get", {inst, RValue("object_index")});
		RValue objname = g_interface->CallBuiltin("object_get_name", {obj});
		RValue layer = g_interface->CallBuiltin("variable_instance_get", {inst, RValue("gameplayLayer")});

		if (objname.ToString() != name || layer.ToInt32() != gameplayLayer)
			continue;

		SpriteData data = {
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("sprite_index")}),
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("x")}),
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("y")}),
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("image_xscale")}),
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("image_yscale")}),
			g_interface->CallBuiltin("variable_instance_get", {inst, RValue("image_angle")})
		};

		RValue validSprite = g_interface->CallBuiltin("sprite_exists", {data.sprite});
		if (!validSprite.ToBoolean())
		{
			g_interface->CallBuiltin("show_debug_message", {data.sprite});
			Print("Not a valid sprite, ignoring...");
			continue;
		}

		if (gameplayLayer > 0)
			g_debugCollisionDataBG.push_back(data);
		else
			g_debugCollisionData.push_back(data);
	}
}

RValue GetInstance(string name)
{
	RValue obj = g_interface->CallBuiltin("asset_get_index", {RValue(name)});
	RValue instNum = g_interface->CallBuiltin("instance_number", {obj});

	for (int i = 0; i < instNum.ToInt32(); i++)
	{
		RValue inst = g_interface->CallBuiltin("instance_find", {obj, RValue(i)});
		if (inst.ToInt32() != -4)
			return inst;
	}

	return RValue();
}

bool DumpVariableFunc(const char *name, RValue *value)
{
	Print(name);
	return false;
}

void DumpGlobalVariables()
{
	Print("Dumping global variables...");

	CInstance *globalInst{};
	AurieStatus gres = g_interface->GetGlobalInstance(&globalInst);
	if (AurieSuccess(gres))
		g_interface->EnumInstanceMembers(globalInst, DumpVariableFunc);
}

void DumpPlayerVariables()
{
	Print("Dumping player variables...");

	RValue obj = g_interface->CallBuiltin("asset_get_index", {RValue("ob_player")});
	RValue instNum = g_interface->CallBuiltin("instance_number", {obj});

	for (int i = 0; i < instNum.ToInt32(); i++)
	{
		RValue inst = g_interface->CallBuiltin("instance_find", {obj, RValue(i)});
		RValue arr = g_interface->CallBuiltin("variable_instance_get_names", {inst});

		vector<RValue> nameArr = arr.ToVector();
		for (int j = 0; j < nameArr.size(); j++)
			g_interface->CallBuiltin("show_debug_message", {nameArr[j]});
	}
}

json roomData;

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
		g_interface->CallBuiltin("show_debug_message", {RValue(e.what())});
		return nullptr;
	}

	file.close();
	return data;
}

void GoToCustomLevelRoom()
{
	roomData = LoadRoomData("testrm.json");
	if (roomData != nullptr)
	{
		RValue templateRoom = g_interface->CallBuiltin("asset_get_index", {RValue("rm_template_room")});
		if (templateRoom.ToInt32() != -1)
		{
			Print("Preparing room");
			int width = roomData["roomInfo"]["width"];
			int height = roomData["roomInfo"]["height"];

			g_interface->CallBuiltin("room_set_width", {templateRoom, RValue(width)});
			g_interface->CallBuiltin("room_set_height", {templateRoom, RValue(height)});

			RValue spawnManagerObject = g_interface->CallBuiltin("asset_get_index", {RValue("ob_spawnManager")});
			g_interface->CallBuiltin("instance_create", {spawnManagerObject});

			Print("Go to template room");
			g_interface->CallBuiltin("room_goto", {templateRoom});
		}
	}
}

void ChangeMainMenuPage(string name)
{
	RValue menuInst = GetInstance("ob_listMenu");
	RValue changePageFunc = g_interface->CallBuiltin("variable_instance_get", {menuInst, RValue("changePage")});

	RValue pageValue = g_interface->CallBuiltin("variable_instance_get", {menuInst, RValue(name)});
	if (pageValue.ToInt32() == -1)
		return;

	vector<RValue> argArray = {pageValue};
	RValue funcArgs = RValue(argArray);
	g_interface->CallBuiltin("method_call", {changePageFunc, funcArgs});
}

void FrameCallback(FWFrame &frameCtx)
{
	UNREFERENCED_PARAMETER(frameCtx);

	RValue dumpGlobalPress = g_interface->CallBuiltin("keyboard_check_pressed", {RValue(VK_F1)});
	if (dumpGlobalPress.ToBoolean())
		DumpGlobalVariables();

	RValue dumpPlayerPress = g_interface->CallBuiltin("keyboard_check_pressed", {RValue(VK_F2)});
	if (dumpPlayerPress.ToBoolean())
		DumpPlayerVariables();

	RValue gotoTemplateRoom = g_interface->CallBuiltin("keyboard_check_pressed", {RValue(VK_F3)});
	if (gotoTemplateRoom.ToBoolean())
		GoToCustomLevelRoom();

	bool prevDebugCollision = g_showDebugCollision;
	RValue colKeyPress = g_interface->CallBuiltin("keyboard_check_pressed", {RValue(VK_F4)});
	if (colKeyPress.ToBoolean())
		g_showDebugCollision = !g_showDebugCollision;

	RValue changePagePress = g_interface->CallBuiltin("keyboard_check_pressed", {RValue(VK_F5)});
	if (changePagePress.ToBoolean())
	{
		RValue inputString = g_interface->CallBuiltin("get_string", {RValue("Type the variable name of the page you want to go to."), RValue("mainPage")});
		if (inputString.ToString() != "")
			ChangeMainMenuPage(inputString.ToString());
	}
}

void EventCallback(FWCodeEvent &eventCtx)
{
	RValue event_type = RValue();
	RValue event_number = RValue();
	RValue event_object = RValue();
	RValue event_object_name = RValue();

	g_interface->GetBuiltin("event_type", nullptr, NULL_INDEX, event_type);
	g_interface->GetBuiltin("event_number", nullptr, NULL_INDEX, event_number);
	g_interface->GetBuiltin("event_object", nullptr, NULL_INDEX, event_object);
	event_object_name = g_interface->CallBuiltin("object_get_name", {event_object});

	switch (event_type.ToInt32())
	{
		// Create event
		case 0:
			break;

		// Destroy event
		case 1:
			break;

		// Step type
		case 3:
			switch (event_number.ToInt32())
			{
				// Step event
				case 0:
					break;
			}
			break;

		// Other type
		case 7:
			switch (event_number.ToInt32())
			{
				// Room start event
				case 4:
					// Get the sprite data from all of the debug collision objects
					if (event_object_name.ToString() == "ob_camera")
					{
						RValue room = RValue();
						g_interface->GetBuiltin("room", nullptr, NULL_INDEX, room);
						RValue roomName = g_interface->CallBuiltin("room_get_name", {room});

						if (roomName.ToString() == "rm_template_room")
						{
							RValue playerObject = g_interface->CallBuiltin("asset_get_index", {RValue("ob_player")});
							if (!g_interface->CallBuiltin("instance_exists", {playerObject}).ToBoolean())
							{
								Print("Creating player object");
								g_interface->CallBuiltin("instance_create_depth", {RValue(32), RValue(32), RValue(0), playerObject});
							}

							vector<json> objectData = roomData["objects"];
							for (int i = 0; i < objectData.size(); i++)
							{
								json obj = objectData[i];

								string oid = obj["id"];
								int ox = obj["x"];
								int oy = obj["y"];
								int oxscale = obj["xscale"];
								int oyscale = obj["yscale"];

								RValue oasset = g_interface->CallBuiltin("asset_get_index", {RValue(oid)});
								if (oasset.ToInt32() == -1)
									continue;

								RValue oinst = g_interface->CallBuiltin("instance_create_layer", {RValue(ox), RValue(oy), RValue("InstancesFG"), oasset});
								g_interface->CallBuiltin("variable_instance_set", {oinst, RValue("image_xscale"), RValue(oxscale)});
								g_interface->CallBuiltin("variable_instance_set", {oinst, RValue("image_yscale"), RValue(oyscale)});
							}
						}

						Print("Getting debug collision sprite data");

						const char *colObjects[] = {"ob_block", "ob_passthrough", "ob_passthroughSlope14",
													"ob_passthroughSlope22", "ob_passthroughSlope45",
													"ob_slope14", "ob_slope22", "ob_slope22_R",
													"ob_slope45", "ob_slope45_R", "ob_ladder"};

						for (int i = 0; i < sizeof(colObjects) / sizeof(colObjects[0]); i++)
							GetDebugCollisionData(colObjects[i]);

						const char *colObjectsBG[] = {"ob_block_BG", "ob_passthrough_BG", "ob_ladder_BG",
													"ob_passthroughSlope22_BG", "ob_passthroughSlope45_BG",
													"ob_slope12_BG", "ob_slope22_BG", "ob_slope45_BG"};

						for (int i = 0; i < sizeof(colObjectsBG) / sizeof(colObjectsBG[0]); i++)
							GetDebugCollisionData(colObjectsBG[i], 1);
					}
					break;

				// Room end event
				case 5:
					if (event_object_name.ToString() == "ob_camera")
					{
						Print("Clearing debug collision sprite data");
						g_debugCollisionData.clear();
						g_debugCollisionDataBG.clear();
					}
					break;
			}
			break;

		// Draw type
		case 8:
			switch (event_number.ToInt32())
			{
				// Draw event
				case 0:
					if (g_showDebugCollision && event_object_name.ToString() == "ob_particleBGDrawer")
						DrawDebugCollisionDataBG();
					if (g_showDebugCollision && event_object_name.ToString() == "ob_camera")
						DrawDebugCollisionData();
					break;

				// Draw Begin event
				case 72:
					break;
			}
			break;
	}
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;

	g_interface = YYTK::GetInterface();
	if (!g_interface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	
	status = g_interface->CreateCallback(Module, EVENT_FRAME, FrameCallback, 2);
	if (!AurieSuccess(status))
		printf("Failed to create frame callback\n");

	status = g_interface->CreateCallback(Module, EVENT_OBJECT_CALL, EventCallback, 1);
	if (!AurieSuccess(status))
		printf("Failed to create event callback\n");

	// Enable debug overlay
	g_interface->CallBuiltin("show_debug_overlay", {RValue(true)});
	g_interface->CallBuiltin("variable_global_set", {RValue("debug"), RValue(true)});
	g_interface->CallBuiltin("variable_global_set", {RValue("debug_visible"), RValue(true)});

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}
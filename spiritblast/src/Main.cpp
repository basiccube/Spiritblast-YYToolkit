#include "main.h"
#include "LuaLibraries.h"
#include "IniFile.h"
#include "Variables.h"
#include "Functions.h"
#include "Menus.h"
#include "DebugCollision.h"
#include "RoomLoader.h"

YYTKInterface *g_interface = nullptr;
LuaContext *g_lua = nullptr;

RValue GetInstance(string name)
{
	RValue obj = GetAsset(name);
	RValue instNum = g_interface->CallBuiltin("instance_number", {obj});

	for (int i = 0; i < instNum.ToInt32(); i++)
	{
		RValue inst = g_interface->CallBuiltin("instance_find", {obj, i});
		if (!inst.IsUndefined())
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

void DumpObjectVariables(string name)
{
	Print(RValue("Dumping object variables: " + name));

	RValue inst = GetInstance(name);
	if (inst.IsUndefined())
	{
		Print(RValue("No instance found for object " + name));
		return;
	}	
	
	vector<string> arr = GetInstanceVariableNames(inst);
	for (string str : arr)
		Print(RValue(str));
}

static bool g_createdDebugMenu = false;
static bool g_createdCustomOptions = false;

static bool g_debugControls = false;
static bool g_skipSplash = false;
static int g_debugOptionsIndex = -1;

static PFUNC_YYGMLScript g_envGetUsernameOriginal;

// Hook used for changing the header text in the options menu.
RValue &EnvironmentGetUsernameHook(CInstance *self, CInstance *other, RValue &returnValue, int argCount, RValue **args)
{
	RValue selfValue = self->ToRValue();
	RValue otherValue = other->ToRValue();

	if (otherValue.IsStruct())
	{
		RValue displayExists = g_interface->CallBuiltin("struct_exists", {otherValue, "displayPage"});
		if (displayExists.ToBoolean())
		{
			RValue pages = g_interface->CallBuiltin("struct_get", {otherValue, "pages"});
			RValue currentPage = g_interface->CallBuiltin("struct_get", {otherValue, "currentPage"});
			RValue currentIndex = g_interface->CallBuiltin("array_get_index", {pages, currentPage});

			if (currentIndex.ToInt32() == g_debugOptionsIndex)
				returnValue = "Debug";
			else
			{
				RValue getPageHeader = g_interface->CallBuiltin("struct_get", {otherValue, "getPageHeaderOriginal"});
				RValue headerValue = CallMethod(getPageHeader);
				returnValue = headerValue;
			}
		}
		else
			goto environment_get_username_func;
	}
	else
		goto environment_get_username_func;

	return returnValue;

environment_get_username_func:
	g_envGetUsernameOriginal(self, other, returnValue, argCount, args);
	return returnValue;
}

static TRoutine g_roomGotoOriginal;

void RoomGotoHook(RValue &result, CInstance *self, CInstance *other, int argCount, RValue *args)
{
	RValue rm = args[0];
	if (g_interface->CallBuiltin("is_string", {rm}).ToBoolean())
	{
		GoToRoomLoaderRoom(rm.ToString());
		return;
	}

	g_roomGotoOriginal(result, self, other, argCount, args);
}

void FrameCallback(FWFrame &frameCtx)
{
	UNREFERENCED_PARAMETER(frameCtx);

	if (!g_debugControls)
		return;

	RValue dumpGlobalPress = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F1});
	if (dumpGlobalPress.ToBoolean())
		DumpGlobalVariables();

	RValue dumpObjectPress = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F2});
	if (dumpObjectPress.ToBoolean())
	{
		string inputString = ShowStringInputPopup("Type the name of the object you want to dump all of the variables from:", "ob_player");
		if (inputString != "")
			DumpObjectVariables(inputString);
	}

	RValue gotoTemplateRoom = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F3});
	if (gotoTemplateRoom.ToBoolean())
		GoToRoomLoaderRoom("room");

	bool prevDebugCollision = g_showDebugCollision;
	RValue colKeyPress = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F4});
	if (colKeyPress.ToBoolean())
		g_showDebugCollision = !g_showDebugCollision;

	RValue changePagePress = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F5});
	if (changePagePress.ToBoolean())
	{
		string inputString = ShowStringInputPopup("Type the variable name of the page you want to go to:", "mainPage");
		if (inputString != "")
		{
			RValue menuInst = GetInstance("ob_listMenu");
			if (menuInst.IsUndefined())
			{
				Print("Not in a menu!");
				return;
			}

			ChangeMenuPage(menuInst, inputString);
		}
	}

	RValue runLuaPress = g_interface->CallBuiltin("keyboard_check_pressed", {VK_F6});
	if (runLuaPress.ToBoolean())
	{
		try
		{
			g_lua->CompileFileAndRun("test.lua");
		}
		catch (runtime_error &e)
		{
			DbgPrintEx(LOG_SEVERITY_ERROR, "LUA ERROR: ");
			DbgPrintEx(LOG_SEVERITY_ERROR, e.what());
		}
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

				// Begin Step event
				case 1:
					if (event_object_name.ToString() == "ob_player" && g_debugOverlayOpen)
					{
						bool debugCheck = IsKeyboardUsedByDebugOverlay();
						RValue playerInputManager = GetAsset("ob_playerInputManager");
						RValue instCheck = g_interface->CallBuiltin("instance_exists", {playerInputManager});
						if (debugCheck)
						{
							if (instCheck.ToBoolean())
								g_interface->CallBuiltin("instance_deactivate_object", {playerInputManager});
						}
						else
						{
							if (!instCheck.ToBoolean())
								g_interface->CallBuiltin("instance_activate_object", {playerInputManager});
						}
					}
					else if (event_object_name.ToString() == "ob_camera" && g_debugOverlayOpen)
					{
						bool debugCheck = IsKeyboardUsedByDebugOverlay();
						RValue globalInput = GetAsset("ob_globalInput");
						RValue instCheck = g_interface->CallBuiltin("instance_exists", {globalInput});
						if (debugCheck)
						{
							if (instCheck.ToBoolean())
								g_interface->CallBuiltin("instance_deactivate_object", {globalInput});
						}
						else
						{
							if (!instCheck.ToBoolean())
								g_interface->CallBuiltin("instance_activate_object", {globalInput});
						}
					}
					break;

				// End Step event
				case 2:
					// Create the debug menu and button.
					// This has to happen here and not in the create event above
					// because this needs the variables that are created in the menus create event
					// and the custom code is executed right before the event.
					if (event_object_name.ToString() == "ob_mainMenu" && !g_createdDebugMenu)
					{
						RValue menu = GetInstance("ob_mainMenu");
						if (menu.IsUndefined())
						{
							Print("No main menu instance found");
							break;
						}

						/*
						RValue debugPage = CreateMenuPage(menu);
						{
							RValue mouseEnabledValue = g_interface->CallBuiltin("variable_instance_get", {menu, "mouseEnabled"});
							CInstance *mouseToggle = CreateMenuToggle(menu, "Mouse Enabled", mouseEnabledValue, menu, "mouseEnabled");
							AddItemToPageValue(menu, debugPage, mouseToggle);

							CInstance *backBtn = CreateBackButton(menu, "Back");
							AddItemToPageValue(menu, debugPage, backBtn);
						}

						CInstance *debugMenuBtn = CreateChangePageButton(menu, "Debug", debugPage);
						AddItemToPage(menu, "mainPage", debugMenuBtn);
						*/

						g_createdDebugMenu = true;
					}
					break;
			}
			break;

		// Other type
		case 7:
			switch (event_number.ToInt32())
			{
				// Room start event
				case 4:
					if (event_object_name.ToString() == "ob_camera")
					{
						RValue room = RValue();
						g_interface->GetBuiltin("room", nullptr, NULL_INDEX, room);
						RValue roomName = g_interface->CallBuiltin("room_get_name", {room});

						// Load the custom room data if we are in the template room
						if (roomName.ToString() == "rm_template_room")
							InitializeRoomLoaderRoom();
						else if (roomName.ToString() == "rm_splashScreen")
						{
							ClearInstanceMap();
							if (g_skipSplash)
							{
								RValue rm_logoDrop = GetAsset("rm_logoDrop");
								if (rm_logoDrop.ToInt32() != GM_INVALID)
								{
									Print("Skip splash screens");
									g_interface->CallBuiltin("room_goto", {rm_logoDrop});
								}
							}
						}

						// Get the sprite data from all of the debug collision objects
						DebugCollisionInit();
					}
					break;

				// Room end event
				case 5:
					if (event_object_name.ToString() == "ob_camera")
					{
						ClearDebugCollisionData();
						
						if (g_debugOverlayOpen)
						{
							RValue playerInputManager = GetAsset("ob_playerInputManager");
							RValue globalInput = GetAsset("ob_globalInput");

							g_interface->CallBuiltin("instance_activate_object", {playerInputManager});
							g_interface->CallBuiltin("instance_activate_object", {globalInput});
						}
					}

					g_createdDebugMenu = false;
					g_createdCustomOptions = false;
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

					// Create all of the custom options.
					if (event_object_name.ToString() == "ob_optionsMenu")
					{
						if (!g_createdCustomOptions)
						{
							RValue menu = GetInstance("ob_optionsMenu");
							if (menu.IsUndefined())
							{
								Print("No options menu instance found");
								break;
							}

							CInstance *globalInst{};
							AurieStatus globalResult = g_interface->GetGlobalInstance(&globalInst);

							if (!AurieSuccess(globalResult))
								break;

							RValue debugPage = CreateMenuPage(menu);
							{
								RValue debugOverlayValue = GetGlobalVariable("debug_overlay");
								CInstance *debugOverlayInst = CreateMenuToggle(menu, "Debug Overlay", debugOverlayValue, globalInst, "debug_overlay");
								AddItemToPageValue(menu, debugPage, debugOverlayInst);

								RValue debugControlsValue = GetGlobalVariable("debug_controls");
								CInstance *debugControlsInst = CreateMenuToggle(menu, "Debug Controls", debugControlsValue, globalInst, "debug_controls");
								AddItemToPageValue(menu, debugPage, debugControlsInst);

								RValue skipSplashValue = GetGlobalVariable("skip_splash");
								CInstance *skipSplashInst = CreateMenuToggle(menu, "Skip Splash Screens", skipSplashValue, globalInst, "skip_splash");
								AddItemToPageValue(menu, debugPage, skipSplashInst);

								CInstance *backBtn = CreateBackButton(menu, "Back");
								AddItemToPageValue(menu, debugPage, backBtn);
							}

							RValue pages = GetInstanceVariable(menu, "pages");
							RValue debugPageIndex = g_interface->CallBuiltin("array_get_index", {pages, debugPage});
							g_debugOptionsIndex = debugPageIndex.ToInt32();

							RValue getPageHeader = GetInstanceVariable(menu, "getPageHeader");
							SetInstanceVariable(menu, "getPageHeaderOriginal", getPageHeader);

							RValue environment_get_username = GetGlobalVariable("environment_get_username");
							SetInstanceVariable(menu, "getPageHeader", environment_get_username);

							CInstance *debugPageBtn = CreateChangePageButton(menu, "Debug", debugPage);
							AddItemToPage(menu, "mainPage", debugPageBtn, 5);

							// For some reason creating a page in the options menu also changes the current page to it
							ChangeMenuPage(menu, "mainPage", false);

							RValue bfullscreenValue = GetGlobalVariable("borderless_fullscreen");
							CInstance *bfullscreenInst = CreateMenuToggle(menu, "Borderless Fullscreen", bfullscreenValue, globalInst, "borderless_fullscreen");
							AddItemToPage(menu, "displayPage", bfullscreenInst, 2);

							g_createdCustomOptions = true;
						}
						
						RValue prevBorderlessValue = g_interface->CallBuiltin("window_get_borderless_fullscreen", {});
						RValue borderlessValue = GetGlobalVariable("borderless_fullscreen");

						if (prevBorderlessValue.ToBoolean() != borderlessValue.ToBoolean())
						{
							Print("Borderless fullscreen value has changed, setting borderless fullscreen mode");
							g_interface->CallBuiltin("window_enable_borderless_fullscreen", {borderlessValue});
						}
					}
					break;

				// Draw Begin event
				case 72:
					break;
			}
			break;

		// Clean Up event
		case 12:
			if (event_object_name.ToString() == "ob_listMenu")
			{
				g_createdDebugMenu = false;
				g_createdCustomOptions = false;

				// Write any custom settings to the ini file
				IniOpen(MOD_SETTINGS_FILE);

				GetGlobalAndWriteToIni({"Video", "BorderlessFullscreen"}, "borderless_fullscreen");

				RValue debugOverlayValue = GetGlobalAndWriteToIni({"Debug", "DebugOverlay"}, "debug_overlay");
				ShowDebugOverlay(debugOverlayValue.ToBoolean());

				RValue debugControlsValue = GetGlobalAndWriteToIni({"Debug", "DebugControls"}, "debug_controls");
				g_debugControls = debugControlsValue.ToBoolean();

				RValue skipSplashValue = GetGlobalAndWriteToIni({"Debug", "SkipSplash"}, "skip_splash");
				g_skipSplash = skipSplashValue.ToBoolean();

				IniClose();
			}
			break;
	}
}

EXPORTED AurieStatus ModuleInitialize(IN AurieModule* Module, IN const fs::path& ModulePath)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;

	g_interface = YYTK::GetInterface();
	if (!g_interface)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	
	status = g_interface->CreateCallback(Module, EVENT_FRAME, FrameCallback, 2);
	if (!AurieSuccess(status))
	{
		DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create frame callback\n");
		return AURIE_MODULE_INITIALIZATION_FAILED;
	}
	
	status = g_interface->CreateCallback(Module, EVENT_OBJECT_CALL, EventCallback, 1);
	if (!AurieSuccess(status))
	{
		DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create event callback\n");
		return AURIE_MODULE_INITIALIZATION_FAILED;
	}

	DbgPrintEx(LOG_SEVERITY_INFO, "Mod successfully initialized!\n");
	DbgPrintEx(LOG_SEVERITY_INFO, string("Spiritblast v" + GetModVersion()).c_str());

	// Read all custom settings from the settings INI.
	IniOpen(MOD_SETTINGS_FILE);

	// Borderless fullscreen
	RValue bfullscreenValue = ReadAndSetGlobalFromIni({"Video", "BorderlessFullscreen", false}, "borderless_fullscreen");
	g_interface->CallBuiltin("window_enable_borderless_fullscreen", {bfullscreenValue});

	// Debug overlay
	RValue debugOverlayValue = ReadAndSetGlobalFromIni({"Debug", "DebugOverlay", false}, "debug_overlay");
	ShowDebugOverlay(debugOverlayValue.ToBoolean());

	// Debug controls
	RValue debugControlsValue = ReadAndSetGlobalFromIni({"Debug", "DebugControls", false}, "debug_controls");
	g_debugControls = debugControlsValue.ToBoolean();

	// Skip splash screens
	RValue skipSplashValue = ReadAndSetGlobalFromIni({"Debug", "SkipSplash", false}, "skip_splash");
	g_skipSplash = skipSplashValue.ToBoolean();

	IniClose();

	// Hook into environment_get_username to override the options menu header later
	g_envGetUsernameOriginal = CreateHook(Module, "gml_Script_environment_get_username", "environment_get_username_hook", EnvironmentGetUsernameHook);

	// Hook for the room loader
	g_roomGotoOriginal = CreateBuiltinHook(Module, "room_goto", "room_goto_hook", RoomGotoHook);

	// Initialize Lua and register all Lua libraries
	g_lua = new LuaContext();
	RegisterLuaInterfaceLibrary();

	return AURIE_SUCCESS;
}
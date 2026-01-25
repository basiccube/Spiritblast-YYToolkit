#pragma once

#include "main.h"

#include <string>
#include <map>
#include <vector>

#include <json.hpp>
using json = nlohmann::json;

extern json g_roomData;
extern string g_roomName;
extern map<string, vector<string>> g_roomInstanceMap;

#define ROOM_VERSION 1

// Returns the string used for the room loader instance map.
string GetInstanceIDString(string obj, float x, float y);

// Clears all room instance lists in the instance map.
void ClearInstanceMap();

// Creates stage data for a level.
map<string, RValue> CreateStageData();

// Loads room data from the specified file path.
json LoadRoomData(string path);

// Goes to the room used by the room loader.
void GoToRoomLoaderRoom(string name);

// Initializes the room loader room using the currently loaded room data.
void InitializeRoomLoaderRoom();
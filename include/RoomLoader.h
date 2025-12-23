#pragma once

#include <string>

#include <json.hpp>
using json = nlohmann::json;

extern json g_roomData;

// Loads room data from the specified file path.
json LoadRoomData(string path);

// Goes to the room used by the room loader.
void GoToRoomLoaderRoom();

// Initializes the room loader room using the currently loaded room data.
void InitializeRoomLoaderRoom();
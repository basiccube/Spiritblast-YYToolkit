#pragma once

#include "main.h"

extern bool g_showDebugCollision;

// Draws the background collision data.
void DrawDebugCollisionDataBG();

// Draws the foreground collision data.
void DrawDebugCollisionData();

// Clears all of the collision data.
void ClearDebugCollisionData();

// Gets the sprite data for all debug collision objects.
void DebugCollisionInit();
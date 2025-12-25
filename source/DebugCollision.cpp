#include "DebugCollision.h"
#include "Variables.h"

struct DebugSpriteData
{
	RValue sprite;
	RValue x;
	RValue y;
	RValue xscale;
	RValue yscale;
	RValue angle;
};

vector<DebugSpriteData> g_debugCollisionData;
vector<DebugSpriteData> g_debugCollisionDataBG;
bool g_showDebugCollision = false;

void DrawDebugCollisionDataBG()
{
	RValue camID = RValue();
	g_interface->GetBuiltin("view_camera", nullptr, 0, camID);

	RValue camX = g_interface->CallBuiltin("camera_get_view_x", {camID});

	for (int i = 0; i < g_debugCollisionDataBG.size(); i++)
	{
		DebugSpriteData &data = g_debugCollisionDataBG[i];
		g_interface->CallBuiltin("draw_sprite_ext", {data.sprite,
													RValue(0),
													RValue(data.x.ToInt32() + 25 + (camX.ToInt32() * 0.5)),
													data.y,
													data.xscale,
													data.yscale,
													data.angle,
													GM_C_WHITE,
													RValue(1)});
	}
}

void DrawDebugCollisionData()
{
	for (int i = 0; i < g_debugCollisionData.size(); i++)
	{
		DebugSpriteData &data = g_debugCollisionData[i];
		g_interface->CallBuiltin("draw_sprite_ext", {data.sprite,
													RValue(0),
													data.x,
													data.y,
													data.xscale,
													data.yscale,
													data.angle,
													GM_C_WHITE,
													RValue(1)});
	}
}

void GetDebugCollisionData(const char *name, int gameplayLayer = 0)
{
	RValue obj = g_interface->CallBuiltin("asset_get_index", {name});
	RValue instNum = g_interface->CallBuiltin("instance_number", {obj});

	for (int i = 0; i < instNum.ToInt32(); i++)
	{
		RValue inst = g_interface->CallBuiltin("instance_find", {obj, i});
		if (inst.IsUndefined())
			continue;

		RValue obj = GetInstanceVariable(inst, "object_index");
		RValue objname = g_interface->CallBuiltin("object_get_name", {obj});
		RValue layer = GetInstanceVariable(inst, "gameplayLayer");

		if (objname.ToString() != name || layer.ToInt32() != gameplayLayer)
			continue;

		DebugSpriteData data = {
			GetInstanceVariable(inst, "sprite_index"),
			GetInstanceVariable(inst, "x"),
			GetInstanceVariable(inst, "y"),
			GetInstanceVariable(inst, "image_xscale"),
			GetInstanceVariable(inst, "image_yscale"),
			GetInstanceVariable(inst, "image_angle")
		};

		RValue validSprite = g_interface->CallBuiltin("sprite_exists", {data.sprite});
		if (!validSprite.ToBoolean())
		{
			Print(data.sprite);
			Print("Not a valid sprite, ignoring...");
			continue;
		}

		if (gameplayLayer > 0)
			g_debugCollisionDataBG.push_back(data);
		else
			g_debugCollisionData.push_back(data);
	}
}

void ClearDebugCollisionData()
{
	Print("Clearing debug collision sprite data");
	g_debugCollisionData.clear();
	g_debugCollisionDataBG.clear();
}

void DebugCollisionInit()
{
	Print("Getting debug collision sprite data");

	const char *colObjects[] = {"ob_block", "ob_passthrough", "ob_passthroughSlope14",
								"ob_passthroughSlope22", "ob_passthroughSlope45",
								"ob_slope14", "ob_slope22", "ob_slope45", "ob_ladder"};

	for (int i = 0; i < sizeof(colObjects) / sizeof(colObjects[0]); i++)
		GetDebugCollisionData(colObjects[i]);

	const char *colObjectsBG[] = {"ob_block_BG", "ob_passthrough_BG", "ob_ladder_BG",
								"ob_passthroughSlope22_BG", "ob_passthroughSlope45_BG",
								"ob_slope12_BG", "ob_slope22_BG", "ob_slope45_BG"};

	for (int i = 0; i < sizeof(colObjectsBG) / sizeof(colObjectsBG[0]); i++)
		GetDebugCollisionData(colObjectsBG[i], 1);
}
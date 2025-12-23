#pragma once

#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

#include <string>
#include <vector>
#include <map>
using namespace std;

extern YYTKInterface *g_interface;

#define GM_C_WHITE RValue(16777215)
#define GM_C_BLACK RValue(0)

#define GM_INVALID -1

#define MOD_SETTINGS_FILE "antonblast_ab-mod_settings.ini"

void Print(RValue str);

// Creates a hook for the specified script function.
PFUNC_YYGMLScript CreateHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction);

// Creates a hook for the specified builtin function.
TRoutine CreateBuiltinHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction);
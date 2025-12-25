#pragma once

#include "main.h"

// Calls the method with the arguments specified.
RValue CallMethod(RValue method, vector<RValue> args = {});

// Gets the internal script name for the specified instance method.
string GetInstanceMethodScriptName(RValue inst, string name);

// Gets the internal script function name.
string GetFunctionScriptName(string name);

// Creates a hook for the specified script function.
PFUNC_YYGMLScript CreateHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction);

// Creates a hook for the specified builtin function.
TRoutine CreateBuiltinHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction);
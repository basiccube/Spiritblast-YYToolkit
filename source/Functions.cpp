#include "Functions.h"
#include "Variables.h"

RValue CallMethod(RValue method, vector<RValue> args)
{
	if (args.empty())
	{
		RValue ret = g_interface->CallBuiltin("method_call", {method});
		return ret;
	}

	RValue methodArgs = RValue(args);
	RValue ret = g_interface->CallBuiltin("method_call", {method, methodArgs});
	return ret;
}

string GetInstanceMethodScriptName(RValue inst, string name)
{
	RValue val = GetInstanceVariable(inst, name);
	RValue scrName = g_interface->CallBuiltin("script_get_name", {val});
	return scrName.ToString();
}

string GetFunctionScriptName(string name)
{
	RValue val = GetGlobalVariable(name);
	RValue scrName = g_interface->CallBuiltin("script_get_name", {val});
	return scrName.ToString();
}

PFUNC_YYGMLScript CreateHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction)
{
	CScript *scrData = nullptr;
	PFUNC_YYGMLScript scrFunction = nullptr;

	AurieStatus status = g_interface->GetNamedRoutinePointer(funcName, reinterpret_cast<PVOID *>(&scrData));
	if (!AurieSuccess(status))
	{
		Print(RValue("Failed to get script data for " + string(funcName)));
		return nullptr;
	}

	Print(RValue("Hooking into " + string(funcName)));
	status = MmCreateHook(module, hookIdentifier, scrData->m_Functions->m_ScriptFunction, hookFunction, reinterpret_cast<PVOID *>(&scrFunction));
	if (!AurieSuccess(status))
	{
		Print(RValue("Failed to create hook for " + string(funcName)));
		return nullptr;
	}

	return scrFunction;
}

TRoutine CreateBuiltinHook(AurieModule *module, const char *funcName, const char *hookIdentifier, PVOID hookFunction)
{
	TRoutine func = nullptr;
	TRoutine funcOriginal = nullptr;

	AurieStatus status = g_interface->GetNamedRoutinePointer(funcName, reinterpret_cast<PVOID *>(&func));
	if (!AurieSuccess(status))
	{
		Print(RValue("Failed to get the function " + string(funcName)));
		return nullptr;
	}

	Print(RValue("Hooking into " + string(funcName)));
	status = MmCreateHook(module, hookIdentifier, func, hookFunction, reinterpret_cast<PVOID *>(&funcOriginal));
	if (!AurieSuccess(status))
	{
		Print(RValue("Failed to create hook for " + string(funcName)));
		return nullptr;
	}

	return funcOriginal;
}
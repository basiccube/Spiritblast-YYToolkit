#include "Variables.h"

bool InstanceVariableExists(RValue inst, string name)
{
	RValue val = g_interface->CallBuiltin("variable_instance_exists", {inst, RValue(name)});
	return val.ToBoolean();
}

vector<string> GetInstanceVariableNames(RValue inst)
{
	RValue varNames = g_interface->CallBuiltin("variable_instance_get_names", {inst});
	vector<RValue> valueVector = varNames.ToVector();
	vector<string> stringVector;
	for (RValue val : valueVector)
	{
		string str = val.ToString();
		stringVector.push_back(str);
	}

	return stringVector;
}

int GetInstanceVariableCount(RValue inst)
{
	RValue val = g_interface->CallBuiltin("variable_instance_names_count", {inst});
	return val.ToInt32();
}

RValue GetInstanceVariable(RValue inst, string name)
{
	RValue val = g_interface->CallBuiltin("variable_instance_get", {inst, RValue(name)});
	return val;
}

void SetInstanceVariable(RValue inst, string name, RValue value)
{
	g_interface->CallBuiltin("variable_instance_set", {inst, RValue(name), value});
}

bool GlobalVariableExists(string name)
{
	RValue val = g_interface->CallBuiltin("variable_global_exists", {RValue(name)});
	return val.ToBoolean();
}

RValue GetGlobalVariable(string name)
{
	RValue val = g_interface->CallBuiltin("variable_global_get", {RValue(name)});
	return val;
}

void SetGlobalVariable(string name, RValue value)
{
	g_interface->CallBuiltin("variable_global_set", {RValue(name), value});
}

RValue CreateReference(RValue inst, RValue var, int index)
{
	RValue ref = g_interface->CallBuiltin("ref_create", {inst, var, index});
	return ref;
}

RValue GetAsset(string name)
{
	RValue asset = g_interface->CallBuiltin("asset_get_index", {RValue(name)});
	return asset;
}
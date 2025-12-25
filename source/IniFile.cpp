#include "IniFile.h"
#include "Variables.h"

void IniOpen(string path)
{
	g_interface->CallBuiltin("ini_open", {RValue(path)});
}

void IniOpenFromString(string ini)
{
	g_interface->CallBuiltin("ini_open_from_string", {RValue(ini)});
}

string IniClose()
{
	RValue ini = g_interface->CallBuiltin("ini_close", {});
	return ini.ToString();
}

void IniWrite(string section, string key, RValue value)
{
	vector<RValue> args = {RValue(section), RValue(key), value};
	if (value.IsString())
		g_interface->CallBuiltin("ini_write_string", args);
	else
		g_interface->CallBuiltin("ini_write_real", args);
}

RValue IniRead(string section, string key, RValue defaultValue, bool isString)
{
	vector<RValue> args = {RValue(section), RValue(key), defaultValue};
	RValue val = RValue();
	if (isString)
		val = g_interface->CallBuiltin("ini_read_string", args);
	else
		val = g_interface->CallBuiltin("ini_read_real", args);

	return val;
}

bool IniSectionExists(string section)
{
	RValue val = g_interface->CallBuiltin("ini_section_exists", {RValue(section)});
	return val.ToBoolean();
}

bool IniKeyExists(string section, string key)
{
	RValue val = g_interface->CallBuiltin("ini_key_exists", {RValue(section), RValue(key)});
	return val.ToBoolean();
}

void IniDeleteSection(string section)
{
	g_interface->CallBuiltin("ini_section_delete", {RValue(section)});
}

void IniDeleteKey(string section, string key)
{
	g_interface->CallBuiltin("ini_key_delete", {RValue(section), RValue(key)});
}

RValue ReadAndSetGlobalFromIni(vector<RValue> iniArgs, string globalVar, bool isString)
{
	RValue val = RValue();
	if (isString)
		val = g_interface->CallBuiltin("ini_read_string", iniArgs);
	else
		val = g_interface->CallBuiltin("ini_read_real", iniArgs);

	SetGlobalVariable(globalVar, val);

	return val;
}

RValue GetGlobalAndWriteToIni(vector<RValue> iniArgs, string globalVar)
{
	RValue val = GetGlobalVariable(globalVar);
	iniArgs.push_back(val);

	if (val.IsString())
		g_interface->CallBuiltin("ini_write_string", iniArgs);
	else
		g_interface->CallBuiltin("ini_write_real", iniArgs);

	return val;
}
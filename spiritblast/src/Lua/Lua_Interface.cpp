#include "LuaLibraries.h"

extern "C"
{
	static int l_call_builtin(lua_State *state)
	{
		int args = lua_gettop(state);
		if (args < 2)
		{
			lprint("Expected 2 arguments");
			return 0;
		}

		luaL_argexpected(state, lua_type(state, 1) == LUA_TSTRING, 1, "string");
		luaL_argexpected(state, lua_istable(state, 2), 2, "table");

		const char *funcName = luaL_checkstring(state, 1);
		vector<RValue> funcArgs;

		size_t len = lua_rawlen(state, 2);
		for (int i = 1; i <= len; i++)
		{
			// push index and get value from the table
			lua_pushinteger(state, i);
			lua_gettable(state, 2);

			switch (lua_type(state, -1))
			{
				case LUA_TNUMBER:
					if (lua_isinteger(state, -1))
						funcArgs.push_back(luaL_checkinteger(state, -1));
					else
						funcArgs.push_back(luaL_checknumber(state, -1));
					break;

				case LUA_TBOOLEAN:
					funcArgs.push_back(lua_toboolean(state, -1));
					break;

				case LUA_TSTRING:
					funcArgs.push_back(luaL_checkstring(state, -1));
					break;

				default:
					funcArgs.push_back(luaL_checknumber(state, -1));
					break;
			}

			lua_pop(state, 1);
		}

		// Return the result depending on what type it is
		RValue result = g_interface->CallBuiltin(funcName, funcArgs);
		switch (result.m_Kind)
		{
			case VALUE_BOOL:
				lua_pushboolean(state, result.ToBoolean());
				break;

			case VALUE_REAL:
				lua_pushnumber(state, result.ToDouble());
				break;

			case VALUE_INT32:
				lua_pushinteger(state, result.ToInt32());
				break;
			case VALUE_INT64:
				lua_pushinteger(state, result.ToInt64());
				break;

			case VALUE_STRING:
				lua_pushstring(state, result.ToCString());
				break;

			case VALUE_ARRAY:
				lprint("Arrays are not supported yet");
				/*
				lua_newtable(state);
				for (int i = 0; i < result.ToVector().size(); i++)
				{
					lua_pushnumber(state, i);
					
					lua_settable(state, -3);
				}
				*/
				break;

			case VALUE_OBJECT:
				lua_pushinteger(state, result.ToInt64());
				break;
			case VALUE_REF:
				lua_pushinteger(state, result.ToInt64());
				break;

			case VALUE_NULL:
				lua_pushnil(state);
				break;
			case VALUE_UNDEFINED:
				lua_pushnil(state);
				break;

			default:
				lua_pushnil(state);
				break;
		}

		return 1;
	}
}

void RegisterLuaInterfaceLibrary()
{
	shared_ptr<LuaLibrary> lib = make_shared<LuaLibrary>("interface");
	lib->AddCFunction("call_builtin", l_call_builtin);

	g_lua->AddLibrary(lib);
}
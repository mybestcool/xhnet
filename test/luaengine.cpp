
#include "qyfh.h"

#include "luaengine.h"
#include "tolua++.h"
#include "main-qyfh.h"

int lua_print(lua_State * luastate)
{
	int nargs = lua_gettop(luastate);

	std::string t;
	for (int i=1; i <= nargs; i++)
	{
		if (lua_istable(luastate, i))
			t += "table";
		else if (lua_isnone(luastate, i))
			t += "none";
		else if (lua_isnil(luastate, i))
			t += "nil";
		else if (lua_isboolean(luastate, i))
		{
			if (lua_toboolean(luastate, i) != 0)
				t += "true";
			else
				t += "false";
		}
		else if (lua_isfunction(luastate, i))
			t += "function";
		else if (lua_islightuserdata(luastate, i))
			t += "lightuserdata";
		else if (lua_isthread(luastate, i))
			t += "thread";
		else
		{
			const char * str = lua_tostring(luastate, i);
			if (str)
				t += lua_tostring(luastate, i);
			else
				t += lua_typename(luastate, lua_type(luastate, i));
		}
		/*if (i!=nargs)
			t += "\t";*/
	}

	XH_LOG_INFO(xhnet::logname_base, "[LUA-print] " << t);

	return 0;
}

CLuaEngine::CLuaEngine()
: m_state(NULL)
, m_callFromLua(0)
, m_bInit(false)
{

}

CLuaEngine::~CLuaEngine( void )
{
	if( m_state )
		lua_close( m_state );
}

bool CLuaEngine::init(unsigned int level)
{
	if (m_bInit)
	{
		return true;
	}
	m_state = lua_open();
	if(!m_state)
		return false;

	m_level = level;

	luaopen_base(m_state);
	luaopen_table(m_state);
	luaL_openlibs(m_state);
	luaopen_string(m_state);
	luaopen_math(m_state);

	const luaL_reg global_functions [] = {
		{"print", lua_print},
		{NULL, NULL}
	};
	luaL_register(m_state, "_G", global_functions);

	string strpath = xhnet::GetModuleDir();
	strpath += "luascript";
	addSearchPath(strpath.c_str());

	tolua_mainqyfh_open(m_state);



	m_bInit = true;

	if(0 != executeScriptFile("luamain.lua"))
		return false;

	return true;
}

int CLuaEngine::executeScriptFile( const char* filename )
{
	string strpath = xhnet::GetModuleDir();
	strpath += "luascript\\";
	strpath += filename;
	++m_callFromLua;
	int nRet = luaL_dofile(m_state, strpath.c_str());
	--m_callFromLua;
	if (nRet != 0)
	{
		XH_LOG_ERROR(xhnet::logname_base, "[LUA ERROR] " << lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
	}
	return nRet;
}

int	CLuaEngine::executeScriptString(const char* luastring)
{
	++m_callFromLua;
	int nRet = luaL_dostring(m_state, luastring);
	--m_callFromLua;
	if (nRet != 0)
	{
		XH_LOG_ERROR(xhnet::logname_base, "[LUA ERROR] " << lua_tostring(m_state, -1));
		lua_pop(m_state, 1);
	}
	return nRet;
}

void CLuaEngine::clean()
{
	lua_settop(m_state, 0);
}

int CLuaEngine::executeFunction( int numArgs )
{
	int functionIndex = -(numArgs + 1);
	if (!lua_isfunction(m_state, functionIndex))
	{
		XH_LOG_ERROR(xhnet::logname_base, "[LUA ERROR] executeFunction 栈中第" << functionIndex << "个元素不是函数");
		lua_pop(m_state, numArgs + 1); // remove function and arguments
		return 0;
	}

	int traceback = 0;
	lua_getglobal(m_state, "__G__TRACKBACK__");                         /* L: ... func arg1 arg2 ... G */
	if (!lua_isfunction(m_state, -1))
	{
		lua_pop(m_state, 1);                                            /* L: ... func arg1 arg2 ... */
	}
	else
	{
		lua_insert(m_state, functionIndex - 1);                         /* L: ... G func arg1 arg2 ... */
		traceback = functionIndex - 1;
	}

	int error = 0;
	++m_callFromLua;
	error = lua_pcall(m_state, numArgs, 1, traceback);                  /* L: ... [G] ret */
	--m_callFromLua;
	if (error)
	{
		if (traceback == 0)
		{
			XH_LOG_ERROR(xhnet::logname_base, "[LUA ERROR] %s" << lua_tostring(m_state, -1));        /* L: ... error */
			lua_pop(m_state, 1); // remove error message from stack
		}
		else                                                            /* L: ... G error */
		{
			lua_pop(m_state, 2); // remove __G__TRACKBACK__ and error message from stack
		}
		return 0;
	}

	// get return value
	int ret = 0;
	if (lua_isnumber(m_state, -1))
	{
		ret = lua_tointeger(m_state, -1);
	}
	else if (lua_isboolean(m_state, -1))
	{
		ret = lua_toboolean(m_state, -1);
	}
	// remove return value from stack
	lua_pop(m_state, 1);                                                /* L: ... [G] */

	if (traceback)
	{
		lua_pop(m_state, 1); // remove __G__TRACKBACK__ from stack      /* L: ... */
	}

	return ret;
}

void CLuaEngine::pushInt( int intValue )
{
	lua_pushinteger(m_state, intValue);
}

void CLuaEngine::pushFloat( float floatValue )
{
	lua_pushnumber(m_state, floatValue);
}

void CLuaEngine::pushBoolean( bool boolValue )
{
	lua_pushboolean(m_state, boolValue);
}

void CLuaEngine::pushString( const char* stringValue )
{
	lua_pushstring(m_state, stringValue);
}

void CLuaEngine::pushString( const char* stringValue, int length )
{
	lua_pushlstring(m_state, stringValue, length);
}

//void CLuaEngine::pushResolveObj( ResolveAction* pValue )
//{
//	lua_pushlightuserdata(m_state, pValue);
//}

void CLuaEngine::pushNil( void )
{
	lua_pushnil(m_state);
}

bool CLuaEngine::pushFunctionByName( const char* functionname )
{
	if(functionname == NULL)
		return false;

	lua_getglobal(m_state, functionname);
	if (!lua_isfunction(m_state, -1))
	{
		XH_LOG_ERROR(xhnet::logname_base, "[LUA ERROR] pushFunctionByName 无法找到Lua函数 " << functionname);
		lua_pop(m_state, 1);
		return false;
	}
	return true;
}

void CLuaEngine::addSearchPath( const char* path )
{
	lua_getglobal(m_state, "package");                                  /* L: package */
	lua_getfield(m_state, -1, "path");                /* get package.path, L: package path */
	const char* cur_path =  lua_tostring(m_state, -1);
	lua_pushfstring(m_state, "%s;%s\\?.lua", cur_path, path);            /* L: package path newpath */
	lua_setfield(m_state, -3, "path");          /* package.path = newpath, L: package path */
	lua_pop(m_state, 2);        
}

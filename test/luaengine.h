#ifndef LuaEngine_h__
#define LuaEngine_h__

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class CLuaEngine
{
public:
	CLuaEngine();
	~CLuaEngine(void);  
    
	lua_State*	getLuaState(void) { return m_state; }

	bool		init(unsigned int level);
	void		clean();
	void		addSearchPath(const char* path);

	int			executeScriptFile(const char* filename);
	int			executeScriptString(const char* luastring);
	int			executeFunction(int numArgs);

	void 		pushInt(int intValue);
	void 		pushFloat(float floatValue);
	void 		pushBoolean(bool boolValue);
	void 		pushString(const char* stringValue);
	void 		pushString(const char* stringValue, int length);
//	void		pushResolveObj(ResolveAction* pValue);
	void 		pushNil(void);
	bool		pushFunctionByName(const char* functionname);

	lua_State*		m_state;
	unsigned int	m_level;
	int				m_callFromLua;
	bool			m_bInit;
};


#endif // LuaEngine_h__
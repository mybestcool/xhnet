/*
** Lua binding: mainqyfh
** Generated automatically by tolua++-1.0.92 on 12/30/14 15:58:22.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_mainqyfh_open (lua_State* tolua_S);

#include "main-qyfh.h"
#include "test-qyfh.h"
#include "qyfh.h"
using namespace xhnet_qyfh;

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_CTestNetIO (lua_State* tolua_S)
{
 CTestNetIO* self = (CTestNetIO*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CTestNetIO");
}

/* method: new of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_new00
static int tolua_mainqyfh_CTestNetIO_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CTestNetIO* tolua_ret = (CTestNetIO*)  Mtolua_new((CTestNetIO)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CTestNetIO");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_new00_local
static int tolua_mainqyfh_CTestNetIO_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CTestNetIO* tolua_ret = (CTestNetIO*)  Mtolua_new((CTestNetIO)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CTestNetIO");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_delete00
static int tolua_mainqyfh_CTestNetIO_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetNetIO of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_GetNetIO00
static int tolua_mainqyfh_CTestNetIO_GetNetIO00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CTestNetIO* tolua_ret = (CTestNetIO*)  CTestNetIO::GetNetIO();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CTestNetIO");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetNetIO'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetConnectIP of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_GetConnectIP00
static int tolua_mainqyfh_CTestNetIO_GetConnectIP00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetConnectIP'", NULL);
#endif
  {
   std::string tolua_ret = (std::string)  self->GetConnectIP();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetConnectIP'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: GetConnectPort of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_GetConnectPort00
static int tolua_mainqyfh_CTestNetIO_GetConnectPort00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'GetConnectPort'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->GetConnectPort();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetConnectPort'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Start of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_Start00
static int tolua_mainqyfh_CTestNetIO_Start00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
  const std::string listenip = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  unsigned int listenport = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
  const std::string connectip = ((const std::string)  tolua_tocppstring(tolua_S,4,0));
  unsigned int connectport = ((unsigned int)  tolua_tonumber(tolua_S,5,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Start'", NULL);
#endif
  {
   self->Start(listenip,listenport,connectip,connectport);
   tolua_pushcppstring(tolua_S,(const char*)listenip);
   tolua_pushcppstring(tolua_S,(const char*)connectip);
  }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Start'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Close of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_Close00
static int tolua_mainqyfh_CTestNetIO_Close00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Close'", NULL);
#endif
  {
   self->Close();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Close'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Stop of class  CTestNetIO */
#ifndef TOLUA_DISABLE_tolua_mainqyfh_CTestNetIO_Stop00
static int tolua_mainqyfh_CTestNetIO_Stop00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CTestNetIO",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CTestNetIO* self = (CTestNetIO*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Stop'", NULL);
#endif
  {
   self->Stop();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Stop'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_mainqyfh_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CTestNetIO","CTestNetIO","",tolua_collect_CTestNetIO);
  #else
  tolua_cclass(tolua_S,"CTestNetIO","CTestNetIO","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CTestNetIO");
   tolua_function(tolua_S,"new",tolua_mainqyfh_CTestNetIO_new00);
   tolua_function(tolua_S,"new_local",tolua_mainqyfh_CTestNetIO_new00_local);
   tolua_function(tolua_S,".call",tolua_mainqyfh_CTestNetIO_new00_local);
   tolua_function(tolua_S,"delete",tolua_mainqyfh_CTestNetIO_delete00);
   tolua_function(tolua_S,"GetNetIO",tolua_mainqyfh_CTestNetIO_GetNetIO00);
   tolua_function(tolua_S,"GetConnectIP",tolua_mainqyfh_CTestNetIO_GetConnectIP00);
   tolua_function(tolua_S,"GetConnectPort",tolua_mainqyfh_CTestNetIO_GetConnectPort00);
   tolua_function(tolua_S,"Start",tolua_mainqyfh_CTestNetIO_Start00);
   tolua_function(tolua_S,"Close",tolua_mainqyfh_CTestNetIO_Close00);
   tolua_function(tolua_S,"Stop",tolua_mainqyfh_CTestNetIO_Stop00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_mainqyfh (lua_State* tolua_S) {
 return tolua_mainqyfh_open(tolua_S);
};
#endif


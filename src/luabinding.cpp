/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL                  *
 *   Copyleft (C) 2006 by Pal Lockheart                                    *
 *   palxex@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, If not, see                          *
 *   <http://www.gnu.org/licenses/>.                                       *
 ***************************************************************************/
/******************************************************************************
* Copyright (C) 1994-2008 Lua.org, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
#ifdef WITH_LUA

//A temprature workaround for vc2008 express
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='X86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")

#include <iostream>
#include <string>
#include <cstdio>
using namespace std;

#define luaall_c
#ifndef WIN32
#define LUA_USE_LINUX
#endif
#include "luabinding.h"
#include <luabind/detail/class_registry.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/copy_policy.hpp>
#include <luabind/adopt_policy.hpp>

using namespace luabind;

#include "resource.h"
#include "game.h"
#include "internal.h"
#include "timing.h"
#include "internal.h"
#include "config.h"

#ifdef WIN32
	#include <windows.h>
	HANDLE thread_id;
#else
	#include <pthread.h>
	pthread_t thread_id;
#endif

#include <signal.h>

extern void process_script_entry(uint16_t func,int16_t param1,int16_t param2,int16_t param3,uint16_t &id,int16_t object);
bool hasConsole=false;
void set_rpg(int offset,int16_t data)
{
	if(offset%2){
        cout<<"Please note this function only works on WORD boundary."<<endl;
	}
	else
		reinterpret_cast<uint16_t*>(&Pal::rpg)[offset/2]=data;
}
int get_rpg(int offset)
{
	if(offset%2){
        cout<<"Please note this function only works on WORD boundary."<<endl;
        return 12345678;
	}
	else
		return reinterpret_cast<uint16_t*>(&Pal::rpg)[offset/2];
}

int get_data(uint8_t *ref,int offset){
	if(offset%2){
        cout<<"Please note this function only works on WORD boundary."<<endl;
        return 12345678;
	}
	return reinterpret_cast<int16_t*>(ref)[offset];
}
void set_data(uint8_t *ref,int offset,int16_t data){
	if(offset%2){
        cout<<"Please note this function only works on WORD boundary."<<endl;
	}
	reinterpret_cast<int16_t*>(ref)[offset]=data;
}

namespace{

    char *progname="palx";
    lua_State *glbl_L;

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}

static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(glbl_L, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  glbl_L=L;
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  if (lua_readline(L, b, prmt) == 0)
    return 0;  /* no input */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  lua_freeline(L, b);
  return 1;
}

static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=stdin");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_saveline(L, 1);
  lua_remove(L, 1);  /* remove line */
  return status;
}

void registerExports(lua_State *L){
	if(hasConsole){
		open(L);
		//manual register uint8_t *
		lua_pushstring(L, "uint8_t");
		detail::class_rep* crep;
		detail::class_registry* r = detail::class_registry::get_registry(L);
		lua_newuserdata(L, sizeof(detail::class_rep));
		crep = reinterpret_cast<detail::class_rep*>(lua_touserdata(L, -1));
		new(crep) detail::class_rep(&typeid(uint8_t), "uint8_t", L, NULL, NULL, &typeid(detail::null_type)
				, &typeid(detail::you_need_to_define_a_get_const_holder_function_for_your_smart_ptr), NULL, NULL, NULL, NULL, NULL, NULL, NULL
				, NULL, 0, 1);
		r->add_class(&typeid(uint8_t), crep);

		module(L)[
			class_<cached_res>("cached_res")
				.def(constructor<>())
				.def("set",&cached_res::set)
				.def("decode",(uint8_t*(cached_res::*)(int,long&))&cached_res::decode,pure_out_value(_3))
				.def("clear",(void (cached_res::*)())&cached_res::clear)
			, //luabind 0.71 bug?
			/*class_<rpg_def>("rpg_def")
				.def_readwrite("save_times",&rpg_def::save_times)
				.def_readwrite("viewport_x",&rpg_def::viewport_x)
				.def_readwrite("viewport_y",&rpg_def::viewport_y)
				.def_readwrite("team_roles",&rpg_def::team_roles)
				.def_readwrite("scene_id",&rpg_def::scene_id)
				.def_readwrite("palette_offset",&rpg_def::palette_offset)
				.def_readwrite("team_direction",&rpg_def::team_direction)
				.def_readwrite("music",&rpg_def::music)
				.def_readwrite("battle_music",&rpg_def::battle_music)
				.def_readwrite("battlefield",&rpg_def::battlefield)
				.def_readwrite("wave_grade",&rpg_def::wave_grade)
				.def_readwrite("reserved",&rpg_def::reserved)
				.def_readwrite("gourd_value",&rpg_def::gourd_value)
				.def_readwrite("layer",&rpg_def::layer)//?
				.def_readwrite("chase_range",&rpg_def::chase_range)
				.def_readwrite("chasespeed_change_cycles",&rpg_def::chasespeed_change_cycles)
				.def_readwrite("team_followers",&rpg_def::team_followers)
				.def_readwrite("coins",&rpg_def::coins)
				.scope[
					class_<rpg_def::position>("position")
						.def_readwrite("role",&rpg_def::position::role)
						.def_readwrite("x",&rpg_def::position::x)
						.def_readwrite("y",&rpg_def::position::y)
						.def_readwrite("frame",&rpg_def::position::frame)
				]
				//.def_readwrite("team",&rpg_def::team)
			,*/
			class_<global_settings>("setting")
				.def("getint",&global_settings::get<int>)
				.def("getbool",&global_settings::get<bool>)
				.def("getstring",&global_settings::get<string>)
				.def("setint",&global_settings::set<int>)
				.def("setbool",&global_settings::set<bool>)
				.def("setstring",&global_settings::set<string>)
				.def("instance",&global_settings::instance,adopt(result))
			,
			def("process_script",&process_script),
			def("process_script_entry",&process_script_entry,out_value(_5)),
			def("set_rpg",&set_rpg),
			def("get_rpg",&get_rpg),
			def("get_data",&get_data),
			def("set_data",&set_data)
		];
		globals(L)["ABC"]=boost::ref(Pal::ABC);
		globals(L)["MAP"]=boost::ref(Pal::MAP);
		globals(L)["GOP"]=boost::ref(Pal::GOP);
		globals(L)["RNG"]=boost::ref(Pal::RNG);
		globals(L)["DATA"]=boost::ref(Pal::DATA);
		globals(L)["SSS"]=boost::ref(Pal::SSS);
		globals(L)["BALL"]=boost::ref(Pal::BALL);
		globals(L)["RGM"]=boost::ref(Pal::RGM);
		globals(L)["FBP"]=boost::ref(Pal::FBP);
		globals(L)["F"]=boost::ref(Pal::F);
		globals(L)["FIRE"]=boost::ref(Pal::FIRE);
		globals(L)["MGO"]=boost::ref(Pal::MGO);
		globals(L)["PAT"]=boost::ref(Pal::PAT);
		globals(L)["RPG"]=boost::ref(Pal::rpg);
		globals(L)["config"]=boost::ref(*global_settings::instance());
	}
}
};

lua_State *luabinding::L;

luabinding::luabinding(){
	L=lua_open();
}
luabinding::~luabinding(){
#ifdef WIN32
	CloseHandle(thread_id);
	if(hasConsole)
	{
		FreeConsole();
	}
#else
	pthread_join(thread_id,0);
#endif
	lua_close(L);
}
void luabinding::callback()
{
#ifdef WIN32
	if(hasConsole=global->get<bool>("debug","console"))
	{
		AllocConsole();
		freopen("CONOUT$","w+t",stdout);
		freopen("CONOUT$","w+t",stderr);
		freopen("CONIN$","r+t",stdin);
	}
#endif
	if(!hasConsole)
		return;

	lua_State *console_L=lua_open();
	lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(console_L);  /* open libraries */
	lua_gc(L, LUA_GCRESTART, 0);
	registerExports(console_L);
  int status;
  while (((status = loadline(console_L)) != -1) && running) {
    if (status == 0) status = docall(console_L, 0, 0);
    report(console_L, status);
    if (status == 0 && lua_gettop(console_L) > 0) {  // any result to print?
      lua_getglobal(console_L, "print");
      lua_insert(console_L, 1);
      if (lua_pcall(console_L, lua_gettop(console_L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(console_L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(console_L, -1)));
    }
    delay(10);
  }
  lua_settop(console_L, 0);  // clear stack
  fputs("\n", stdout);
  fflush(stdout);
  lua_close(console_L);
}
void luabinding::init()
{
	lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(L);  /* open libraries */
	lua_gc(L, LUA_GCRESTART, 0);

	registerExports(L);

	#ifdef WIN32
		thread_id = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)callback, NULL, 0, NULL);
	#else
	int i=0;
		pthread_create(&thread_id, NULL, (void* (*)(void*))callback, (void *)i);
	#endif
}
#endif


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
#ifdef WITH_LUA

#include <iostream>
#include <string>
#include <cstdio>
using namespace std;

#include "resource.h"
#include "internal.h"
#include "timing.h"
#include "luabinding.h"
#include <luabind/out_value_policy.hpp>

using namespace luabind;

#ifdef WIN32
	#include <windows.h>
	HANDLE thread_id;
#else
	#include <pthread.h>
	pthread_t thread_id;
#endif

lua_State *luabinding::L;

luabinding::luabinding(){
	L=lua_open();
}
luabinding::~luabinding(){
#ifdef WIN32
	CloseHandle(thread_id);
#else
	pthread_join(thread_id,0);
#endif
	lua_close(L);
}
void luabinding::callback()
{
	while(running)
	{
		static char input[1000];
		memset(input,0,1000);
		cout<<"callback begin"<<endl;
		if(cin.getline(input,1000) && strlen(input)){
			luaL_dostring(L,input);
		}
		delay(10);
    }
}

void luabinding::init()
{
	lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(L);  /* open libraries */
	lua_gc(L, LUA_GCRESTART, 0);

    open(L);
	module(L)[
        class_<cached_res>("cached_res")
            .def(constructor<>())
            .def("set",&cached_res::set)
            .def("decode",(uint8_t*(cached_res::*)(int,long&))&cached_res::decode,out_value(_3))
            .def("clear",(void (cached_res::*)())&cached_res::clear)
		,
		def("process_script",&process_script)
    ];
	globals(L)["ABC"]=Pal::ABC;

	luaL_dostring(L,"print(\"lua engine started\\n\")");

	#ifdef WIN32
		thread_id = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)callback, NULL, 0, NULL);
	#else
		pthread_create(&thread_id, NULL, (void *)callback, (void *)i);
	#endif
}
#endif


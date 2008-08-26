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
#include "luabinding.h"

using namespace luabind;
using namespace std;

#include "resource.h"


lua_State *luabinding::L;
void luabinding::callback()
{
	luaL_dostring(L,"print(\"Lua Callback begin:\\n\")");
    luaL_dostring(L,"print(ABC)");
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
            .def("decode",(uint8_t*(cached_res::*)(int,long&))&cached_res::decode)
            .def("clear",(void (cached_res::*)())&cached_res::clear)
    ];
	globals(L)["ABC"]=boost::cref(Pal::ABC);

	luaL_dostring(L,"print(\"lua engine started\\n\")");
}
#endif


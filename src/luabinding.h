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
 #ifndef LUABINDING_H
 #define LUABINDING_H

#ifdef WITH_LUA
extern "C"
{
   #include <lua.h>
   #include <lualib.h>
   #include <lauxlib.h>
}
#include <luabind/luabind.hpp>

 class luabinding{
     static lua_State *L;
     public:
     luabinding(){
         L=lua_open();
     }
     ~luabinding(){
         lua_close(L);
     }
     void init();
     static void callback();
 };
 #else
 struct luabinding{
     void init(){}
     static void callback(){}
 };
 #endif

 #endif //LUABINDING_H

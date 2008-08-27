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
#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

enum PAL_VKEY { PAL_VK_NONE=0,PAL_VK_MENU=1,PAL_VK_EXPLORE,PAL_VK_DOWN,PAL_VK_LEFT,PAL_VK_UP,PAL_VK_RIGHT,PAL_VK_PGUP,PAL_VK_PGDN,PAL_VK_REPEAT,PAL_VK_AUTO,PAL_VK_DEFEND,PAL_VK_USE,PAL_VK_THROW,PAL_VK_QUIT,PAL_VK_STATUS,PAL_VK_FORCE,PAL_VK_PRINTSCREEN};
PAL_VKEY async_getkey();
PAL_VKEY sync_getkey();

PAL_VKEY get_key_lowlevel();
void key_watcher(int scancode);

#endif // KEYBOARD_H_INCLUDED

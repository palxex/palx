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
#ifndef _ITEM_H
#define _ITEM_H

int compact_items();
int compact_magic(int role);
void learnmagic(bool flag_dialog,int magic,int role);
int get_magic_pos(int role,int magic);
void add_goods_to_list(int goods,int num);
int count_item(int item,bool hasequip=true);
int get_cons_attrib(int role,int attrib);
void use_item(int item,int amount);

extern int role_parts[6][18][74];
extern int prev_equip;

#endif


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
#ifndef UI_H
#define UI_H

#include "allegdef.h"
#include "internal.h"
#include "game.h"
#include "scene.h"

#include <vector>
#include <string>

extern void show_wait_icon();
extern void dialog_string(const char *str,int x,int y,int color,bool shadow,BITMAP *bmp=screen);
extern void draw_oneline_m_text(char *str,int x,int y);

class dialog
{
	sprite *border[3][3];
public:
	dialog(int style,int x,int y,int rows,int columns,bool =true);
	friend class menu;
};

class menu
{
	dialog menu_dialog;
	std::vector<std::string> menu_items;
	int text_x,text_y;
	int got;
	int selected;
	int color_selecting;
public:
	menu(int x,int y,int menus,int begin,int chars);
	menu(int x,int y,std::vector<std::string> &strs,int chars);
	int operator()(int selected=0);
	virtual void prev_action();
	virtual void post_action();
	virtual void draw();
	virtual int select();
};

class single_dialog
{
	sprite *border[3];
public:
	single_dialog(int x,int y,int len,BITMAP *bmp=screen);
};

int select_rpg(int =0,BITMAP * =screen);
int select_item(int mask,int ,int selected);
void num2pic(int num,int x,int y);
#endif

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
	dialog(int style,int x,int y,int rows,int columns,bool =true,BITMAP * =screen);
	friend struct menu;
};

struct menu_tmp;
struct menu
{
    bitmap bak;
	dialog menu_dialog;
	std::vector<std::string> menu_items;
	int text_x,text_y;
public:
	menu(int x,int y,int menus,int begin,int chars,int style=0,bool shadow=true);
	menu(int x,int y,std::vector<std::string> &strs,int chars,int length=-1,int =0);
	int operator()(const menu_tmp &,int selected=0);
};
struct menu_tmp
{
	int got;
	int selected;
	int color_selecting;
	menu_tmp():got(-1),color_selecting(0xFA){}
	virtual void prev_action(menu*)=0;
	virtual void post_action(menu*)=0;
	virtual void draw(menu*)=0;
	virtual int select(menu*,int)=0;
	virtual int keyloop(menu*)=0;
	virtual ~menu_tmp(){}
};
struct single_menu:public menu_tmp
{
	void prev_action(menu*);
	void post_action(menu*);
	void draw(menu*);
	int select(menu*,int);
	int keyloop(menu*);
};
struct multi_menu:public menu_tmp
{
	int mask,skip,max,max_ori,begin_y,paging,middle;
	bitmap buf;
	multi_menu(int _mask,int _skip,int _paging=8):menu_tmp(),mask(_mask),skip(_skip),paging(_paging),middle(paging/2),buf(0,SCREEN_W,SCREEN_H){got=0;}
	void prev_action(menu*);
	void post_action(menu*);
	void draw(menu*);
	int select(menu*,int);
	int keyloop(menu*);
};

class single_dialog
{
	sprite *border[3];
	BITMAP *cache;
public:
	single_dialog(int x,int y,int len,BITMAP *bmp,bool shadow=true);
	void to_screen();
};


int select_rpg(int =0,BITMAP * =screen);
int select_item(int mask,int ,int selected);
int select_theurgy(int role,int mask,int selected);
void show_money(int num,int x,int y,int text,bool shadow);
void show_num_lim(int num,int x,int y,int digits,BITMAP *bmp=screen);
void show_num_han(int num,int x,int y,int digits,BITMAP *bmp=screen,bool shadow=true);
void show_number(int number,int x,int y,int color,BITMAP *bmp=screen);
void display_role_status(int flag,int role,int x,int y,BITMAP *bmp=screen);

int yes_or_no(int word,int selected);

void shop(int);
void hockshop();
#endif

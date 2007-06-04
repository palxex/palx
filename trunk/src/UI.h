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
	dialog(int style,int x,int y,int rows,int columns);
	friend class menu;
};

class menu
{
	dialog menu_dialog;
	std::vector<std::string> menu_items;
	int text_x,text_y;
public:
	menu(int x,int y,int menus,int begin,int chars);
	int select(int selected=0);
};

class single_dialog
{
	sprite *border[3];
public:
	single_dialog(int x,int y,int len,BITMAP *bmp=screen);
};

int select_rpg(int =0,BITMAP * =screen);
void num2pic(int num,int x,int y);
#endif
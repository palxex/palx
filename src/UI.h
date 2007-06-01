#include "allegdef.h"
#include "internal.h"
#include "game.h"
#include "scene.h"

#include <vector>
#include <string>

class dialog
{
	sprite *border[3][3];
public:
	dialog(int style,int x,int y,int rows,int columns);
};

class menu
{
	dialog menu_dialog;
	std::vector<std::string> menu_items;
public:
	menu(int x,int y,int menus,int begin)
		:dialog(0,x,y,menus,2)
	{
		for(int i=begin;i<begin+menus;i++)
			menu_items.push_back(string(cut_msg_impl("word.dat")(i*10,(i+1)*10)));
	}
	int select()
	{
	}
};
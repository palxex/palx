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
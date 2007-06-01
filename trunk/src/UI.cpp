#include "UI.h"

dialog::dialog(int style,int x,int y,int rows,int columns)
{
	rows--;columns--;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			border[i][j]=game->UIpics.getsprite(i*3+j+style);
	int len=0;
	for(int i=0;i<2+rows;i++)
	{
		int ti,j=0;
		if(i==0)
			ti=0;
		else if(i==1+rows)
				ti=2;
		else
			ti=1; 
		border[ti][0]->blit_to(screen,x,y+len);
		for(;j<columns;j++)
			border[ti][1]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len);
		border[ti][2]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len);
		len+=border[ti][1]->height;
	}
}

menu::menu(int x,int y,int menus,int begin,int chars)
	:menu_dialog(0,x,y,menus,chars),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	for(int i=begin;i<begin+menus;i++)
		menu_items.push_back(std::string(cut_msg_impl("word.dat")(i*10,(i+1)*10)));
}
int menu::select(int selected)
{
	int color_selecting=0xFA;
	int key=0,ok=-1,color;
	do{
		int i=0;
		for(std::vector<std::string>::iterator r=menu_items.begin();r!=menu_items.end();r++,i++)
		{
			if(i==selected)
				color=color_selecting;
			else
				color=0x4E;
			ttfont(r->c_str()).blit_to(screen,text_x,text_y+18*i,color);
		}
		if(ok)
			while(!(key=get_key())) rest(10);
		switch(key){
			case VK_UP:
				selected--;
				break;
			case VK_DOWN:
				selected++;
				break;
			case VK_MENU:
				return -1;
			case VK_EXPLORE:
				ok=1;key=0;
				color_selecting=0x2B;
				break;
		}
		selected+=menu_items.size();
		selected%=menu_items.size();
	}while(ok--);
	return selected;
}
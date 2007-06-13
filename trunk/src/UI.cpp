#include "UI.h"
#include "timing.h"

#include <boost/lexical_cast.hpp>

dialog::dialog(int style,int x,int y,int rows,int columns,bool shadow)
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
		border[ti][0]->blit_to(screen,x,y+len,shadow);
		for(;j<columns;j++)
			border[ti][1]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len,shadow);
		border[ti][2]->blit_to(screen,x+border[ti][0]->width+j*border[ti][1]->width,y+len,shadow);
		len+=border[ti][1]->height;
	}
}

single_dialog::single_dialog(int x,int y,int len,BITMAP *bmp)
{
	int i=0;
	for(i=0;i<3;i++)
		border[i]=game->UIpics.getsprite(44+i);
	border[0]->blit_to(bmp,x,y,true);
	for(i=0;i<len;i++)
		border[1]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,true);
	border[2]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,true);
}

int select_rpg(int ori_select,BITMAP *bmp)
{
	static cut_msg_impl word("word.dat");
	int selected=ori_select;
	BITMAP *cache=create_bitmap(SCREEN_W,SCREEN_H);
	selected=(selected>=1?selected:1);
	int ok=1;
	std::vector<std::string> menu_items;
	blit(bmp,cache,0,0,0,0,SCREEN_W,SCREEN_H);
	do{
		for(int r=0;r<5;r++){			
			single_dialog(0xB4,4+0x26*r,6,cache);
			dialog_string((std::string(word(0x1AE,0x1B2))+boost::lexical_cast<std::string>((selected-1)/5*5+r+1)).c_str(),0xBE,14+0x26*r,r==(selected-1)%5?0xFA:0,r==(selected-1)%5,cache);
		}
		blit(cache,bmp,0,0,0,0,SCREEN_W,SCREEN_H);
		while(!get_key()) delay(10);
		switch(keygot){
			case VK_UP:
				selected--;
				break;
			case VK_DOWN:
				selected++;
				break;
			case VK_PGUP:
				selected-=5;
				break;
			case VK_PGDN:
				selected+=5;
				break;
			case VK_MENU:
				return 0;
			case VK_EXPLORE:
				ok=0;
				break;
		}
		selected=(selected<=1?1:selected);
	}while(ok);
	destroy_bitmap(cache);
	return selected;
}

menu::menu(int x,int y,int menus,int begin,int chars)
	:menu_dialog(0,x,y,menus,chars),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	for(int i=begin;i<begin+menus;i++)
		menu_items.push_back(std::string(cut_msg_impl("word.dat")(i*10,(i+1)*10)));
}
int menu::select(int selected)
{
	selected=(selected>=0?selected:0);
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
			dialog_string(r->c_str(),text_x,text_y+18*i,color,true);
		}
		if(ok)
			while(!get_key()) wait(10);
		else
			break;
		switch(keygot){
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
		selected+=(int)menu_items.size();
		selected%=menu_items.size();
	}while(ok--);
	return selected;
}

int select_item(int mask,int skip,int selected)
{
	dialog(9,2,33,8,18,false);
	wait_for_key();
	return selected;
}
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
#include "UI.h"
#include "timing.h"
#include "structs.h"

#include <boost/lexical_cast.hpp>

#define SAFE_GETKEY(x) \
	do{ \
		while(!(x=get_key())) \
		{ \
			extern bool running; \
			if(!running) \
				if(starting) \
					throw new std::exception(); \
				else \
					return -1; \
			switch_proc(); \
			rest(10); \
		} \
	}while(false)

static cut_msg_impl word("word.dat");

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

		VKEY keygot;
		SAFE_GETKEY(keygot);
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
	:menu_dialog(0,x,y,menus,chars,false),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	for(int i=begin;i<begin+menus;i++)
		menu_items.push_back(std::string(word(i*10,(i+1)*10)));
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
		VKEY keygot;
		if(ok)
			SAFE_GETKEY(keygot);
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
	int max=compact_items(),max_ori=max,begin_y;

	if(!skip)//装备中的土灵珠等}
		for(int i=0;i<=game->rpg.team_roles;i++)
			for(int j=0xB;j<=0x10;j++)
				if(rpg.objects[((roles*)&game->rpg.roles_properties)[j][i]].param & 1)
//only available on gcc					game->rpg.items[max++]=(RPG::ITEM){((roles*)&game->rpg.roles_properties)[j][i],1,0};
				{
					RPG::ITEM it;it.item=((roles*)&game->rpg.roles_properties)[j][i];it.amount=1;it.using_amount=0;
					game->rpg.items[max++]=it;
				}
	if(skip==-1)
		begin_y=-8;
	else
		begin_y=33;


	static int const paging=8,middle=paging/2;static int locating=selected;//8 for dos,98 maybe 7?
	static bitmap buf(0,SCREEN_W,SCREEN_H),bak(0,SCREEN_W,SCREEN_H);

	dialog(9,2,33,paging,18,false);//DOS ver;for item should use 98 ver.
	blit(screen,bak,0,0,0,0,SCREEN_W,SCREEN_H);
	bool ok=false;int color_selecting,key;
	VKEY keygot;
	do{
		static int offset=0,pre_locate=0;
		offset=(locating/3<middle?0:locating/3-middle);
		blit(bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x100 && game->rpg.items[r].item)
				ttfont(word(game->rpg.items[r].item*10)).blit_to(buf,16+100*(r%3),begin_y+12+(r/3-offset)*18,(r==locating)?((game->rpg.objects[game->rpg.items[r].item].param&mask)?0xFA:0x1C):(r<=max_ori?((game->rpg.objects[game->rpg.items[r].item].param&mask)?0x4E:0x18):0xC8),true);
		game->UIpics.getsprite(69)->blit_to(buf,16+100*(locating%3)+24,begin_y+12+(locating/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
		SAFE_GETKEY(keygot);
		switch(keygot){
			case VK_UP:
				locating-=3;
				break;
			case VK_DOWN:
				locating+=3;
				break;
			case VK_LEFT:
				locating--;
				break;
			case VK_RIGHT:
				locating++;
				break;
			case VK_PGUP:
				locating-=middle*3;
				break;
			case VK_PGDN:
				locating+=middle*3;
				break;
			case VK_MENU:
				return -1;
			case VK_EXPLORE:
				ok=1;key=0;
				color_selecting=0x2B;
				break;
		}
		locating=(locating<0?0:(locating>max-1?max-1:locating));
	}while(keygot!=VK_MENU && !ok);

	for(int i=max_ori;i<max;i++)
		game->rpg.items[i].item=0,
		game->rpg.items[i].amount=0,
		game->rpg.items[i].using_amount=0;

	return locating;
}

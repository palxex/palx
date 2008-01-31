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

dialog::dialog(int style,int x,int y,int rows,int columns,bool shadow,BITMAP *bmp)
{
	rows--;columns--;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			border[i][j]=res::UIpics.getsprite(i*3+j+style);
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
		border[ti][0]->blit_to(bmp,x,y+len,shadow);
		for(;j<columns;j++)
			border[ti][1]->blit_to(bmp,x+border[ti][0]->width+j*border[ti][1]->width,y+len,shadow);
		border[ti][2]->blit_to(bmp,x+border[ti][0]->width+j*border[ti][1]->width,y+len,shadow);
		len+=border[ti][1]->height;
	}
}

single_dialog::single_dialog(int x,int y,int len,BITMAP *bmp):cache(bmp)
{
	int i=0;
	for(i=0;i<3;i++)
		border[i]=res::UIpics.getsprite(44+i);
	border[0]->blit_to(bmp,x,y,true);
	for(i=0;i<len;i++)
		border[1]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,true);
	border[2]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,true);
}
void single_dialog::to_screen()
{
    blit(cache,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}

int select_rpg(int ori_select,BITMAP *bmp)
{
	int selected=ori_select;
	bitmap cache(0,SCREEN_W,SCREEN_H);
	selected=(selected>=1?selected:1);
	int ok=1;
	std::vector<std::string> menu_items;
	blit(bmp,cache,0,0,0,0,SCREEN_W,SCREEN_H);
	do{
		for(int r=0;r<5;r++){
			single_dialog(0xB4,4+0x26*r,6,cache);
			dialog_string((std::string(objs(0x1AE,0x1B2))+boost::lexical_cast<std::string>((selected-1)/5*5+r+1)).c_str(),0xBE,14+0x26*r,r==(selected-1)%5?0xFA:0,r==(selected-1)%5,cache);
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
            default:
                break;
		}
		selected=(selected<=1?1:selected);
	}while(ok);
	return selected;
}

menu::menu(int x,int y,int menus,int begin,int chars,int style,bool shadow)
	:bak(screen),menu_dialog(style,x,y,menus,chars,shadow,bak),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	for(int i=begin;i<begin+menus;i++)
		menu_items.push_back(std::string(objs(i*10,(i+1)*10)));
	blit(bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
menu::menu(int x,int y,std::vector<std::string> &strs,int chars,int length,int style)
:bak(screen),menu_dialog(style,x,y,length==-1?strs.size():length,chars,true,bak),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	menu_items.swap(strs);
	blit(bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
int menu::operator()(const menu_tmp &con,int _selected)
{
	return const_cast<menu_tmp&>(con).select(this,_selected);
}
int single_menu::select(menu *abs,int _selected)
{
	selected=(_selected>=0?_selected:0);
	prev_action(abs);
	do{
		draw(abs);
		int s=keyloop(abs);
		if(s==-2)
			break;
		else if(s==-1)
			return -1;
	}while(got--);
	post_action(abs);
	return selected;
}
void single_menu::draw(menu *abs)
{
	static int color=0;
	int i=0;
	for(std::vector<std::string>::iterator r=abs->menu_items.begin();r!=abs->menu_items.end();r++,i++)
	{
		if(i==selected)
			color=color_selecting;
		else
			color=0x4E;
        dialog_string(r->c_str(),abs->text_x,abs->text_y+18*i,color,true,abs->bak);
	}
	blit(abs->bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
int single_menu::keyloop(menu *abs)
{
	VKEY keygot;
	if(got)
		SAFE_GETKEY(keygot);
	else
		return -2;
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
			got=1;
			color_selecting=0x2B;
			break;
        default:
            break;
	}
	selected+=(int)abs->menu_items.size();
	selected%=abs->menu_items.size();
	return selected;
}
void single_menu::prev_action(menu *abs)
{
	blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
}
void single_menu::post_action(menu *abs)
{
    blit(abs->bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}

int multi_menu::select(menu *abs,int _selected)
{
	prev_action(abs);int s;
	selected=((_selected<=max && _selected>=0)?_selected:0);
	do{
		draw(abs);
		s=keyloop(abs);
		if(s==-1)
			return -1;
	}while(!got);
	post_action(abs);
	return selected;
}
void multi_menu::prev_action(menu *abs)
{
	max=compact_items();
	max_ori=max;
	if(!skip)//装备中的土灵珠等}
		for(int i=0;i<=res::rpg.team_roles;i++)
			for(int j=0xB;j<=0x10;j++)
				if(res::rpg.objects[((roles*)&res::rpg.roles_properties)[j][i]].param & 1)
//only available on gcc					res::rpg.items[max++]=(RPG::ITEM){((roles*)&res::rpg.roles_properties)[j][i],1,0};
				{
					RPG::ITEM it;it.item=((roles*)&res::rpg.roles_properties)[j][i];it.amount=1;it.using_amount=0;
					res::rpg.items[max++]=it;
				}
	if(skip==-1)
		begin_y=-8;
	else
		begin_y=33;
	blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
}
void multi_menu::post_action(menu *abs)
{
	for(int i=max_ori;i<max;i++)
		res::rpg.items[i].item=0,
		res::rpg.items[i].amount=0,
		res::rpg.items[i].using_amount=0;
}
void multi_menu::draw(menu *abs)
{
		static int offset=0;
		offset=(selected/3<middle?0:selected/3-middle);
		blit(abs->bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x100 && res::rpg.items[r].item)
				ttfont(objs(res::rpg.items[r].item*10)).blit_to(buf,16+100*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((res::rpg.objects[res::rpg.items[r].item].param&mask)?0xFA:0x1C):(r<=max_ori?((res::rpg.objects[res::rpg.items[r].item].param&mask)?0x4E:0x18):0xC8),true);
		res::UIpics.getsprite(69)->blit_to(buf,16+100*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
int multi_menu::keyloop(menu *abs)
{
	VKEY keygot;
	SAFE_GETKEY(keygot);
		switch(keygot){
			case VK_UP:
				selected-=3;
				break;
			case VK_DOWN:
				selected+=3;
				break;
			case VK_LEFT:
				selected--;
				break;
			case VK_RIGHT:
				selected++;
				break;
			case VK_PGUP:
				selected-=middle*3;
				break;
			case VK_PGDN:
				selected+=middle*3;
				break;
			case VK_MENU:
				return -1;
			case VK_EXPLORE:
				got=1;
				color_selecting=0x2B;
				break;
            default:
                break;
		}
		return selected=(selected<0?0:(selected>max-1?max-1:selected));
}
int select_item(int mask,int skip,int selected)
{
	return menu(2,33,8,0,18,9,false)(multi_menu(mask,skip),selected);
}

struct magic_menu:public multi_menu
{
	int role;
	magic_menu(int _role,int _mask):multi_menu(_mask,0),role(_role){}
	void prev_action(menu *abs){
		max=0x20-std::count_if(res::rpg.role_prop_tables+0x20,res::rpg.role_prop_tables+0x40,rolemagic_select(role,0));
		max_ori=max;
		blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
		begin_y=33;
	}
	void draw(menu *abs){
		int offset=(selected/3<middle?0:selected/3-middle);
		blit(abs->bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x20 && res::rpg.role_prop_tables[0x20+r][role])
				ttfont(objs(res::rpg.role_prop_tables[0x20+r][role]*10)).blit_to(buf,16+100*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((res::rpg.objects[res::rpg.role_prop_tables[0x20+r][role]].param&mask)?0xFA:0x1C):((res::rpg.objects[res::rpg.role_prop_tables[0x20+r][role]].param&mask)?0x4E:0x18),true);
		res::UIpics.getsprite(69)->blit_to(buf,16+100*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	}
};
int select_theurgy(int role,int mask,int selected)
{
	return menu(2,33,8,0,18,9,false)(magic_menu(role,mask),selected);
}

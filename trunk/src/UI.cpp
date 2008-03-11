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

single_dialog::single_dialog(int x,int y,int len,BITMAP *bmp,bool shadow):cache(bmp)
{
	int i=0;
	for(i=0;i<3;i++)
		border[i]=res::UIpics.getsprite(44+i);
	border[0]->blit_to(bmp,x,y,shadow);
	for(i=0;i<len;i++)
		border[1]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,shadow);
	border[2]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,shadow);
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
			dialog_string(objs(0x1AE,0x1B2),0xC5,13+0x26*r,r==(selected-1)%5?0xFA:0,r==(selected-1)%5,cache);
			show_num_han((selected-1)/5*5+r+1,0xC5 + 16*2,13+0x26*r,r==(selected-1)%5?0xFA:0,cache,r==(selected-1)%5);
			FILE *fp=fopen((global->get<std::string>("config","path")+"/"+boost::lexical_cast<std::string>((selected-1)/5*5+r+1)+".RPG").c_str(),"rb");
			if(fp){
				uint16_t saves;
				fread(&saves,2,1,fp);
				fclose(fp);
				show_number(saves,0x10E,19+0x26*r,0,cache);
			}
		}
		blit(cache,bmp,0,0,0,0,SCREEN_W,SCREEN_H);

		switch(sync_getkey()){
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
	}while(running && ok);
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
	}while(running && got--);
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
		keygot=sync_getkey();
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
	}while(running && !got);
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
				if(res::rpg.objects[((roles*)&res::rpg.roles_properties)[j][i]].item.param & 1)
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
		show_money(res::rpg.objects[res::rpg.items[selected].item].item.value /2 ,0,0xE0,0x19,false);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x100 && res::rpg.items[r].item){
				Font->blit_to(objs(res::rpg.items[r].item*10),buf,16+100*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((res::rpg.objects[res::rpg.items[r].item].item.param&mask)?0xFA:0x1C):(r<=max_ori?((res::rpg.objects[res::rpg.items[r].item].item.param&mask)?0x4E:0x18):0xC8),true);
				if(res::rpg.items[r].amount>1)
					show_number(res::rpg.items[r].amount,16+100*(r%3)+84,begin_y+12+(r/3-offset)*18+6,0,buf);
			}
		res::UIpics.getsprite(69)->blit_to(buf,16+100*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
int multi_menu::keyloop(menu *abs)
{
		switch(sync_getkey()){
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
	magic_menu(int _role,int _mask):multi_menu(_mask,0,5),role(_role){}
	void prev_action(menu *abs){
		max=0x20-std::count_if(res::rpg.role_prop_tables+0x20,res::rpg.role_prop_tables+0x40,rolemagic_select(role,0));
		max_ori=max;
		blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
		begin_y=45;
	}
	void draw(menu *abs){
		int offset=(selected/3<middle?0:selected/3-middle);
		blit(abs->bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
		single_dialog(0xB7,7,4,buf,false);
		show_number(res::magics[res::rpg.objects[res::rpg.role_prop_tables[0x20+selected][role]].magic.magic].power_used,0xD6,0x14,0,buf);
		res::UIpics.getsprite(39)->blit_to(buf,0xDE,0x14);
		show_number(res::rpg.roles_properties.MP[role],0xF0,0x14,2,buf);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x20 && res::rpg.role_prop_tables[0x20+r][role])
				Font->blit_to(objs(res::rpg.role_prop_tables[0x20+r][role]*10),buf,34+88*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((res::rpg.objects[res::rpg.role_prop_tables[0x20+r][role]].magic.param&mask)?0xFA:0x1C):((res::rpg.objects[res::rpg.role_prop_tables[0x20+r][role]].magic.param&mask)?0x4E:0x18),true);
		res::UIpics.getsprite(69)->blit_to(buf,34+88*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	}
};
int select_theurgy(int role,int mask,int selected)
{
	return menu(0xA,0x2D,5,0,17,9,false)(magic_menu(role,mask),selected);
}
int yes_or_no(int word,int selected)
{
	bitmap buf(screen);
	bool got=false;
	do{
		single_dialog(0x78,0x64,2,buf).to_screen();
		single_dialog(0xC8,0x64,2,buf).to_screen();
		dialog_string(objs(word*10   ,word*10+10),0x78+0xF,0x64+0x9,(selected==0)?0xFA:0,(selected==0)?true:false);
		dialog_string(objs(word*10+10,word*10+20),0xC8+0xF,0x64+0x9,(selected==1)?0xFA:0,(selected==1)?true:false);
		switch(sync_getkey())
		{
		case VK_MENU:
            return -1;
		case VK_EXPLORE:
			got=true;
			break;
		case VK_LEFT:
			selected--;
			break;
		case VK_RIGHT:
			selected++;
			break;
		default:
			break;
		}
		selected=!!selected;
	}while(running && !got);
	return !!selected;
}
void show_num_lim(int num,int x,int y,int digits,BITMAP *bmp)//unimplemented caller:10941@battle,script6a
{
    if(num<0)
        return;
	x=x+digits*6-6;
	for(int i=0;num>0 && i<digits;num/=10,i++)
		res::UIpics.getsprite(num%10+19)->blit_to(bmp,x-i*6,y);
}
void show_num_han(int num,int x,int y,int color,BITMAP *bmp,bool shadow)
{
	static class digi{
		uint16_t digit[10];
		char buf[10];
	public:
		digi()
		{
			strncpy((char*)&digit[1],objs(43*10+4,43*10+6),2);
			strncpy((char*)&digit[2],objs(44*10+4,44*10+6),2);
			strncpy((char*)&digit[3],objs(45*10+4,45*10+6),2);
			strncpy((char*)&digit[4],objs(46*10+4,46*10+6),2);
			strncpy((char*)&digit[5],objs(47*10+4,47*10+6),2);
			strncpy((char*)&digit[6],objs(0xB2C,0xB2E),2);
			strncpy((char*)&digit[7],objs(0xD52,0xD54),2);
			strncpy((char*)&digit[8],objs(0x155E,0x1560),2);
			strncpy((char*)&digit[9],objs(0x157E,0x1580),2);
			strncpy((char*)&digit[0],objs(0x38E,0x390),2);
		}
		char * operator[](int n)
		{
			memset(buf,0,sizeof(buf));
			strncpy(buf,(const char*)&digit[n],2);
			return (buf);
		}
	}digi_def;
    if(num<0)
        return;
	int digits=boost::lexical_cast<std::string>(num).length();
	x=x+digits*16-16;
	for(int i=0;num>0 && i<digits;num/=10,i++)
		Font->blit_to(digi_def[num%10],bmp,x-i*16,y,color,shadow);
}
void show_number(int number,int x,int y,int color,BITMAP *bmp)
{
	if(number<0)
		return;
	int i=(color==0?19:(color==1?29:(color==2?56:0)));
	x+=6;
	do
		res::UIpics.getsprite(i+number%10)->blit_to(bmp,x-=6,y);
	while(number/=10);
}
void show_money(int num,int x,int y,int text,bool shadow)
{
	single_dialog(x,y,5,bitmap(screen),shadow).to_screen();
	Font->blit_to(objs(text*10,text*10+10),screen,x+10,y+10,0,false);
	show_num_lim(num,x+48,y+15,6);
}

boost::shared_ptr<def_font> Font;


bool process_Menu()
{
	static int main_select=0,role_select=0,magic_select=0,itemuse_select=0,item_select=0,sys_select=0,rpg_select=0,music_selected=(global->get<int>("music","volume")>0),sfx_selected=(global->get<int>("music","volume_sfx")>0);
	show_money(res::rpg.coins,0,0,0x15,true);
	switch(main_select=menu(3,37,4,3,2)(single_menu(),main_select))
	{
	case 0:
		break;
	case 1:
		{
			if(res::rpg.team_roles)
			{
				std::vector<std::string> names;
				for(int i=0;i<=res::rpg.team_roles;i++)
					names.push_back(objs(res::rpg.roles_properties.name[res::rpg.team[i].role]*10));
				role_select=menu(0x2c,0x40,names,3)(single_menu(),role_select);
			}
			else
				role_select=0;
			magic_select=select_theurgy(res::rpg.team[role_select].role,1,magic_select);
		}
		break;
	case 2:
		switch(itemuse_select=menu(0x1e,0x3c,2,0x16,2)(single_menu(),itemuse_select))
		{
		case 0:
			item_select=select_item(2,0,item_select);
			{
				uint16_t &equip_script=res::rpg.objects[res::rpg.items[item_select].item].item.equip;
				process_script(equip_script,0);
			}
			break;
		case 1:
			item_select=select_item(1,0,item_select);
			{
				uint16_t &use_script=res::rpg.objects[res::rpg.items[item_select].item].item.use;
				process_script(use_script,0);
			}
			break;
		}
		break;
	case 3:
		switch(sys_select=menu(0x28,0x3c,5,0xB,4)(single_menu(),sys_select))
		{
		case 0:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				res::save(rpg_to_load);
			break;
		case 1:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				res::load(rpg_to_load);
			else
				return true;
			break;
		case 2:
			if((music_selected=yes_or_no(0x11,music_selected))<0)
                break;
			rix->setvolume((music_selected==1)?255:0);
			break;
		case 3:
			if((sfx_selected=yes_or_no(0x11,sfx_selected))<0)
                break;
			global->set<int>("music","volume_sfx",(sfx_selected==1)?255:0);
			break;
		case 4:
            if(yes_or_no(0x13,0)==1)
                return false;
		}
	}
	clear_keybuf();
	rest(100);
	return running;
}

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
#include "item.h"

#include <boost/lexical_cast.hpp>

using namespace Pal;

dialog::dialog(int style,int x,int y,int rows,int columns,bool shadow,BITMAP *bmp)
{
	rows--;columns--;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			border[i][j]=UIpics.getsprite(i*3+j+style);
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
		border[i]=UIpics.getsprite(44+i);
	border[0]->blit_to(bmp,x,y,shadow);
	for(i=0;i<len;i++)
		border[1]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,shadow);
	border[2]->blit_to(bmp,x+border[0]->width+i*border[1]->width,y,shadow);
}
void single_dialog::to_screen()
{
    blit(cache,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}

menu::menu(int x,int y,int menus,int begin,int chars,int style,bool shadow)
	:bak(screen),menu_dialog(style,x,y,menus,chars,shadow,bak),text_x(x+menu_dialog.border[0][0]->width-8),text_y(y+menu_dialog.border[1][0]->height-8)
{
	for(int i=begin;i<begin+menus;i++)
		menu_items.push_back(std::string(objs(i)));
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
menu::~menu(){
    clear_keybuf();
}

void menu_tmp::prev_action(menu *abs)
{
	blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
}
void menu_tmp::post_action(menu *abs)
{
    blit(abs->bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
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
	PAL_VKEY keygot;
	if(got)
		keygot=sync_getkey();
	else
		return -2;
	switch(keygot){
		case PAL_VK_UP:
			selected--;
			break;
		case PAL_VK_DOWN:
			selected++;
			break;
		case PAL_VK_MENU:
			return -1;
		case PAL_VK_EXPLORE:
			got_action(abs);
			break;
        default:
            break;
	}
	selected+=(int)abs->menu_items.size();
	selected%=abs->menu_items.size();
	return selected;
}
void single_menu::got_action(menu *){
	got=1;
	color_selecting=0x2B;
}

void multi_menu::draw(menu *abs){
	static int offset=0;
	offset=(selected/3<middle?0:selected/3-middle);
	blit(abs->bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
	show_money(rpg.objects[rpg.items[selected].item].item.value /2 ,0,0xE0,0x19,false);
	for(int r=offset*3;r<offset*3+paging*3;r++)
		if(r<0x100 && rpg.items[r].item){
			Font->blit_to(objs(rpg.items[r].item),buf,16+100*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((rpg.objects[rpg.items[r].item].item.param&mask)?0xFA:0x1C):(r<max_ori?((rpg.objects[rpg.items[r].item].item.param&mask)?0x4E:0x18):0xC8),true);
			if(rpg.items[r].amount>1)
				show_number(rpg.items[r].amount,16+100*(r%3)+84,begin_y+12+(r/3-offset)*18+6,0,buf);
		}
	UIpics.getsprite(69)->blit_to(buf,16+100*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
	blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
}
int multi_menu::select(menu *abs,int _selected)
{
	prev_action(abs);
	selected=((_selected<=max && _selected>=0)?_selected:0);
	do{
		draw(abs);
		if(keyloop(abs)==-1)
			return -1;
	}while(running && !got);
	post_action(abs);
	return selected;
}
int multi_menu::keyloop(menu *abs)
{
		switch(sync_getkey()){
			case PAL_VK_UP:
				selected-=3;
				break;
			case PAL_VK_DOWN:
				selected+=3;
				break;
			case PAL_VK_LEFT:
				selected--;
				break;
			case PAL_VK_RIGHT:
				selected++;
				break;
			case PAL_VK_PGUP:
				selected-=middle*3;
				break;
			case PAL_VK_PGDN:
				selected+=middle*3;
				break;
			case PAL_VK_MENU:
				return -1;
			case PAL_VK_EXPLORE:
				got_action(abs);
				break;
            default:
                break;
		}
		return selected=(selected<0?0:(selected>max-1?max-1:selected));
}
void multi_menu::got_action(menu *){
	got=1;
	color_selecting=0x2B;
}

struct magic_menu:public multi_menu
{
	int role;
	bool after;
	magic_menu(int _role,int _mask,bool b=true):multi_menu(_mask,0,5),role(_role),after(b){}
	void prev_action(menu *abs){
		max=compact_magic(role);
		blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
		begin_y=45;
	}
	int select(menu *abs,int _selected)
	{
		prev_action(abs);
		selected=((_selected<=max && _selected>=0)?_selected:0);
		while(running){
			do{
				draw(abs);
				if(keyloop(abs)==-1)
					return -1;
			}while(running && !got);
			got=0;
			if(after)
				post_action(abs);
			else
				break;
		}
		return selected;
	}
	void got_action(menu*){
		if(rpg.objects[rpg.roles_properties.magics[selected][role]].magic.param&mask  && rpg.roles_properties.MP[role]>=magics[rpg.objects[rpg.roles_properties.magics[selected][role]].magic.magic].power_used)
			got=1;
	}
	void draw(menu *abs){
		int offset=(selected/3<middle?0:selected/3-middle);
		blit(abs->bak,buf,0,0,0,0,SCREEN_W,SCREEN_H);
		single_dialog(0xB7,7,4,buf,false);
		show_number(magics[rpg.objects[rpg.roles_properties.magics[selected][role]].magic.magic].power_used,0xD6,0x14,0,buf);
		UIpics.getsprite(39)->blit_to(buf,0xDE,0x14);
		show_number(rpg.roles_properties.MP[role],0xF0,0x14,2,buf);
		for(int r=offset*3;r<offset*3+paging*3;r++)
			if(r<0x20 && rpg.roles_properties.magics[r][role])
				Font->blit_to(objs(rpg.roles_properties.magics[r][role]),buf,34+88*(r%3),begin_y+12+(r/3-offset)*18,(r==selected)?((rpg.objects[rpg.roles_properties.magics[r][role]].magic.param&mask  && rpg.roles_properties.MP[role]>=magics[rpg.objects[rpg.roles_properties.magics[selected][role]].magic.magic].power_used)?0xFA:0x1C):((rpg.objects[rpg.roles_properties.magics[r][role]].magic.param&mask && rpg.roles_properties.MP[role]>=magics[rpg.objects[rpg.roles_properties.magics[selected][role]].magic.magic].power_used)?0x4E:0x18),true);
		UIpics.getsprite(69)->blit_to(buf,34+88*(selected%3)+24,begin_y+12+(selected/3-offset)*18+11,true,3,2);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	}
	void post_action(menu *abs){
		static int target=0;
		if(rpg.objects[rpg.roles_properties.magics[selected][role]].magic.param & 0x10)//apply to all
			target=role;
		else
			if((target=select_role(target))<0)
				return;
		uint16_t &effect_script=rpg.objects[rpg.roles_properties.magics[selected][role]].magic.post;
		effect_script=process_script(effect_script,target);
		if(prelimit_OK)
			rpg.roles_properties.MP[role]-=magics[rpg.objects[rpg.roles_properties.magics[selected][role]].magic.magic].power_used,
			rpg.roles_properties.MP[role]=(rpg.roles_properties.MP[role]<0?0:rpg.roles_properties.MP[role]);
		show_status_bar(abs->bak);
	}
	int select_role(int role)
	{
		int ok=0;
		bitmap buf(screen);
		do{
			buf.blit_to(screen);
			UIpics.getsprite(67)->blit_to(screen,0x50+role*0x4b,0x9E);
			switch(sync_getkey())
			{
			case PAL_VK_MENU:
				return -1;
			case PAL_VK_UP:
			case PAL_VK_LEFT:
				role--;
				break;
			case PAL_VK_DOWN:
			case PAL_VK_RIGHT:
				role++;
				break;
			case PAL_VK_EXPLORE:
				ok=1;
				break;
			default:
				break;
			}
			role=(role<0?0:(role>rpg.team_roles?rpg.team_roles:role));
		}while(!ok && running);
		return role;
	}
};

struct equip_menu:public multi_menu
{
	equip_menu():multi_menu(2,0){}
	equip_menu(int mask,int skip):multi_menu(mask,skip){}
	void prev_action(menu *abs){
		max_ori=max=compact_items();
		if(skip==-1)
			begin_y=-8;
		else
			begin_y=33;
		blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
	}
	void got_action(menu *abs){
		if(rpg.objects[rpg.items[selected].item].item.param &mask)
		{
			bitmap buf(FBP.decode(1),320,200);
			bool ok=false;
			int role_sele=0,role_max=rpg.team_roles;
			int equip_selected=rpg.items[selected].item;
			int role=rpg.team[role_sele].role;

			dialog(0,3,0x62,rpg.team_roles+1,3,true,buf);
			bitmap bak(buf);
			do{
				int param=(rpg.objects[equip_selected].item.param>>6);

				sprite(BALL.decode(rpg.objects[equip_selected].item.image)).blit_to(buf,0x10,0xF);

				Font->blit_to(objs(equip_selected),buf,4,0x45,0xD);
				show_number(std::find(rpg.items,rpg.items+sizeof(rpg.items)/sizeof(RPG::ITEM),equip_selected)->amount,0x48,0x48,2,buf);

				for(int i=0;i<=rpg.team_roles;i++)
					Font->blit_to(objs(rpg.roles_properties.name[rpg.team[i].role]),buf,0x10,0x6E +i*0x12,(param>>Pal::rpg.team[i].role)&1?(i==role_sele?0xFA:0x4E):(i==role_sele?0x1C:0x18));

				for(int i=0xB;i<=0x10;i++)
					Font->blit_to(objs(rpg.role_prop_tables[i][role]),buf,0x84,0xB+0x16*(i-0xB),0x4E);

				for(int i=0x11;i<=0x15;i++)
					show_number(get_cons_attrib(role,i),0x116,0x10+0x16*(i-0x11),0,buf);

				buf.blit_to(screen);
				bak.blit_to(buf);

				switch(sync_getkey())
				{
					if(!equip_selected){
						ok=true;
						break;
					}
				case PAL_VK_MENU:
					ok=true;
					break;
				case PAL_VK_EXPLORE:
					if((param>>role)&1)
					{
						uint16_t contract_magic=rpg.roles_properties.contract_magic[role];

						uint16_t &equip_script=rpg.objects[equip_selected].item.equip;
						process_script(equip_script,role_sele);

						rpg.roles_properties.contract_magic[role]=contract_magic;

						use_item(equip_selected,1);
						add_goods_to_list(prev_equip,1);
						equip_selected=prev_equip;
					}
					break;
				case PAL_VK_UP:
					role_sele--;
					break;
				case PAL_VK_DOWN:
					role_sele++;
					break;
				default:
					break;
				}
				role_sele=(role_sele<0?0:(role_sele>role_max?role_max:role_sele));
				role=rpg.team[role_sele].role;
			}while(running && !ok);
			max=max_ori=compact_items();
		}
	}
};
struct use_menu:public equip_menu
{
	bool after;
	use_menu():equip_menu(1,0){}
	use_menu(int i,int skip):equip_menu(i,skip){}
	void prev_action(menu *abs){
		equip_menu::prev_action(abs);

		if(skip==0 && mask==1)//装备中的土灵珠等
			for(int i=0;i<=rpg.team_roles;i++)
				for(int j=0xB;j<=0x10;j++)
					if(rpg.objects[((roles*)&rpg.roles_properties)[j][rpg.team[i].role]].item.param & 1)
	//only available on gcc					rpg.items[max++]=(RPG::ITEM){((roles*)&rpg.roles_properties)[j][i],1,0};
					{
						RPG::ITEM it;it.item=((roles*)&rpg.roles_properties)[j][rpg.team[i].role];it.amount=0;it.using_amount=0;
						rpg.items[max++]=it;
					}
	}
	void got_action(menu *abs){
		if(rpg.objects[rpg.items[selected].item].item.param &mask)
			got=1;
	}
	void post_action(menu *abs){
		if(skip==-1)
			return;
		if(rpg.objects[rpg.items[selected].item].item.param & (2<<(4-1)))
		{
			scene->produce_one_screen();
			uint16_t &use_script=rpg.objects[rpg.items[selected].item].item.use;
			process_script(use_script,0);
			if(prelimit_OK && (rpg.objects[rpg.items[selected].item].item.param & (2<<(3-1))))
				use_item(rpg.items[selected].item,1);
		}else{
			bool ok=false;
			int role_sele=0,role_max=rpg.team_roles;
			int x=0x6E,y=2;
			blit(screen,abs->bak,0,0,0,0,SCREEN_W,SCREEN_H);
			bitmap buf(screen);
			do{
				if(rpg.items[selected].amount==0){
					compact_items();
					return;
				}
				dialog(0,x,y,8,10,false,buf);
				UIpics.getsprite(70)->blit_to(buf,x+8,y+0x4f,true,6,5);

				display_role_status(1,rpg.team[role_sele].role,x+0x54,y+0xC,buf);

				for(int i=0,x2=x+0xE,y2=y+0xC;i<=rpg.team_roles;i++,y2+=0x16)
					Font->blit_to(objs(rpg.roles_properties.name[rpg.team[i].role]),buf,x2,y2,(i==role_sele)?0xFA:0x4E);

				sprite(BALL.decode(rpg.objects[rpg.items[selected].item].item.image)).blit_to(buf,x+0x10,y+0x56);
				Font->blit_to(objs(rpg.items[selected].item),buf,x+4,y+0x8E,0xD,true);
				if(rpg.items[selected].amount > 1)
					show_number(rpg.items[selected].amount,x+0x3E,y+0x84,2,buf);
				buf.blit_to(screen);
				switch(sync_getkey())
				{
				case PAL_VK_MENU:
					return;
				case PAL_VK_EXPLORE:
				{
					uint16_t &use_script=rpg.objects[rpg.items[selected].item].item.use;
					process_script(use_script,role_sele);
					if(prelimit_OK && (rpg.objects[rpg.items[selected].item].item.param & (2<<(3-1))))
						use_item(rpg.items[selected].item,1);
				}
				break;
				case PAL_VK_UP:
					role_sele--;
					break;
				case PAL_VK_DOWN:
					role_sele++;
					break;
				default:
					break;
				}
				role_sele=(role_sele<0?0:(role_sele>role_max?role_max:role_sele));
			}while(running & !ok);
		}
	}
	~use_menu(){
		for(int i=max_ori;i<max;i++)
			rpg.items[i].item=0,
			rpg.items[i].amount=0,
			rpg.items[i].using_amount=0;
	}
};
void display_role_status(int flag,int role,int x,int y,BITMAP *buf)
{
	show_number(rpg.roles_properties.level[role],x+50,y+4,0,buf);
	for(int i=0x30,y1=y;i<=0x37;i++,y1+=18)
		if(flag)
			Font->blit_to(objs(i),buf,x,y1,0xBB);
		else
			UIpics.getsprite(47)->blit_to(buf,x+0x10,y1+6);
	for(int i=7,y2=y+20;i<=8;i++,y2+=18){
		show_number(rpg.role_prop_tables[2+i][role],x+50,y2,0,buf);
		UIpics.getsprite(39)->blit_to(buf,x+54,y2+3);
		show_number(rpg.role_prop_tables[i][role],x+70,y2+6,1,buf);
	}
	for(int i=0x11,y3=y+58;i<=0x15;i++,y3+=18)
		show_number(get_cons_attrib(role,i),x+50,y3,0,buf);
}
bool fade_filter(int srcVal, uint8* pOutVal, void* pUserData)
{
	if(srcVal==-1)
		return false;
	int color=*(int*)pUserData;
	*pOutVal=std::max(srcVal&0xf-color&0xF,0)|(color&0xF0);
	return true;
}
void show_status_bar(BITMAP *buf)
{
	int start_x=(flag_battling?0x5A:0x2A);
	for(int i=0;i<=rpg.team_roles;i++)
	{
		int role=rpg.team[i].role;
		int x=start_x+i*0x4E,y=0xA0;
		int color=0;
		for(int it=0,max=-1;it<16;it++)
			if(rpg.objects[rpg.poison_stack[it][i].poison].poison.toxicity>max)
				max=rpg.objects[rpg.poison_stack[it][i].poison].poison.toxicity,
				color=rpg.objects[rpg.poison_stack[it][i].poison].poison.color;
		color=(rpg.roles_properties.HP[role]?color:2);
		UIpics.getsprite(18)->blit_to(buf,x,0xA5);
		{
			if(color)
				UIpics.getsprite(48+role)->setfilter(fade_filter,color);
			else
				UIpics.getsprite(48+role)->setfilter();
			UIpics.getsprite(48+role)->blit_to(buf,x-3,y);
			UIpics.getsprite(39)->blit_to(buf,x+0x2F,y+0xB);
			UIpics.getsprite(39)->blit_to(buf,x+0x2F,y+0x1B);
			show_number(rpg.roles_properties.HP_max[role],x+0x40,y+0xD,0,buf);
			show_number(rpg.roles_properties.MP_max[role],x+0x40,y+0x1D,2,buf);
			show_number(rpg.roles_properties.HP[role],x+0x2A,y+0x9,0,buf);
			show_number(rpg.roles_properties.MP[role],x+0x2A,y+0x19,2,buf);
		}
	}
}

int select_rpg(int ori_select,BITMAP *bmp)
{
	int selected=ori_select;
	bitmap cache(0,SCREEN_W,SCREEN_H);
	selected=(selected>=1?selected:1);
	int ok=1;
	std::vector<std::string> menu_items;
	blit(bmp,cache,0,0,0,0,SCREEN_W,SCREEN_H);
	for(int r=0;r<5;r++)
		single_dialog(0xB4,4+0x26*r,6,cache);
	do{
		for(int r=0;r<5;r++){
			single_dialog(0xB4,4+0x26*r,6,cache,false);
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
			case PAL_VK_UP:
				selected--;
				break;
			case PAL_VK_DOWN:
				selected++;
				break;
			case PAL_VK_PGUP:
				selected-=5;
				break;
			case PAL_VK_PGDN:
				selected+=5;
				break;
			case PAL_VK_MENU:
				return 0;
			case PAL_VK_EXPLORE:
				ok=0;
				break;
            default:
                break;
		}
		selected=(selected<=1?1:selected);
	}while(running && ok);
	return selected;
}
int select_theurgy(int role,int mask,int selected,bool after)
{
	return menu(0xA,0x2D,5,0,17,9,false)(magic_menu(role,mask,after),selected);
}
int yes_or_no(int word,int selected)
{
	bitmap buf(screen);
	bool got=false;
	single_dialog(0x78,0x64,2,buf);
	single_dialog(0xC8,0x64,2,buf);
	do{
		single_dialog(0x78,0x64,2,buf,false);
		single_dialog(0xC8,0x64,2,buf,false);
		dialog_string(objs(word  ),0x78+0xF,0x64+0x9,(selected==0)?0xFA:0,(selected==0)?true:false,buf);
		dialog_string(objs(word+1),0xC8+0xF,0x64+0x9,(selected==1)?0xFA:0,(selected==1)?true:false,buf);
		buf.blit_to(screen);
		switch(sync_getkey())
		{
		case PAL_VK_MENU:
            return -1;
		case PAL_VK_EXPLORE:
			got=true;
			break;
		case PAL_VK_LEFT:
			selected--;
			break;
		case PAL_VK_RIGHT:
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
	do
		UIpics.getsprite(num%10+19)->blit_to(bmp,x+digits*6-6,y);
	while((num/=10) && --digits);
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
		UIpics.getsprite(i+number%10)->blit_to(bmp,x-=6,y);
	while(number/=10);
}
void show_money(int num,int x,int y,int text,bool shadow,BITMAP *bmp)
{
	single_dialog(x,y,5,bmp,shadow);
	Font->blit_to(objs(text),bmp,x+10,y+10,0,false);
	show_num_lim(num,x+48,y+15,6,bmp);
}

boost::shared_ptr<def_font> Font;

void role_status()
{
	int selected=0,cur_role=rpg.team[selected].role;
	bool ok=true;
	bitmap buf(NULL,320,200);
	do{
		fbp(0).blit_to(buf);

		Font->blit_to(objs(2   ),buf,6,8   ,0x4E,true);
		show_number(rpg.roles_exp[0][cur_role].exp,                    0x4E,8   ,0,buf);
		show_number(upgradexp[rpg.roles_exp[0][cur_role].level+1],0x4E,0x12,1,buf);

		Font->blit_to(objs(0x30),buf,6,0x22,0x4E,true);
		show_number(rpg.roles_properties.level[cur_role],              0x3A,0x26,0,buf);

		Font->blit_to(objs(0x31),buf,6,0x39,0x4E,true);
		show_number(rpg.roles_properties.HP[cur_role],                 0x3A,0x3A,0,buf);
		UIpics.getsprite(39)->blit_to(buf,0x3E,0x3D);
		show_number(rpg.roles_properties.HP_max[cur_role],             0x4E,0x40,1,buf);

		Font->blit_to(objs(0x32),buf,6,0x4F,0x4E,true);
		show_number(rpg.roles_properties.HP[cur_role],                 0x3A,0x50,0,buf);
		UIpics.getsprite(39)->blit_to(buf,0x3E,0x53);
		show_number(rpg.roles_properties.HP_max[cur_role],             0x4E,0x56,1,buf);

		for(int i=0x11,y=0x66;i<=0x15;i++,y+=0x14)
		{
			Font->blit_to(objs(0x22+i),buf,6,y-4,0x4E,true);
			show_number(get_cons_attrib(cur_role,i),0x3A,y,0,buf);
		}

		if(int equip=rpg.roles_properties.head_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0xBE,0);
			Font->blit_to(objs(equip),buf,0xBE + 4,0+0x2A,0xBD,true);
		}

		if(int equip=rpg.roles_properties.armo_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0xF8,0x28);
			Font->blit_to(objs(equip),buf,0xF8+4,0x28+0x2A,0xBD,true);
		}

		if(int equip=rpg.roles_properties.body_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0xFC,0x66);
			Font->blit_to(objs(equip),buf,0xFC+4,0x66+0x2A,0xBD,true);
		}

		if(int equip=rpg.roles_properties.arms_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0xCA,0x86);
			Font->blit_to(objs(equip),buf,0xCA+4,0x86+0x2A,0xBD,true);
		}

		if(int equip=rpg.roles_properties.feet_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0x8E,0x8E);
			Font->blit_to(objs(equip),buf,0x8E + 4,0x8E + 0x2A,0xBD,true);
		}

		if(int equip=rpg.roles_properties.trea_equip[cur_role]){
			sprite(BALL.decode(rpg.objects[equip].item.image)).blit_to(buf,0x52,0x7E);
			Font->blit_to(objs(equip),buf,0x52+4,0x7E + 0x2A,0xBD,true);
		}

		sprite(RGM.decode(rpg.roles_properties.icon[cur_role])).blit_middle(buf,0x9b,0x4e);
		Font->blit_to(objs(rpg.roles_properties.name[cur_role]),buf,0x6E,8,0x4E,true);

		for(int i=0,poisons=0;i<0x10;i++,poisons=(poisons>4?4:poisons))
			if(rpg.objects[rpg.poison_stack[i][selected].poison].poison.color)
				Font->blit_to(objs(rpg.poison_stack[i][selected].poison),buf,0xBC,0x3c+0x12*poisons++,rpg.objects[rpg.poison_stack[i][selected].poison].poison.color+10);

		buf.blit_to(screen);
		switch(sync_getkey()){
			case PAL_VK_MENU:
				ok=false;
			case PAL_VK_UP:
			case PAL_VK_LEFT:
				selected--;
				break;
			case PAL_VK_DOWN:
			case PAL_VK_RIGHT:
				selected++;
				break;
            default:
                break;
		}
		cur_role=rpg.team[selected=((selected<0)?0:((selected>rpg.team_roles)?rpg.team_roles:selected))].role;
	}while(ok && running);
}
int menu_item(int selected,int filter)
{
	return menu(2,-8,8,0,18,9,false)(use_menu(filter,-1),selected);
}
bool process_Menu()
{
	static int main_select=0,role_select=0,magic_select=0,itemuse_select=0,item_select=0,sys_select=0,rpg_select=0,music_selected=(global->get<int>("music","volume")>0),sfx_selected=(global->get<int>("music","volume_sfx")>0);
	show_money(rpg.coins,0,0,0x15,true);
	switch(main_select=menu(3,37,4,3,2)(single_menu(),main_select))
	{
	case 0:
		role_status();
		break;
	case 1:
		{
			show_status_bar();
			if(rpg.team_roles)
			{
				std::vector<std::string> names;
				for(int i=0;i<=rpg.team_roles;i++)
					names.push_back(objs(rpg.roles_properties.name[rpg.team[i].role]));
				role_select=menu(0x2c,0x40,names,3)(single_menu(),role_select);
			}
			else
				role_select=0;
			if(role_select>=0)
				magic_select=select_theurgy(rpg.team[role_select].role,1,magic_select);
		}
		break;
	case 2:
		switch(itemuse_select=menu(0x1e,0x3c,2,0x16,2)(single_menu(),itemuse_select))
		{
		case 0:
			item_select=menu(2,33,8,0,18,9,false)(equip_menu(),item_select);
			break;
		case 1:
			item_select=menu(2,33,8,0,18,9,false)(use_menu(),item_select);
			break;
		}
		break;
	case 3:
		switch(sys_select=menu(0x28,0x3c,5,0xB,4)(single_menu(),sys_select))
		{
		case 0:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				save(rpg_to_load);
			break;
		case 1:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				load(rpg_to_load);
			else
				return true;
			break;
		case 2:
			if((music_selected=yes_or_no(0x11,music_selected))<0)
                break;
			musicplayer->setvolume((music_selected==1)?255:0);
			break;
		case 3:
			if((sfx_selected=yes_or_no(0x11,sfx_selected))<0)
                break;
			global->set<int>("music","volume_sfx",(sfx_selected==1)?255:0);
			break;
		case 4:
            if(yes_or_no()==1)
                return false;
		}
	}
	clear_keybuf();
	rest(100);
	return running;
}

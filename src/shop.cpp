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
#include "item.h"
using namespace Pal;

struct Shop:public single_menu
{
	int shop;
	Shop(int s):shop(s)
	{}
	void prev_action(menu *abs){
		single_menu::prev_action(abs);
		UIpics.getsprite(70)->blit_to(abs->bak,0x28,8,true,6,5);
		show_money(0,0x14,0x8D,0x15,true,abs->bak);
		show_money(0,0x14,0x64,0x23,true,abs->bak);
	}
	int select(menu *abs,int _selected){
		abs->text_x+=(8-22+0x1C);
		abs->text_y=0x15;
		color_selecting=0x2F;
		
		return single_menu::select(abs,_selected);
	}
	void draw(menu *abs){
		UIpics.getsprite(70)->blit_to(abs->bak,0x28,8);
		sprite(BALL.decode(rpg.objects[shops[shop].item[selected]].item.image)).blit_to(abs->bak,0x30,0xF);
		show_money(rpg.coins,0x14,0x8D,0x15,false,abs->bak);
		show_money(count_item(shops[shop].item[selected]),0x14,0x64,0x23,false,abs->bak);

		for(unsigned int i=0;i<abs->menu_items.size();i++)
			show_number(rpg.objects[shops[shop].item[i]].item.value,0x102,abs->text_y+5+0x12*i,0,abs->bak);
		single_menu::draw(abs);
	}
	void got_action(menu *){
		if(yes_or_no() && rpg.coins>=rpg.objects[shops[shop].item[selected]].item.value)
		{
			add_goods_to_list(shops[shop].item[selected],1);
			rpg.coins-=rpg.objects[shops[shop].item[selected]].item.value;
		}
	}
};
struct HockShop:public multi_menu{
	HockShop():multi_menu(32,1,8){}
	void prev_action(menu *abs){
		max_ori=max=compact_items();
		begin_y=33;
		multi_menu::prev_action(abs);
	}
	void draw(menu *abs){
		show_money(rpg.coins,0,0,0x15,false,abs->bak);		
		if(rpg.objects[rpg.items[selected].item].item.param & mask)
			show_money(rpg.objects[rpg.items[selected].item].item.value/2,0xE0,0,0x19,false,abs->bak);
		else
			single_dialog(0xE0,0,5,abs->bak,false);
		multi_menu::draw(abs);
	}
	void got_action(menu *abs){
		if( (rpg.objects[rpg.items[selected].item].item.param & mask) && yes_or_no() )
		{
			use_item(rpg.items[selected].item,1);
			rpg.coins+=rpg.objects[rpg.items[selected].item].item.value/2;
			max_ori=max=compact_items();
		}
	}
};
void shop(int num)
{
	std::vector<std::string> items;
	for(int i=0;i<9;i++)
		if(shops[num].item[i])
			items.push_back(objs(shops[num].item[i]));
	menu(0x7a,8,items,9,9,9)(Shop(num),0);
}
void hockshop()
{
	menu(2,33,8,0,18,9,false)(HockShop(),0);
}

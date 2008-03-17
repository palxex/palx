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
#include <algorithm>
#include "internal.h"
#include "game.h"

int role_parts[5][17][74];
int prev_equip;

int compact_items()
{
	for(int i=0;i<0x100;i++)
		if(res::rpg.items[i].amount==0)
			std::copy(res::rpg.items+i+1,res::rpg.items+0x100,res::rpg.items+i);
	return std::find(res::rpg.items,res::rpg.items+0x100,0)-res::rpg.items;
}
void add_goods_to_list(int goods,int num)
{
    RPG::ITEM *ptr=std::find(res::rpg.items,res::rpg.items+0x100,goods);
    if (ptr==res::rpg.items+0x100)
        ptr=std::find(res::rpg.items,res::rpg.items+0x100,0),
        ptr->item=goods;
    ptr->amount+=num;
}


void learnmagic(bool flag_dialog,int magic,int role)
{
	if(std::find_if(res::rpg.roles_properties.magics,res::rpg.roles_properties.magics+0x20,rolemagic_select(role,magic))!=res::rpg.role_prop_tables+0x40)
		return;
	*std::find_if(res::rpg.roles_properties.magics,res::rpg.roles_properties.magics+0x20,rolemagic_select(role,0))[role]=magic;
	if(flag_dialog)
		;//
}

int get_cons_attrib(int role,int attrib)
{
	int result=res::rpg.role_prop_tables[attrib][role];
	for(int part=0xB;part<=0x10;part++)
		result+=role_parts[role][part][attrib];
	return result;
}

void use_item(int item,int amount)
{
	if(amount<=0)
		return;
	int i;
	for(i=0;i<0x100;i++)
		if(res::rpg.items[i].item==item)
			break;
	if((res::rpg.items[i].using_amount-=amount)<0)
		res::rpg.items[i].using_amount=0;
	if(res::rpg.items[i].amount>=amount)
		res::rpg.items[i].amount-=amount,
		amount=0;
	else
		amount-=res::rpg.items[i].amount,
		res::rpg.items[i].amount=0;
	for(int j=1;j<=amount;j++)
		for(int k=0,role=res::rpg.team[k].role;k<=res::rpg.team_roles;k++,role=res::rpg.team[k].role)
			for(int l=0xB;l<=0x10;l++)
				if(res::rpg.role_prop_tables[l][role]==item)
					res::rpg.role_prop_tables[l][role]=0;
}
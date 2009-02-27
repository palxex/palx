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
#include "game.h"

int role_parts[6][18][74];
int prev_equip;

int compact_items()
{
	for(int i=0;i<0x100;i++)
		if(Pal::rpg.items[i].amount==0)
			std::copy(Pal::rpg.items+i+1,Pal::rpg.items+0x100,Pal::rpg.items+i);
	return std::find(Pal::rpg.items,Pal::rpg.items+0x100,0)-Pal::rpg.items;
}
void add_goods_to_list(int goods,int num)
{
    RPG::ITEM *ptr=std::find(Pal::rpg.items,Pal::rpg.items+0x100,goods);
    if (ptr==Pal::rpg.items+0x100)
        ptr=std::find(Pal::rpg.items,Pal::rpg.items+0x100,0),
        ptr->item=goods;
    ptr->amount+=num;
}
int count_item_equiping(int item)
{
	int amount=0;
	for(int i=0,role=Pal::rpg.team[i].role;i<Pal::rpg.team_roles;role=Pal::rpg.team[++i].role)
		for(int part=0xB;part<=0x10;part++)
			if(Pal::rpg.role_prop_tables[part][role] == item)
				amount++;
	return amount;
}
int count_item(int item)
{
	RPG::ITEM *it=NULL;int amount=0;
	if((it=std::find(Pal::rpg.items,Pal::rpg.items+0x100,item))==Pal::rpg.items+0x100)
		amount=0;
	else
		amount=it->amount;
	return amount+count_item_equiping(item);
}
int compact_magic(int role)
{
	using namespace Pal;
		for(int i=0;i<31;i++)
			for(int j=0;j<32;j++)
				if(rpg.roles_properties.magics[i][role]<rpg.roles_properties.magics[j][role])
					std::swap(rpg.roles_properties.magics[i][role],rpg.roles_properties.magics[j][role]);
		for(int i=0;i<32;i++)
			if(rpg.roles_properties.magics[i][role])
				std::swap(std::find_if(rpg.roles_properties.magics,rpg.roles_properties.magics+sizeof(rpg.roles_properties.magics)/sizeof(roles),rolemagic_select(role,0))[0][role],rpg.roles_properties.magics[i][role]);
		return 0x20-std::count_if(rpg.role_prop_tables+0x20,rpg.role_prop_tables+0x40,rolemagic_select(role,0));
}

int get_magic_pos(int role_pos,int magic)
{
	int role=Pal::rpg.team[role_pos].role;
	int target=std::find_if(Pal::rpg.roles_properties.magics,Pal::rpg.roles_properties.magics+0x20,rolemagic_select(role,magic))-Pal::rpg.role_prop_tables;
	return target==0x40?0:target;
}

void learnmagic(bool flag_dialog,int magic,int role)
{
	if(get_magic_pos(role,magic)!=0)
		return;
	Pal::rpg.roles_properties.magics[get_magic_pos(role,0)-0x20][role]=magic;
	if(flag_dialog)
		;//
}

int get_cons_attrib(int role,int attrib)
{
	int result=Pal::rpg.role_prop_tables[attrib][role];
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
		if(Pal::rpg.items[i].item==item)
			break;
	if(i>=0x100)
		return;
	if((Pal::rpg.items[i].using_amount-=amount)<0)
		Pal::rpg.items[i].using_amount=0;
	if(Pal::rpg.items[i].amount>=amount)
		Pal::rpg.items[i].amount-=amount,
		amount=0;
	else
		amount-=Pal::rpg.items[i].amount,
		Pal::rpg.items[i].amount=0;
	for(int j=1;j<=amount;j++)
		for(int k=0,role=Pal::rpg.team[k].role;k<=Pal::rpg.team_roles;k++,role=Pal::rpg.team[k].role)
			for(int l=0xB;l<=0x10;l++)
				if(Pal::rpg.role_prop_tables[l][role]==item)
					Pal::rpg.role_prop_tables[l][role]=0;
}


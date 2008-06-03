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

#include "scene.h"
#include "game.h"
#include "begin.h"
#include "config.h"
#include "fade.h"

#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[])
{
	global_init _global(argc, argv);global=&_global;
	int save=_global();
			bitmap _fakescreen;fakescreen=_fakescreen;clear_bitmap(fakescreen);
			bitmap _backbuf;backbuf=_backbuf;clear_bitmap(backbuf);
			bitmap _bakscreen;bakscreen=_bakscreen;clear_bitmap(bakscreen);

	using namespace Pal;
	init_resource();
	//load save
	if(save!=0 || (save=begin_scene()()))
		rpg_to_load=save;
	Scene normal;	scene=&normal;
	flag_to_load=0x10;
	bool new_rpg=!(save&&load());
	if(new_rpg){
		flag_to_load|=0xD;
		map_toload=1;
		for(int i=0;i<5;i++)
			for(int j=0;j<8;j++)
				if(j)
					rpg.roles_exp[j][i].level=rpg.roles_properties.level[i]+round(rnd1(2))+2,
					rpg.roles_exp[j][i].exp=round(rnd1(20));
				else
					rpg.roles_exp[j][i].level=rpg.roles_properties.level[i],
					rpg.roles_exp[j][i].exp=0;
		pal_fade_out(1);
	}
	run();
	destroy_resource();
	return 0;
}
END_OF_MAIN();

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
	global_settings::set_param(argc, argv);
	global=global_settings::instance();
	global->init();
	if(!running)
		return -1;

	bitmap _fakescreen;fakescreen=_fakescreen;clear_bitmap(fakescreen);
	bitmap _backbuf;backbuf=_backbuf;clear_bitmap(backbuf);
	bitmap _bakscreen;bakscreen=_bakscreen;clear_bitmap(bakscreen);
	int save=global->get_save();

	using namespace Pal;
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
					rpg.exps[i].roles_exp[j].level=rpg.roles_properties.level[i]+roundto(rnd1(2))+2,
					rpg.exps[i].roles_exp[j].exp=roundto(rnd1(20));
				else
					rpg.exps[i].roles_exp[j].level=rpg.roles_properties.level[i],
					rpg.exps[i].roles_exp[j].exp=0;
		pal_fade_out(1);
	}
	run();
	global_settings::destroy();
	return 0;
}
END_OF_MAIN();

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

#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[])
{
	try{
		global_init _global("palx.conf");global=&_global;
		if(argc==1)
		{
			int save=_global();
			bitmap _fakescreen(NULL,SCREEN_W,SCREEN_H);fakescreen=_fakescreen;
			bitmap _backbuf(NULL,SCREEN_W,SCREEN_H);backbuf=_backbuf;
			bitmap _bakscreen(NULL,SCREEN_W,SCREEN_H);bakscreen=_bakscreen;

			playrix player;rix=&player;
			using namespace res;
			init_resource();
			//load save
			if(save!=0 || (save=begin_scene()()))
				rpg_to_load=save;
			Scene normal;	scene=&normal;
			bool new_rpg=!save;
			new_rpg&=load();
			if(new_rpg){
				flag_to_load=0x12;
				map_toload=1;
				for(int i=0;i<5;i++)
					for(int j=0;j<8;j++)
						if(j)
							rpg.roles_exp[j][i].level=rpg.roles_properties.level[i]+rnd1(2)+2,
							rpg.roles_exp[j][i].exp=rnd1(20);
						else
							rpg.roles_exp[j][i].level=rpg.roles_properties.level[i],
							rpg.roles_exp[j][i].exp=0;
				pal_fade_out(1);
			}
			run();
			destroy_resource();
		}else
            global->set<string>("config","path",".");
	}catch(...){
#if defined(_CRTDBG_MAP_ALLOC_NEW)
		_CrtDumpMemoryLeaks();
#endif
		allegro_exit();
	}
	return 0;
}
END_OF_MAIN();

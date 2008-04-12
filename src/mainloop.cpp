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
#include "game.h"
#include "scene.h"
#include "timing.h"

#include <boost/shared_ptr.hpp>
extern int CARD,mutex;
bool running=true;
namespace{
	void mainloop_proc()
	{
		PAL_VKEY keygot=PAL_VK_NONE;
		do{
			if(flag_to_load)
				Load_Data();

			//Parse Key
			keygot=get_key_lowlevel();
			clear_keybuf();memset((void*)key,0,KEY_MAX);

			flag_to_load=0;
			GameLoop_OneCycle(true);
		}while(flag_to_load);

		if(!running)
			return;

		//clear scene back
		scene->clear_scanlines();

		sprite_queue sprites;
		sprites.clear_active();
		sprites.calc_team_walking();
		sprites.our_team_setdraw();
		sprites.visible_NPC_movment_setdraw();

		scene->move_usable_screen();

		sprites.Redraw_Tiles_or_Fade_to_pic();

		scene->scanline_draw_normal_scene(sprites,1);

			//Parse Key
			keygot=get_key_lowlevel();
			clear_keybuf();memset((void*)key,0,KEY_MAX);

		if(keygot==PAL_VK_EXPLORE)
			process_Explore();
		if(keygot==PAL_VK_MENU)
			running=process_Menu();

		flag_parallel_mutex=!flag_parallel_mutex;
	}
	END_OF_FUNCTION(mainloop_proc);
}

namespace Pal{
    int run(){
        //游戏主循环10fps,画面100fps,音乐70fps。
        while(running)
            mainloop_proc();
		keyboard_lowlevel_callback=NULL;
        return 0;
    }
}

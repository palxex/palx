/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL   *
 *   Copyleft (C) 2006 by Pal Lockheart   *
 *   palxex@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "game.h"
#include "internal.h"
#include "scene.h"
#include "timing.h"

#include <boost/shared_ptr.hpp>
extern int CARD,mutex;
bool running=true;
namespace{
	void mainloop_proc()
	{
		if(flag_to_load)
			Load_Data();

		//Parse Key
		VKEY keygot=get_key_lowlevel();
		
		GameLoop_OneCycle(true);
		if(flag_to_load)
			Load_Data();

		//clear scene back
		scene->clear_scanlines();
		scene->clear_active();
		scene->calc_team_walking(keygot);
		scene->our_team_setdraw();
		scene->visible_NPC_movment_setdraw();
		scene->move_usable_screen();
		scene->Redraw_Tiles_or_Fade_to_pic();
		scene->scanline_draw_normal_scene(1);
		if(keygot==VK_EXPLORE)
			process_Explore();
		if(keygot==VK_MENU)
			running=process_Menu();

		flag_parallel_mutex=!flag_parallel_mutex;
		
	}
	END_OF_FUNCTION(mainloop_proc);
}

int Game::run(){
	//游戏主循环10fps,画面100fps,音乐70fps。
	while(running){
		rest(1);
		if(time_interrupt_occurs>=10)
			mainloop_proc(),time_interrupt_occurs=0;
	}
	return 0;
}

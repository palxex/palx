#include "Game.h"
#include "internal.h"
#include "scene.h"
#include "timing.h"

#include <boost/shared_ptr.hpp>

VKEY keygot;
namespace{
	bool running=true;
	void mainloop_proc()
	{
		if(flag_to_load)
			Load_Data();

		//Parse Key
		get_key();
		
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
		scene->draw_normal_scene(1);
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
		rest(5);
		if(time_interrupt_occurs>=10)
			mainloop_proc(),time_interrupt_occurs=0;
	}
	return 0;
}
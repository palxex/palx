#include "Game.h"
#include "internal.h"
#include "scene.h"

#include <boost/shared_ptr.hpp>

namespace{
	bool running=true;
	void mainloop_proc()
	{
		if(flag_battling)
			return;

		if(flag_to_load)
			Load_Data(flag_to_load);

		//Parse Key
		int keygot=get_key();
		
		GameLoop_OneCycle(true);

		//clear scene back
		scene->clear_scanlines();
		scene->clear_active();
		scene->calc_team_walking(keygot);
		scene->our_team_setdraw();
		scene->visible_NPC_movment_setdraw();
		scene->Redraw_Tiles_or_Fade_to_pic();
		scene->move_usable_screen();
		scene->process_scrn_drawing(1);
		if(keygot==VK_MENU)
			running=process_Menu();
		if(keygot==VK_EXPLORE)
			process_Explore();

		flag_parallel_mutex=!flag_parallel_mutex;

		if(flag_to_load)
			Load_Data(flag_to_load);
	}
	END_OF_FUNCTION(mainloop_proc);
	void timer_proc()
	{
		static int pal_lock=0;
		mutux_setpalette=false;
		if(pal_lock++==10){
			PALETTE pal;
			get_palette(pal);
			RGB temp=pal[0xF6];
			memcpy(pal+0xF6,pal+0xF7,6);
			pal[0xF8]=temp;
			temp=pal[0xF9];
			memcpy(pal+0xF9,pal+0xF8,0xF);
			pal[0xFE]=temp;
			set_palette(pal);
			pal_lock=0;
		};
		time_interrupt_occers++;
		mutux_setpalette=true;
	}
	END_OF_FUNCTION(timer_proc);
}

int Game::run(){
	//游戏主循环10fps,画面100fps,音乐70fps。
	//因为allegro int无法调试，调试期改用循环。
	/*
	install_int(mainloop_proc,100);
	LOCK_VARIABLE(time_interrupt_occers);
	install_int(timer_proc,10);

	while(running) rest(10);

	remove_int(mainloop_proc);
	remove_int(timer_proc);/*///
	while(running){
		static int i;
		timer_proc();
		rest(10);
		if(i++==5)
			i=0,mainloop_proc();
	}//*/

	return 1;
}
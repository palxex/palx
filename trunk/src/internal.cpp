#include "internal.h"
#include "allegdef.h"
#include "scene.h"
#include "game.h"
bool flag_battling=false;

int flag_to_load=0;
int rpg_to_load=0;

bool flag_parallel_mutex=false;
int redraw_flag=0;

bool mutux_setpalette=false;
volatile int time_interrupt_occers; 

int RPG_screen_wave_grade=0;
int wave_progression=0;

Scene *scene;
BattleScene *battle_scene;
playrix *rix;
Game *game;

void Load_Data(int &flag)
{
	if(flag&0x10){
		//load sfx; this task has been not needed since we didn't be limited by 640K.
	}
	if(flag&0x20){
		//load rpg
		game->load(rpg_to_load);
	}
	else if(scene->current!=scene->toload){
		game->rpg.wave_grade=0;
		wave_progression=0;
		//save previous scene's event objects,not needed in this policy
	}
	redraw_flag=0;
	x_scrn_offset=0xA0;
	y_scrn_offset=0x70;
	scene->current=scene->toload;
	if(flag&4){
		//load evtobjs
		scene->sprites_begin=game->evtobjs.begin()+game->scenes[scene->current].prev_evtobjs+1;
		scene->sprites_end  =game->evtobjs.begin()+game->scenes[scene->current+1].prev_evtobjs+1;		
	}
	//load map & npc
	scene->scenemap.change(game->scenes[scene->current].id);
	scene->get_sprites();
	scene->produce_one_screen();
	if(flag&1){
		load_team_mgo();
	}
	if(flag&8){
		//enter a new scene;
		flag|=2;
		uint16_t &enterscript=game->scenes[scene->current].enter_script;
		enterscript=process_script(enterscript,0);
		if(scene->current!=scene->toload)
			Load_Data(flag);
	}
	if(flag&2){
		//play music
		rix->play(game->rpg.music);
	}
	flag=0;
}
inline int16_t absdec(int16_t &s)
{
	int16_t s0=s;
	if(s>0)	s--;
	else if(s<0) s++;
	return s0;
}
void GameLoop_OneCycle(bool trigger)
{
	typedef std::vector<EVENT_OBJECT>::iterator evt_obj;
	if(trigger)
		for(evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&scene->current==scene->toload;++iter)
			if(absdec(iter->vanish_time)==0)
				if(iter->status>0 && iter->trigger_method>=4){
					if(abs(scene->team_pos.x-iter->pos_x)+abs(scene->team_pos.y-iter->pos_y)*2<=(iter->trigger_method-4)*32)// in the distance that can trigger
					{
						if(iter->frames)
						{
							stop_and_update_frame();
							iter->curr_frame=0;
							//calc_facing_to();
							redraw_everything();
						}
						//x_off=0,y_off=0;
						uint16_t &triggerscript=iter->trigger_script;
						triggerscript=process_script(triggerscript,(int16_t)(iter-game->evtobjs.begin()));
					}
				}else if(iter->status<0){//&& in the screen
					iter->status=abs(iter->status);
					//step==0
				}
	for(evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&scene->current==scene->toload;++iter)
	{
		if(iter->status!=0)
			if(uint16_t &autoscript=iter->auto_script)
				autoscript=process_autoscript(autoscript,(int16_t)(iter-game->evtobjs.begin()));
		if(iter->status==2 && iter->image>0 && trigger)//&& beside role && face to it
		{
			//check barrier;this means, role status 2 means it takes place
		}
	}
}
bool process_Menu()
{
	return false;
}
void process_Explore()
{
}

void redraw_everything(int time_gap)
{
	flag_parallel_mutex=!flag_parallel_mutex;
	scene->clear_active();
	if(flag_battling)
		;
	else{
		scene->visible_NPC_movment_setdraw();
		scene->our_team_setdraw();
		scene->Redraw_Tiles_or_Fade_to_pic();
		scene->draw_normal_scene(time_gap);
	}
}
void clear_effective(int16_t p1,int16_t p2)
{
	redraw_everything();
}
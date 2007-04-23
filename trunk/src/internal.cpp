#include "internal.h"

bool flag_battling=false;

int flag_to_load=0;
int step_off_x=16,step_off_y=8;
int coordinate_x_max=1696,coordinate_y_max=1840;
bool flag_parallel_mutex=false;

Scene *scene,*battle_scene;
playrix *rix;
Game *game;
void redraw_everything(){}
void Load_Data(int flag)
{
	if(flag&0x10){
		//load sfx
	}
	if(flag&0x20){
		//load rpg
		game->reload();
	}
	if(scene->current!=scene->toload){
		if(scene->current!=0){
			//save previous scene's event objects
		}
		scene->current=scene->toload;
	}
	if(flag&4){
		//load evtobjs
		scene->sprites_begin=game->evtobjs.begin()+game->scenes[scene->current].prev_evtobjs+1;
		scene->sprites_end  =game->evtobjs.begin()+game->scenes[scene->current+1].prev_evtobjs+1;
		
		//load map & npc mgo
	}
	if(flag&1){
		//load role mgo
	}
	if(flag&8){
		//enter a new scene;
		flag&=2;
		uint16_t &enterscript=game->scenes[scene->current].enter_script;
		process_script(enterscript,0);
		if(scene->current!=scene->toload)
			Load_Data(flag);
	}
	if(flag&2){
		//play music
	}
	flag=0;
}
inline int16_t absdec(int16_t &s)
{
	int16_t s0=s;
	if(s>0)
		s--;
	else if(s<0)
		s++;
	return s0;
}
void GameLoop_OneCycle(bool trigger)
{
	typedef std::vector<EVENT_OBJECT>::iterator evt_obj;
	if(trigger)
		for(evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&scene->current==scene->toload;++iter)
			if(absdec(iter->vanish_time)==0)
				if(iter->status>0 && iter->trigger_method>=4){
					if(abs(scene->user_pos.x-iter->pos_x)+abs(abs(scene->user_pos.y-iter->pos_y)*2<=(iter->trigger_method-4)*32))// in the distance that can trigger
					{
						if(iter->frames)
							redraw_everything();
						uint16_t &triggerscript=iter->trigger_script;
						process_script(triggerscript,(int16_t)(iter-game->evtobjs.begin()));
					}
				}else if(iter->status<0){//&& in the screen
					iter->status=abs(iter->status);
					//step==0
				}
	for(evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&scene->current==scene->toload;++iter)
	{
		if(iter->status!=0)
			if(uint16_t &autoscript=iter->auto_script)
				process_autoscript(autoscript,(int16_t)(iter-game->evtobjs.begin()));
		if(iter->status==2 && iter->image>0 && trigger)//&& beside role && face to it
		{
			//check barrier;this means, role status 2 means it takes place
		}
	}
}
void process_scrn_drawing(int)
{
	scene->now->blit_to(screen,scene->camera_pos.x,scene->camera_pos.y,0,0);
}
bool process_Menu()
{
	return false;
}
void process_Explore()
{}
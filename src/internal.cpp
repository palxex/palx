#include "internal.h"
#include "allegdef.h"
#include "scene.h"
#include "game.h"
#include "UI.h"
bool flag_battling=false;

int flag_to_load=0;
int rpg_to_load=0;
int map_toload=0;

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

extern int scale;
bool key_enable=true;

void Load_Data()
{
	flag_battling=false;
	if(flag_to_load&0x10){
		//load sfx; this task has been not needed since we didn't be limited by 640K.
	}
	if(flag_to_load&0x20){
		//load rpg
		game->load(rpg_to_load);
	}
	else if(game->rpg.scene_id!=map_toload){
		game->rpg.wave_grade=0;
		wave_progression=0;
		//save previous scene's event objects,not needed in this policy
	}
	redraw_flag=0;
	x_scrn_offset=0xA0*scale;
	y_scrn_offset=0x70*scale;
	scene->team_pos.toXY().x=game->rpg.viewport_x+x_scrn_offset;
	scene->team_pos.toXY().y=game->rpg.viewport_y+y_scrn_offset;
	game->rpg.scene_id=map_toload;
	if(flag_to_load&4){
		//load evtobjs
		scene->sprites_begin=game->evtobjs.begin()+game->scenes[game->rpg.scene_id].prev_evtobjs+1;
		scene->sprites_end  =game->evtobjs.begin()+game->scenes[game->rpg.scene_id+1].prev_evtobjs+1;		
	}
	//load map & npc
	scene->scenemap.change(game->scenes[game->rpg.scene_id].id);
	scene->get_sprites();
	scene->produce_one_screen();
	if(flag_to_load&1){
		load_team_mgo();
	}
	if(flag_to_load&8){
		//enter a new scene;
		flag_to_load&=2;key_enable=false;
		uint16_t &enterscript=game->scenes[game->rpg.scene_id].enter_script;
		enterscript=process_script(enterscript,0);
		if(game->rpg.scene_id!=map_toload)
			Load_Data();
	}
	if(flag_to_load&2){
		//play music
		rix->play(game->rpg.music);
	}
	flag_to_load=0;
}
bool process_Menu()
{
	//show_money();
	//dialog(,0,0,1,5);
	switch(menu(3,37,4,3,2).select())
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		switch(menu(0x28,0x3c,5,0xB,4).select())
		{
		case 0:
			game->save(rpg_to_load);
			break;
		case 1:
			game->load(rpg_to_load);
			break;
		case 4:
			return false;
		}
	}
	return true;
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
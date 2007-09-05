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
int flag_pic_level=0;

bool mutux_setpalette=false;

int RPG_screen_wave_grade=0;
int wave_progression=0;

Scene *scene;
BattleScene *battle_scene;
playrix *rix;
Game *game;

extern int scale;
extern bool running;
bool key_enable=true;

#include <exception>
void perframe_proc()
{
	extern bool running;
	if(!running)
		if(starting)
			throw new std::exception();
		else
			exit(-1);
	switch_proc();
	ShakeScreen();
}
void switch_proc()
{
	mutex_int=1;
	if(key[KEY_F11] || ((key[KEY_ALT]||key[KEY_ALTGR]) && key[KEY_ENTER]))
	{
		while(mutex_paletting || mutex_blitting)
			rest(1);
		int &mode=CARD;
		static PALETTE pal;static bitmap bak(NULL,SCREEN_W,SCREEN_H);
		if(mode==GFX_AUTODETECT_WINDOWED || mode ==GFX_AUTODETECT)
			mode=GFX_AUTODETECT_FULLSCREEN;
		else
			mode=GFX_AUTODETECT_WINDOWED;
		get_palette(pal);blit(screen,bak,0,0,0,0,SCREEN_W,SCREEN_H);
		vsync();
		set_gfx_mode(mode,SCREEN_W,SCREEN_H,0,0);
		set_palette(pal);blit(bak,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	}
	mutex_int=0;
}
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
	//罢,罢,想加这个功能惹出一堆事
	//scene->team_pos.toXY().x=game->rpg.viewport_x+x_scrn_offset;
	//scene->team_pos.toXY().y=game->rpg.viewport_y+y_scrn_offset;
	game->rpg.scene_id=map_toload;
	if(flag_to_load&4){
		//load evtobjs
		scene->sprites_begin=game->evtobjs.begin()+game->scenes[game->rpg.scene_id].prev_evtobjs+1;
		scene->sprites_end  =game->evtobjs.begin()+game->scenes[game->rpg.scene_id+1].prev_evtobjs+1;
		for(std::vector<EVENT_OBJECT>::iterator i=scene->sprites_begin;i!=scene->sprites_end;i++)
			if(i->image)
				i->frames_auto=sprite_prim().determain_smkfs(MGO.decode(i->image));
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
	static int main_select=0,role_select=0,itemuse_select=0,item_select=0,sys_select=0,rpg_select=0;
	//show_money();
	single_dialog(0,0,5);
	ttfont(cut_msg_impl("word.dat")(0x15*10,0x16*10)).blit_to(screen,10,10,0,false);
	switch(main_select=menu(3,37,4,3,2).select(main_select))
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		switch(itemuse_select=menu(0x1e,0x3c,2,0x16,2).select(itemuse_select))
		{
		case 0:
			item_select=select_item(2,0,item_select);
			{
				uint16_t &equip_script=game->rpg.objects[game->rpg.items[item_select].item].script[1];
				process_script(equip_script,0);
			}
			break;
		case 1:
			item_select=select_item(1,0,item_select);
			{
				uint16_t &use_script=game->rpg.objects[game->rpg.items[item_select].item].script[0];
				process_script(use_script,0);
			}
			break;
		}
		break;
	case 3:
		switch(sys_select=menu(0x28,0x3c,5,0xB,4).select(sys_select))
		{
		case 0:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				game->save(rpg_to_load);
			break;
		case 1:
			if(rpg_select=select_rpg(rpg_to_load))
				rpg_to_load=rpg_select,
				game->load(rpg_to_load);
			else
				return true;
			break;
		case 4:
			return false;
		}
	}
	clear_keybuf();
	rest(10);
	return true;
}

void redraw_everything(int time_gap)
{
	flag_parallel_mutex=!flag_parallel_mutex;
	scene->clear_active();
	if(flag_battling)
		;
	else{
		rest(100);
		scene->visible_NPC_movment_setdraw();
		scene->our_team_setdraw();
		scene->Redraw_Tiles_or_Fade_to_pic();
		scene->scanline_draw_normal_scene(time_gap);
	}
}

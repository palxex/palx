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
#include "allegdef.h"
#include "scene.h"
#include "game.h"
#include "config.h"
#include "item.h"
#include "battle.h"
#include "fade.h"

bool flag_battling=false;

int flag_to_load=0;
int rpg_to_load=0;
int map_toload=0;

bool flag_parallel_mutex=false;
int redraw_flag=0;
int flag_pic_level=0;

int RPG_screen_wave_grade=0;
int wave_progression=0;

Scene *scene;
playrix *rix;
Game *game;

bool key_enable=true;

void switch_proc()
{
	if(key[KEY_F11] || ((key[KEY_ALT]||key[KEY_ALTGR]) && key[KEY_ENTER]))
	{
		mutex_switching=1;
		while(mutex_paletting || mutex_blitting)
			rest(1);
		global->display_setup(false);
		mutex_switching=0;
	}
}
void Load_Data()
{
	using namespace Pal;
	flag_battling=false;
	x_off=0,y_off=0;
	if(flag_to_load&0x10){
		//load sfx; this task has been not needed since we didn't be limited by 640K.
	}
	if(flag_to_load&0x20){
		//load rpg
		load(rpg_to_load);
	}
	else if(rpg.scene_id!=map_toload){
		rpg.wave_grade=0;
		wave_progression=0;
		//save previous scene's event objects,not needed in this policy
	}
	redraw_flag=0;
	x_scrn_offset=0xA0*scale;
	y_scrn_offset=0x70*scale;
	//罢,罢,想加这个功能惹出一堆事
	//scene->team_pos.toXY().x=rpg.viewport_x+x_scrn_offset;
	//scene->team_pos.toXY().y=rpg.viewport_y+y_scrn_offset;
	rpg.scene_id=map_toload;
	if(flag_to_load&4){
		//load evtobjs
		scene->sprites_begin=evtobjs.begin()+scenes[rpg.scene_id].prev_evtobjs+1;
		scene->sprites_end  =evtobjs.begin()+scenes[rpg.scene_id+1].prev_evtobjs+1;
		for(std::vector<EVENT_OBJECT>::iterator i=scene->sprites_begin;i!=scene->sprites_end;i++)
			if(i->image)
				i->frames_auto=MGO.slices(i->image);
	}
		//load map & npc
		scene->scenemap.change(scenes[rpg.scene_id].id);
		scene->get_sprites();
		scene->produce_one_screen();
	if(flag_to_load&1){
		load_team_mgo();
	}
	if(flag_to_load&8){
		//enter a new scene;
		flag_to_load&=2;key_enable=false;
		uint16_t &enterscript=scenes[rpg.scene_id].enter_script;
		enterscript=process_script(enterscript,0);
		if(rpg.scene_id!=map_toload)
			Load_Data();
	}
	if(flag_to_load&2){
		//play music
		musicplayer->play(rpg.music);
	}
	flag_to_load=0;
	setup_our_team_data_things();
}

void redraw_everything(int time_gap,BITMAP *dst)
{
	flag_parallel_mutex=!flag_parallel_mutex;
	if(flag_battling){
		battle::get()->sprites.clear_active();
		battle::get()->draw_battle_scene(0,1);
	}
	else{
		sprite_queue sprites;
		sprites.visible_NPC_movment_setdraw();
		sprites.our_team_setdraw();
		sprites.Redraw_Tiles_or_Fade_to_pic();
		scene->scanline_draw_normal_scene(sprites,time_gap,dst);
	}
}

void setup_our_team_data_things()
{
	//清除物品使用记录
	for(int i=0;i<0x100;i++)
		Pal::rpg.items[i].using_amount=0;

	//清除装备记录
	memset(role_parts,0,sizeof(role_parts));

	for(int i=0,role=Pal::rpg.team[i].role;i<=Pal::rpg.team_roles;i++,role=Pal::rpg.team[i].role)
	{
		//战象初始化,合体初始化
		battle_role_data[i].battle_avatar=Pal::rpg.roles_properties.battle_avator[role];
		battle_role_data[i].contract_magic=Pal::rpg.roles_properties.contract_magic[role];
		//全攻消除
		Pal::rpg.roles_properties.attack_all[role]=0;
		//双攻消除
		role_status_pack[i].pack.twice_attack=0;
		//重新装备
		for(int j=0xB;j<=0x10;j++)
			if(Pal::rpg.role_prop_tables[j][role])
			{
				uint16_t &equip_script=Pal::rpg.objects[Pal::rpg.role_prop_tables[j][role]].item.equip;
				equip_script=process_script(equip_script,i);
			}
	}
}


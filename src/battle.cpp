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
#include "game.h"
#include "timing.h"
#include "battle.h"
#include "scene.h"
#include "UI.h"

using namespace Pal;

union _role_status role_status_pack[TEAMROLES];
_battle_role_data battle_role_data[TEAMROLES];
_battle_enemy_data battle_enemy_data[TEAMROLES];
bool flag_autobattle=false;

int battle::max_blow_away=0;
battle *battle::thebattle=NULL;

struct{int x,y;} role_poses[4][4]={{{240,170}},{{200,176},{256,152}},{{180,180},{234,170},{270,146}},{{160,184},{204,175},{246,160},{278,144}}},
	instrum_pos[4]={{0x1C,0x8C},{0,0x9B},{0x37,0x9B},{0x1B,0xAA}};

int enemy_sequence[5]={2,1,0,4,3};

void battle::load_enemy(int enemy_pos,int enemy_id)
{
	battle_enemy_data[enemy_pos].id=enemy_id;
	if(enemy_id){
		battle_enemy_data[enemy_pos].battle_avatar=rpg.objects[enemy_id].enemy.enemy;
		battle_enemy_data[enemy_pos].HP=monsters[rpg.objects[enemy_id].enemy.enemy].hp;
		battle_enemy_data[enemy_pos].script.script.before=rpg.objects[enemy_id].enemy.before;
		battle_enemy_data[enemy_pos].script.script.when=rpg.objects[enemy_id].enemy.occur;
		battle_enemy_data[enemy_pos].script.script.after=rpg.objects[enemy_id].enemy.after;
		enemy_exps+=monsters[rpg.objects[enemy_id].enemy.enemy].exp;
		enemy_money+=monsters[rpg.objects[enemy_id].enemy.enemy].coins;
	}
}
void battle::setup_role_enemy_image()
{
	for(int i=0;i<=rpg.team_roles;i++){
		team_images[i]=sprite_prim(F,Pal::rpg.roles_properties.battle_avator[Pal::rpg.team[i].role]);
		battle_role_data[i].pos_x=role_poses[rpg.team_roles][i].x;
		battle_role_data[i].pos_y=role_poses[rpg.team_roles][i].y;
		battle_role_data[i].pos_x_bak=battle_role_data[i].pos_x;
		battle_role_data[i].pos_y_bak=battle_role_data[i].pos_y;
	}
	for(int i=0;i<enemy_poses_count;i++){
		if(battle_enemy_data[i].HP>0 && battle_enemy_data[i].id>0){
			enemy_images[i]=sprite_prim(Pal::ABC,rpg.objects[enemyteams[enemy_team].enemy[i]].general.inbeing);
			battle_enemy_data[i].length=enemy_images[i].getsprite(0)->height;
		}
		battle_enemy_data[i].pos_x=enemyposes.pos[i][enemy_poses_count-1].x;
		battle_enemy_data[i].pos_y=enemyposes.pos[i][enemy_poses_count-1].y+monsters[rpg.objects[battle_enemy_data[i].id].enemy.enemy].pos_y_offset;
		battle_enemy_data[i].pos_x_bak=battle_enemy_data[i].pos_x;
		battle_enemy_data[i].pos_y_bak=battle_enemy_data[i].pos_y;
		battle_enemy_data[i].frame=battle_enemy_data[i].frame_bak=0;
	}
}
void battle::setup_role_status()
{
	for(int i=0;i<=rpg.team_roles;i++){
		int role=rpg.team[i].role;
		battle_role_data[i].frame=battle_role_data[i].prev_frame;
		role_attack_table[i].alive=0;
		if(rpg.roles_properties.HP[role]<100 && rpg.roles_properties.HP[role]<=rpg.roles_properties.HP_max[role]/5)
			battle_role_data[i].frame=1;
		if(role_status_pack[i].pack.sleep)
			battle_role_data[i].frame=1;
		if(role_status_pack[i].pack.dumn)
			battle_role_data[i].frame=0;
		if(rpg.roles_properties.HP[role]<=0)
			battle_role_data[i].frame=2;
		if(battle_role_data[i].frame_bak!=2)
			role_attack_table[i].alive=-1;
		battle_role_data[i].frame_bak=battle_role_data[i].frame;
		if(battle_role_data[i].frame<1){
			battle_role_data[i].pos_x=battle_role_data[i].pos_x_bak;
			battle_role_data[i].pos_y=battle_role_data[i].pos_y_bak;
		}
	}
}
int battle::get_member_alive()
{
	return 1;
}
int battle::get_enemy_alive()
{
	return 1;
}

void battle::draw(int delay,int time)
{
	stage_blow_away+=rnd1(max_blow_away);

	for(int i=1;i<=time;i++)
	{
		for(int i=Pal::rpg.team_roles;i>=0;i--)
			team_images[i].getsprite(0)->blit_middle(battlescene,battle_role_data[i].pos_x,battle_role_data[i].pos_y-team_images[i].getsprite(0)->height/2);
		for(int i=4;i>=0;i--)
			if(Pal::enemyteams[enemy_team].enemy[i]>0)
				enemy_images[i].getsprite(0)->blit_to(battlescene,battle_enemy_data[i].pos_x-enemy_images[i].getsprite(0)->width/2,battle_enemy_data[i].pos_x-enemy_images[i].getsprite(0)->height);

		battlescene.blit_to(screen,0,0,0,0);
	}
}
bool brighter_filter(int srcVal, uint8* pOutVal, void* pUserData)
{
	if(srcVal==-1)
		return false;
	int color=*(int*)pUserData;
	*pOutVal=std::min((srcVal&0xf)+(color&0xF),0xF)|(color&0xF0);
	return true;
}
bool sadden_filter(int srcVal, uint8* pOutVal, void* pUserData)
{
	if(srcVal==-1)
		return false;
	int color=*(int*)pUserData;
	if((srcVal&0xf)-(color&0xF)<0)
		*pOutVal=0;
	else
		*pOutVal=((srcVal&0xf)-(color&0xF))|(color&0xF0);
	return true;
}
void battle::battle_produce_screen(BITMAP *buf)
{
	battlescene.blit_to(buf);
	for(int i=enemy_poses_count-1;i>=0;i--)
		if(enemyteams[enemy_team].enemy[i]>0){
			if(affected_enemies[i])
				enemy_images[i].getsprite(battle_enemy_data[i].frame)->setfilter(brighter_filter,6);
			enemy_images[i].getsprite(battle_enemy_data[i].frame)->blit_middlebottom(buf,battle_enemy_data[i].pos_x,battle_enemy_data[i].pos_y);
			if(affected_enemies[i])
				enemy_images[i].getsprite(battle_enemy_data[i].frame)->setfilter();
		}
	for(int i=Pal::rpg.team_roles;i>=0;i--)
		team_images[i].getsprite(battle_role_data[i].frame)->blit_middlebottom(buf,battle_role_data[i].pos_x,battle_role_data[i].pos_y);

}
void battle::draw_battle_scene()
{
	battle_produce_screen(screen);//fake...
}
void battle::draw_battle_scene_selecting()
{
	draw_battle_scene();
	for(int i=0;i<4;i++)
		UIpics.getsprite(40+i)->blit_filter(screen,instrum_pos[i].x,instrum_pos[i].y,sadden_filter,4,true);
	show_status_bar();
}
int battle::bout_selecting()
{
	do{
		static int selected=0;
		for(int i=0;i<4;i++){
			UIpics.getsprite(40+i)->blit_filter(screen,instrum_pos[i].x,instrum_pos[i].y,sadden_filter,4,i!=selected);
		}
		switch(sync_getkey()){
			case PAL_VK_MENU:
				return -1;
			case PAL_VK_EXPLORE:
				return selected;
			case PAL_VK_UP:
				selected=0;
				break;
			case PAL_VK_LEFT:
				selected=1;
				break;
			case PAL_VK_RIGHT:
				selected=2;
				break;
			case PAL_VK_DOWN:
				selected=3;
				break;
			default:
				break;
		}
	}while(running);
	return 0;
}
battle::battle(int team,int script):battlescene(0,SCREEN_W,SCREEN_H),battlebuf(0,SCREEN_W,SCREEN_H),enemy_team(team),script_escape(script),stage_blow_away(0),magic_wave(0),battle_wave(battlefields[Pal::rpg.battlefield].waving),endbattle_method(0),battle_result(0),escape_flag(0),
                                    flag_withdraw(false),effect_height(200),battle_scene_draw(false),flag_high_attack(false),flag_summon(false),flag_selecting(false),
									role_invisible_rounds(0),enemy_poses_count(0),enemy_exps(0),enemy_money(0),need_battle(true)
{
	memset(affected_enemies,0,sizeof(affected_enemies));
	memset(affected_roles,0,sizeof(affected_roles));

	//确保我方没有开战即死
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]<=0)
			rpg.roles_properties.HP[rpg.team[i].role]=1;

	setup_our_team_data_things();
	
	scene->scenemap.change(0);
	scene->scenemap.change(Pal::scenes[Pal::rpg.scene_id].id);

	flag_battling=1;
	for(int i=0;i<12;i++)
		battle_numbers[i].exist=false;
	//隐藏经验纪录清空,原版盖罗娇不清,修正
	for(int i=0;i<6;i++)
		for(int j=0;j<8;j++)
			rpg.roles_exp[j][i].count=0;

	for(int i=0;i<5;i++){
		if(enemyteams[enemy_team].enemy[i]>0)
			load_enemy(i,enemyteams[enemy_team].enemy[i]);
		if(enemyteams[enemy_team].enemy[i]>=0)
			enemy_poses_count++;
	}
	if(!enemy_poses_count){
		check_end_battle();
		return;
	}

	setup_role_enemy_image();
	setup_role_status();

	musicplayer->play(Pal::rpg.battle_music);

	fbp(Pal::rpg.battlefield).blit_to(battlescene);
	battle_produce_screen(battlebuf);
	for(int i=0;i<=5;i++){
		crossFade_self(i,battlebuf);
		delay(6);
	}

	pal_fade_in(0);
	memset(enemy_HP_r,0,sizeof(enemy_HP_r));

	thebattle=this;
}
int battle::process()
{
	while(running && need_battle){
		static int itemuse_select,item_select,magic_select;
		//战前脚本
		for(int i=0;i<enemy_poses_count;i++)
			if(battle_enemy_data[i].HP>0){
				uint16_t &before_script=battle_enemy_data[i].script.script.before;
				before_script=process_script(before_script,i);
			}
		if(endbattle_method){
			battle_result=endbattle_method;//程序退出
			check_end_battle();
		}
		if(get_member_alive()==0){
			battle_result=1;//我方全灭
			check_end_battle();
		}
		if(get_enemy_alive()==0){
			battle_result=3;//敌方全灭
			check_end_battle();
		}
		
		int instrum_selected=0,instrum_object=0;
		role_counter=0;
		flag_repeat=false;

		//清除物品使用记录
		for(int i=0;i<0x100;i++)
			Pal::rpg.items[i].using_amount=0;

		memset(role_attack_table,0,sizeof(role_attack_table));

		if(flag_autobattle==0){
			for(int i=0;i<=rpg.team_roles;i++)
				battle_role_data[i].frame_bak=0;
			setup_role_status();
			draw_battle_scene_selecting();
		}

		//selection loop
		for(int role=0;role<=rpg.team_roles && running;)
		{
			bool ok=false;
			draw_battle_scene_selecting();
			textout(screen,font,boost::lexical_cast<std::string>(role).c_str(),0,0,0xFA);
			switch(bout_selecting())
			{
			case -1:
				if(role>0)
					role--;
				else
					check_end_battle();
				break;
			case 0:
				break;
			case 1:
				show_money(rpg.coins,10,7,0x15,false);
				magic_select=select_theurgy(rpg.team[role].role,role_status_pack[role].pack.seal?0:2,magic_select,false);
				ok=true;
				break;
			case 2:
				break;
			case 3:
				switch(menu(4,0x10,5,0x38,2)(single_menu(),0))
				{
				case 1:
					switch(itemuse_select=menu(0x18,0x32,2,0x17,2)(single_menu(),itemuse_select))
					{
					case 0:
						item_select=menu_item(item_select,1);
						ok=true;
						break;
					case 1:
						item_select=menu_item(item_select,4);
						ok=true;
						break;
					}
					break;
				case 4:
					role_status();
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			if(!need_battle)
				break;
			if(ok)
				role++;
			perframe_proc();
		}

		delay(1);
	}
	return battle_result;
}
void battle::check_end_battle(){
	need_battle=false;
}
battle::~battle()
{
	thebattle=NULL;
	flag_to_load|=3;
}

int process_Battle(uint16_t enemy_team,uint16_t script_escape)
{
	return battle(enemy_team,script_escape).process();
}

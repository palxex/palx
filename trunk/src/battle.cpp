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

using namespace Pal;

int role_status[TEAMROLES][9];
_battle_role_data battle_role_data[TEAMROLES];
_battle_enemy_data battle_enemy_data[TEAMROLES];

int battle::max_blow_away=0;
battle *battle::thebattle=NULL;

static struct{int x,y;} role_poses[4][4]={{{240,170}},{{200,176},{256,152}},{{180,180},{234,170},{270,146}},{{160,184},{204,175},{246,160},{278,144}}};

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
		if(battle_enemy_data[i].HP>0 && battle_enemy_data[i].id>0)
			enemy_images[i]=sprite_prim(Pal::ABC,rpg.objects[enemyteams[enemy_team].enemy[i]].general.inbeing);
		battle_enemy_data[i].pos_x=enemyposes.pos[i][enemy_poses_count-1].x;
		battle_enemy_data[i].pos_y=enemyposes.pos[i][enemy_poses_count-1].y+monsters[rpg.objects[battle_enemy_data[i].id].enemy.enemy].pos_y_offset;
		battle_enemy_data[i].pos_x_bak=battle_enemy_data[i].pos_x;
		battle_enemy_data[i].pos_y_bak=battle_enemy_data[i].pos_y;
		battle_enemy_data[i].length=enemy_images[i].getsprite(0)->height;
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
		if(role_status[i][2])
			battle_role_data[i].frame=1;
		if(role_status[i][4])
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
	*pOutVal=std::min(srcVal&0xf+color&0xF,0xF)|(color&0xF0);
	return true;
}
void battle::battle_produce_screen()
{
	battlescene.blit_to(screen);
	for(int i=enemy_poses_count-1;i>=0;i--)
		if(enemyteams[enemy_team].enemy[i]>0){
			if(affected_enemies[i])
				enemy_images[i].getsprite(battle_enemy_data[i].frame)->setfilter(brighter_filter,6);
			enemy_images[i].getsprite(battle_enemy_data[i].frame)->blit_to(screen,battle_enemy_data[i].pos_x-enemy_images[i].getsprite(battle_enemy_data[i].frame)->width/2,battle_enemy_data[i].pos_y-enemy_images[i].getsprite(battle_enemy_data[i].frame)->height);
			if(affected_enemies[i])
				enemy_images[i].getsprite(battle_enemy_data[i].frame)->setfilter();
		}
		for(int i=Pal::rpg.team_roles;i>=0;i--)
			team_images[i].getsprite(0)->blit_middle(screen,battle_role_data[i].pos_x,battle_role_data[i].pos_y-team_images[i].getsprite(0)->height/2);

}
battle::battle(int team,int script):battlescene(0,SCREEN_W,SCREEN_H),enemy_team(team),script_escape(script),stage_blow_away(0),magic_wave(0),battle_wave(battlefields[Pal::rpg.battlefield].waving),endbattle_method(0),battle_result(0),escape_flag(0),
                                    flag_withdraw(false),effect_height(200),battle_scene_draw(false),flag_high_attack(false),flag_summon(false),flag_selecting(false),
									role_invisible_rounds(0),enemy_poses_count(0),enemy_exps(0),enemy_money(0)
{
	memset(affected_enemies,0,sizeof(affected_enemies));
	memset(affected_roles,0,sizeof(affected_roles));

	//确保我方没有开战即死
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]<=0)
			rpg.roles_properties.HP[rpg.team[i].role]=1;

	setup_our_team_data_things();
	//scene->scenemap.curr_map=0;
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

	rix->play(Pal::rpg.battle_music);

	bitmap fbp(FBP.decode(Pal::rpg.battlefield),320,200);fbp.blit_to(battlescene);

	thebattle=this;
}
int battle::process()
{
	do{
		battle_produce_screen();
		wait(1);
	}while(running && sync_getkey()==PAL_VK_NONE);
	return 0;
}
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
void battle::check_end_battle(){
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

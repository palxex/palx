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
#include "game.h"
#include "timing.h"
#include "battle.h"
#include "scene.h"
#include "UI.h"
#include "item.h"
#include "fade.h"

using namespace Pal;

union _role_status role_status_pack[TEAMROLES],enemy_status_pack[TEAMENEMIES];
_battle_role_data battle_role_data[TEAMROLES];
_battle_enemy_data battle_enemy_data[TEAMENEMIES];
RPG::POISON_DEF enemy_poison_stack[16][TEAMENEMIES];
int flag_autobattle=0;

bool instrum_usable[4];//指令可用扩展;从而封攻、令成为可能
int twoside_affected[5],delayed_damage[5];//延迟伤害
int x[4],y[4],s[4];

MONSTER &get_monster(int pos)
{
	return monsters[rpg.objects[battle_enemy_data[pos].id].enemy.enemy];
}
int enemy_level_scaler(int enemy,int scaler)
{
	return (get_monster(enemy).level+6)*scaler;
}
void role_restore_position(int role_pos)
{
	battle_role_data[role_pos].pos_x=battle_role_data[role_pos].pos_x_bak;
	battle_role_data[role_pos].pos_y=battle_role_data[role_pos].pos_y_bak;
	battle_role_data[role_pos].frame=battle_role_data[role_pos].frame_bak;
}
int calc_base_damage(double D,double A)
{
	double damage=0;
	if(A<0 || A<0.6*D)
		damage= 0;
	else if(A>D)
		damage= A-0.8*D;
	else if(A>0.6*D)
		damage= 0.5*A-0.3*D;
	return round(damage);
}
int calc_final_damage(double A,int enemy,int magic)
{
	double D=get_monster(enemy).defence+enemy_level_scaler(enemy,4);
	double damage=calc_base_damage(A*(1+rnd1(0.1)),D);
	damage=damage/2+Pal::magics[magic].base_damage;
	if(magics[magic].elem_attr)
		damage=damage*(10-get_monster(enemy).elem_defences[magics[magic].elem_attr])/5;
	damage=damage*(10+Pal::battlefields[rpg.battlefield].elem_property[magics[magic].elem_attr])/10;
	return round(damage);
}

battle *battle::thebattle=NULL;

struct{int x,y;} const role_poses[4][4]={{{240,170}},{{200,176},{256,152}},{{180,180},{234,170},{270,146}},{{160,184},{204,175},{246,160},{278,144}}},
	instrum_pos[4]={{0x1C,0x8C},{0,0x9B},{0x37,0x9B},{0x1B,0xAA}};

int enemy_sequence[5]={2,1,0,4,3};

void battle::display_damage_number(int color,int num,int x,int y)
{
	if(num)
		for(int i=0;i<12;i++)
			if(battle_numbers[i].times<=0){
				battle_numbers[i].num=num;
				battle_numbers[i].times=10;
				battle_numbers[i].x=x;
				battle_numbers[i].y=y;
				battle_numbers[i].color=color;
				if(battle_numbers[i].y<15)
					battle_numbers[i].y=15;
				break;
			}
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
void battle::setup_role_enemy_image()
{
	for(int i=0;i<=rpg.team_roles;i++){
		team_images[i]=sprite_prim(F,battle_role_data[i].battle_avatar);//Pal::rpg.roles_properties.battle_avator[Pal::rpg.team[i].role]
		battle_role_data[i].pos_x=role_poses[rpg.team_roles][i].x;
		battle_role_data[i].pos_y=role_poses[rpg.team_roles][i].y;
		battle_role_data[i].pos_x_bak=battle_role_data[i].pos_x;
		battle_role_data[i].pos_y_bak=battle_role_data[i].pos_y;
	}
	for(int i=0;i<enemy_poses_count;i++){
		if(battle_enemy_data[i].HP>0 && battle_enemy_data[i].id>0){
			enemy_images[i]=sprite_prim(Pal::ABC,battle_enemy_data[i].battle_avatar);//rpg.objects[enemyteams[enemy_team].enemy[i]].general.inbeing);
			battle_enemy_data[i].length=enemy_images[i].getsprite(0)->height;
		}
		battle_enemy_data[i].pos_x=enemyposes.pos[i][enemy_poses_count-1].x;
		battle_enemy_data[i].pos_y=enemyposes.pos[i][enemy_poses_count-1].y+get_monster(i).pos_y_offset;
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
		role_action_table[i].alive=0;
		if(rpg.roles_properties.HP[role]<100 && rpg.roles_properties.HP[role]<=rpg.roles_properties.HP_max[role]/5)
			battle_role_data[i].frame=1;
		if(role_status_pack[i].pack.sleep)
			battle_role_data[i].frame=1;
		if(role_status_pack[i].pack.dummy)
			battle_role_data[i].frame=0;
		if(rpg.roles_properties.HP[role]<=0){
			battle_role_data[i].frame=2;
			if(battle_role_data[i].frame_bak!=2)
				role_action_table[i].alive=-1;
		}
		battle_role_data[i].frame_bak=battle_role_data[i].frame;
		if(battle_role_data[i].frame<=1){
			battle_role_data[i].pos_x=battle_role_data[i].pos_x_bak;
			battle_role_data[i].pos_y=battle_role_data[i].pos_y_bak;
		}
	}
}
int battle::get_member_alive()
{
	int n=0;
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]>0)
			n++;
	return n;
}
int battle::get_enemy_alive()
{
	int n=0;
	for(int i=0;i<enemy_poses_count;i++)
		if(battle_enemy_data[i].HP>0)
			n++;
	return n;
}
int battle::select_targetting_enemy()
{
	if(get_enemy_alive()==1)
		for(int i=0;i<enemy_poses_count;i++)
			if(battle_enemy_data[i].HP>0)
				return targetting_enemy=i;
	int trend=1;
	targetting_enemy=0;
	while(running){
		switch(async_getkey())
		{
		case PAL_VK_LEFT:
			trend=-1;
			targetting_enemy--;
			break;
		case PAL_VK_RIGHT:
			trend=1;
			targetting_enemy++;
			break;
		case PAL_VK_MENU:
			return -1;
		case PAL_VK_EXPLORE:
			return targetting_enemy;
		default:
			break;
		}
		targetting_enemy=(targetting_enemy+enemy_poses_count)%enemy_poses_count;
		while(battle_enemy_data[targetting_enemy].HP<=0)
			targetting_enemy=(targetting_enemy+trend+enemy_poses_count)%enemy_poses_count;
		draw_battle_scene(0,1);
	}
	return targetting_enemy;
}
int battle::select_targetting_role(){
	if(get_member_alive()==1)
		for(int i=0;i<=rpg.team_roles;i++)
			if(rpg.roles_properties.HP[i]>0)
				return targetting_role=i;
	int trend=1;
	targetting_role=0;
	while(running){
		switch(async_getkey())
		{
		case PAL_VK_LEFT:
			trend=-1;
			targetting_role--;
			break;
		case PAL_VK_RIGHT:
			trend=1;
			targetting_role++;
			break;
		case PAL_VK_MENU:
			return -1;
		case PAL_VK_EXPLORE:
			return targetting_role;
		default:
			trend=0;
			break;
		}
		targetting_role=(targetting_role+rpg.team_roles+1)%(rpg.team_roles+1);
		while(rpg.roles_properties.HP[targetting_role]<=0)
			targetting_role=(targetting_role+trend+rpg.team_roles+1)%(rpg.team_roles+1);
		draw_battle_scene(0,1);
	}
	return targetting_role;
}
int battle::select_a_living_role_randomly()
{
	int selected;
	do
		selected=round(rnd1(rpg.team_roles));
	while(selected>rpg.team_roles || rpg.roles_properties.HP[rpg.team[selected].role]<=0);
	return selected;
}
void battle::bright_every_role(int begin,int end)
{
	for(int pass=1;pass<=15;pass++){
		delay(2);
		for(int pos=begin;pos<=end;pos++)
			team_images[pos].getsprite(battle_role_data[pos].frame)->blit_filter(screen,battle_role_data[pos].pos_x,battle_role_data[pos].pos_y,brighter_filter,pass,true,true);
	}
}

void battle::battle_produce_screen(BITMAP *buf)
{
	battlescene.blit_to(buf);
	for(int i=enemy_poses_count-1;i>=0;i--)
		if(battle_enemy_data[i].HP>0){
			enemy_images[i].getsprite(battle_enemy_data[i].frame)->blit_filter(buf,battle_enemy_data[i].pos_x,battle_enemy_data[i].pos_y,brighter_filter,6,affected_enemies[i],true);
		}

	if(flag_summon){
		//drew_scenes_summon
	}
	else
		if(role_invisible_rounds==0)
			for(int i=Pal::rpg.team_roles;i>=0;i--)//应按posY排序保证不遮挡。实现到再说
				team_images[i].getsprite(battle_role_data[i].frame)->blit_middlebottom(buf,battle_role_data[i].pos_x,battle_role_data[i].pos_y);

}
void battle::draw_battle_scene(int delaytime,int times,BITMAP *bmp)
{
	if(times==0)
		times=1;
	stage_blow_away+=round(rnd1(max_blow_away));
	for(int t=1;t<=times;t++)
	{
		bitmap scanline(0,SCREEN_W,effective_y);
		if(battlefield_waving+magic_waving)
			wave_screen(battlescene,scanline,battlefield_waving+magic_waving,effective_y,shake_viewport_y);
		else
			battlescene.blit_to(scanline,0,shake_viewport_y,0,0);
		for(int e=enemy_poses_count-1;e>=0;e--)
		{
			int enemy=rpg.objects[enemyteams[enemy_team].enemy[e]].enemy.enemy;
			int frames=monsters[enemy].vibra_frames;
			if(battle_enemy_data[e].HP<=0 && store_for_diff.enemies[e].HP<=0)
				continue;
			if(!frames)
				continue;
			if(enemy_moving_semaphor && frames<99 && !( enemy_status_pack[e].pack.fixed || enemy_status_pack[e].pack.sleep)	&& (drawlist_parity%(frames)==0))//manually slow down,origin doesn't *2
				battle_enemy_data[e].frame=(battle_enemy_data[e].frame+1)%monsters[enemy].stand_frames;
			if(flag_selecting && (drawlist_parity&1) && (targetting_enemy == e))
				affected_enemies[e]=1;
			int crazybits=0;
			if(enemy_status_pack[e].pack.crazy)
				crazybits=round(rnd1(3));
			sprites.push(boost::shared_ptr<sprite>(enemy_images[e].getsprite(battle_enemy_data[e].frame)->clone()->setXYL(battle_enemy_data[e].pos_x+crazybits+stage_blow_away,battle_enemy_data[e].pos_y+sth_about_y+stage_blow_away/2,0)));
		}
		if(flag_summon)
			;//add_summon_img
		else{
			for(int r=rpg.team_roles;r>=0;r--)
			{
				int crazybits=0;
				if(role_status_pack[r].pack.crazy)
					crazybits=round(rnd1(3));
				if(role_invisible_rounds==0){
					sprites.push(boost::shared_ptr<sprite>(team_images[r].getsprite(battle_role_data[r].frame)->clone()->setXYL(battle_role_data[r].pos_x+crazybits,battle_role_data[r].pos_y,0)));
				}
				if(flag_selecting)
				{
					if(targetting_role == r)
						sprites.push(boost::shared_ptr<sprite>(UIpics.getsprite(66+(drawlist_parity&1))->clone()->setXYL(battle_role_data[r].pos_x-4,battle_role_data[r].pos_y,0x3D)));
					if(commanding_role == r)
						sprites.push(boost::shared_ptr<sprite>(UIpics.getsprite(68+(drawlist_parity&1))->clone()->setXYL(battle_role_data[r].pos_x-4,battle_role_data[r].pos_y,0x44)));
				}
			}
		}
		add_occuring_magic_to_drawlist(false);
		for(int i=0;i<12;i++)
			if(battle_numbers[i].times){
				int num=battle_numbers[i].num,posx=battle_numbers[i].x+6,layer=999,color=(battle_numbers[i].color==0?0x13:(battle_numbers[i].color==1?0x1D:(battle_numbers[i].color==2?0x38:0)));
				if(num>=0)
					do
						sprites.push(boost::shared_ptr<sprite>(Pal::UIpics.getsprite(color+num%10)->clone()->setXYL(posx-=6,battle_numbers[i].y+layer,layer)));
					while((num/=10)>0);
				battle_numbers[i].times--;
				battle_numbers[i].y--;
			}

		sprites.flush(scanline);
		sprites.clear_active();
		scanline.blit_to(bmp);

		for(int i=0;i<enemy_poses_count;i++)
			if(affected_enemies[i])
				enemy_images[i].getsprite(battle_enemy_data[i].frame)->blit_filter(bmp,battle_enemy_data[i].pos_x+stage_blow_away,battle_enemy_data[i].pos_y+stage_blow_away/2,brighter_filter,6,true,true);
		for(int i=0;i<rpg.team_roles;i++)
			if(affected_roles[i])
				team_images[i].getsprite(battle_role_data[i].frame)->blit_filter(bmp,battle_role_data[i].pos_x,battle_role_data[i].pos_y,brighter_filter,6,true,true);

		delay(delaytime+5);
		shake_screen();
		drawlist_parity++;

		//battle_produce_screen(bmp);//fake...
	}
	memset(affected_enemies,0,sizeof(affected_enemies));
	memset(affected_roles,0,sizeof(affected_roles));
}
void battle::draw_battle_scene_selecting()
{
	effective_y=200;
	bitmap buf;
	draw_battle_scene(-4,1,buf);
	effective_y=140;
	for(int i=0;i<4;i++)
		UIpics.getsprite(40+i)->blit_filter(buf,instrum_pos[i].x,instrum_pos[i].y,sadden_filter,4,true);
	show_status_bar(buf);
	buf.blit_to(screen);
}
int battle::select_an_enemy_randomly()
{
	return 0;
}
int battle::bout_selecting(int &selected)
{
	do{
		for(int i=0;i<4;i++){
			if(instrum_usable[i])
				UIpics.getsprite(40+i)->blit_filter(screen,instrum_pos[i].x,instrum_pos[i].y,sadden_filter,4,i!=selected);
			else
				UIpics.getsprite(40+i)->blit_filter(screen,instrum_pos[i].x,instrum_pos[i].y,sadden_filter,0x14,true);
		}
		draw_battle_scene(0,1);
		switch(PAL_VKEY vkey=async_getkey()){
			case PAL_VK_MENU:
				return -1;
			case PAL_VK_EXPLORE:
				return selected;
			case PAL_VK_UP:
				if(instrum_usable[0])
					selected=0;
				break;
			case PAL_VK_LEFT:
				if(instrum_usable[1])
					selected=1;
				break;
			case PAL_VK_RIGHT:
				if(instrum_usable[2])
					selected=2;
				break;
			case PAL_VK_DOWN:
				if(instrum_usable[3])
					selected=3;
				break;
			case PAL_VK_REPEAT:
			case PAL_VK_AUTO:
			case PAL_VK_DEFEND:
			case PAL_VK_USE:
			case PAL_VK_THROW:
			case PAL_VK_QUIT:
			case PAL_VK_STATUS:
			case PAL_VK_FORCE:
				return vkey+100;//modified modulation algo.
			default:
				//draw_battle_scene_selecting();
				break;
		}
	}while(running);
	return 0;
}
battle::battle(int team,int script):enemy_team(team),script_escape(script),stage_blow_away(0),magic_waving(0),battlefield_waving(battlefields[Pal::rpg.battlefield].waving),endbattle_method(NOT),battle_result(NOT),escape_flag(NOT),
									max_blow_away(0),role_invisible_rounds(0),action_taker(0),flag_attacking_hero(false),enemy_poses_count(0),
									flag_withdraw(false),effect_height(200),battle_scene_draw(false),magic_image_occurs(0),flag_summon(false),flag_selecting(false),
									enemy_exps(0),enemy_money(0),need_battle(true),drawlist_parity(0),sth_about_y(0),effective_y(200),flag_second_attacking(false),
									auto_selected_enemy(0),battle_sfx(0),magic_frame(0),shake_viewport_y(0),drew_scenes_summon(0)
{
	memset(&store_for_diff,0,sizeof(store_for_diff));
	memset(affected_enemies,0,sizeof(affected_enemies));
	memset(affected_roles,0,sizeof(affected_roles));
	memset(battle_numbers,0,sizeof(battle_numbers));
	memset(bak_action_table,0,sizeof(bak_action_table));

	//确保我方没有开战即死
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]<=0)
			rpg.roles_properties.HP[rpg.team[i].role]=1;

	setup_our_team_data_things();

	scene->scenemap.change(0);
	//scene->scenemap.change(Pal::scenes[Pal::rpg.scene_id].id);
	flag_battling=true;
	sprites.clear_active();

	for(int i=0;i<12;i++)
		battle_numbers[i].times=0;
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
	backupBackground();
	bitmap battle_screen;
	battle_produce_screen(battle_screen);
	for(int i=0;i<=5;i++){
		crossFade_self(i,battle_screen);
		delay(6);
	}

	pal_fade_in(0);
	restoreBackground();
	memset(store_for_diff.enemies,0,sizeof(store_for_diff.enemies));

	thebattle=this;
}
battle::END battle::process()
{
	int summon_magic=0;
	while(running && need_battle){
		static int action_select,itemuse_select,item_select,magic_select;
		int instrum_selected=0,instrum_object=0;
		//战前脚本
		for(int i=0;i<enemy_poses_count;i++)
			if(battle_enemy_data[i].HP>0){
				uint16_t &before_script=battle_enemy_data[i].script.script.before;
				before_script=process_script(before_script,i);
			}
		if(endbattle_method){
			battle_result=endbattle_method;//程序退出
			return check_end_battle();
		}
		if(get_member_alive()==0){
			battle_result=ROLE_FAIL;//我方全灭
			return check_end_battle();
		}
		if(get_enemy_alive()==0){
			battle_result=ENEMY_FAIL;//敌方全灭
			return check_end_battle();
		}

		flag_repeat=false;

		//清除物品使用记录
		for(int i=0;i<0x100;i++)
			Pal::rpg.items[i].using_amount=0;

		memset(role_action_table,0,sizeof(role_action_table));

		if(flag_autobattle==0){
			for(int i=0;i<=rpg.team_roles;i++)
				battle_role_data[i].prev_frame=0;
			setup_role_status();
			draw_battle_scene_selecting();
		}

		//role command loop
		for(commanding_role=0;commanding_role<=rpg.team_roles && running;)
		{
			enemy_moving_semaphor=true;
			magic_image_occurs=0;
			if(role_status_pack[commanding_role].pack.dummy){
				role_action_table[commanding_role].target=select_an_enemy_randomly();
				role_action_table[commanding_role].action=ATTACK;
				commanding_role++;
				continue;//MASK:not completely port. Original version don't reset the first bools again.
			}
			if(role_status_pack[commanding_role].pack.crazy || role_status_pack[commanding_role].pack.fixed || role_status_pack[commanding_role].pack.sleep){
				draw_battle_scene(0,4);
				commanding_role++;
				continue;//MASK:not completely port. Original version don't reset the first bools again.
			}
			if(rpg.roles_properties.HP[rpg.team[commanding_role].role]<=0){
				commanding_role++;
				continue;//MASK:not completely port. Original version don't reset the first bools again.
			}

			//消灭上回合用掉的东东的记录
			for(int i=commanding_role;i<=rpg.team_roles;i++)
				if(role_action_table[i].action==USE_ITEM || role_action_table[i].action==THROW_ITEM){
					if(rpg.items[role_action_table[i].target].using_amount)
						rpg.items[role_action_table[i].target].using_amount--;
					role_action_table[i].action=ATTACK;//？那R……
				}

			if(flag_autobattle)
				break;

			targetting_role=-1;targetting_enemy=-1;
			flag_selecting=true;

			memset(instrum_usable,1,sizeof(instrum_usable));
			if(rpg.team_roles==0 || battle_role_data[commanding_role].frame)
				instrum_usable[2]=0;
			if(role_status_pack[commanding_role].pack.seal)
				instrum_usable[1]=0;


			bool ok=false;//clear_keybuf();
			bool ck=false;
			switch(bout_selecting(instrum_selected))
			{
			case -1:
				if(commanding_role>0)
					commanding_role--;
				else{
					battle_result=ENEMY_FAIL;
					return check_end_battle();
				}
				break;
			case 0:
				ok=true;
				if(rpg.roles_properties.attack_all[rpg.team[commanding_role].role])
					role_action_table[commanding_role].action=ATTACK_ALL;
				else if((targetting_enemy=select_targetting_enemy())>=0){
					role_action_table[commanding_role].target=targetting_enemy;
					role_action_table[commanding_role].action=ATTACK;
				}
				else
					ok=false;
				break;
			case 1:
				{
					bool tk;
					do{
						tk=false;
						show_money(rpg.coins,10,7,0x15,false);
						magic_select=select_theurgy(rpg.team[commanding_role].role,role_status_pack[commanding_role].pack.seal?0:2,instrum_object,false);
						draw_battle_scene_selecting();
						if(instrum_object>=0){
							role_action_table[commanding_role].tool=magic_select;
							role_action_table[commanding_role].toolpos=instrum_object;
							ok=true;
							int prop=rpg.objects[rpg.roles_properties.magics[instrum_object][rpg.team[commanding_role].role]].magic.param;
							if((prop>>TARGET_ENEMY)&1){
								if((prop>>OBJECT_ALL)&1)
									role_action_table[commanding_role].target=commanding_role;
								else{
									int enemy=select_targetting_enemy();
									if(enemy<0){
										tk=true;
										continue;
									}
									else
										role_action_table[commanding_role].target=enemy;
								}
								role_action_table[commanding_role].action=MAGIC_TO_ENEMY;
							}else{
								if((prop>>OBJECT_ALL)&1)
									role_action_table[commanding_role].target=commanding_role;
								else{
									int role=select_targetting_role();
									if(role<0){
										tk=true;
										continue;
									}
									else
										role_action_table[commanding_role].target=role;
								}
								role_action_table[commanding_role].action=MAGIC_TO_US;
							}
						}
					}while(tk);
					break;
				}
			case 2:
				{
					int target;
					if( (rpg.objects[battle_role_data[commanding_role].contract_magic].magic.param>>OBJECT_ALL) &1)
						target=select_targetting_enemy();
					else
						target=0;
					if(target>=0){
						for(int r=0;r<=rpg.team_roles;r++)
							role_action_table[commanding_role].action=(ACTION)-1;
						role_action_table[commanding_role].action=COSTAR;
						commanding_role=rpg.team_roles;
						ok=true;
					}
				}
				break;
			case 3:
				{
					bool tk;
					do{
						tk=false;
						int tk2;
						switch(tk2=menu(4,0x10,5,0x38,2)(single_menu(),action_select))
						{
						case -2:
						case -1:
							draw_battle_scene_selecting();
							break;
						case 0:
							auto_attack();
							ok=true;
							break;
						case 1:
							{
								bool tk3;
								do{
									tk3=false;
									menu(0x18,0x32,2,0x17,2)(single_menu(),itemuse_select);
									if(itemuse_select>=0)
										use_or_throw(itemuse_select,item_select,tk3);
									else
										tk=true;
								}while(tk3);
							}
							if(tk){
								draw_battle_scene_selecting();
								continue;
							}
							ok=true;
							break;
						case 2:
							role_action_table[commanding_role].action=DEFENCE;
							ok=true;
							break;
						case 3:
							escape();
							ok=true;
							break;
						case 4://
							status();
							break;
						default:
							break;
						}
					}while(tk);
				}
				break;
			case PAL_VK_REPEAT+100://fill!!!
				flag_repeat=true;
				for(int r=commanding_role;r<=rpg.team_roles;r++){
					memcpy(role_action_table+r,bak_action_table+r,sizeof(_role_attack));
					switch(ACTION &action=role_action_table[r].action){//follow the original order
					case ATTACK_ALL:
						if(!rpg.roles_properties.attack_all[r])
							action=ATTACK;
						break;
					case ATTACK:
						if(rpg.roles_properties.attack_all[r])
							action=ATTACK_ALL;
						break;
					case MAGIC_TO_US:
					case MAGIC_TO_ENEMY:
						{
							int pos=get_magic_pos(r,role_action_table[r].tool);
							role_action_table[r].toolpos=pos-0x20;
							if(pos==0 || role_status_pack[r].pack.fixed || (rpg.roles_properties.MP[r]<magics[rpg.objects[role_action_table[r].tool].magic.magic].power_used))
								if(action==MAGIC_TO_US)
									action=DEFENCE;
								else if(action==MAGIC_TO_ENEMY)
									action=ATTACK;
						}
						break;
					case USE_ITEM:
					case THROW_ITEM:
						{
							int pos=std::find(Pal::rpg.items,Pal::rpg.items+0x100,role_action_table[r].tool)-rpg.items;
							if(pos!=0x100 || rpg.items[pos].amount < rpg.items[pos].using_amount){//not found,used up; or will use up in this bout
								role_action_table[r].toolpos=pos;
								if(action==USE_ITEM)
									action=DEFENCE;
								else if(action==THROW_ITEM)
									action=ATTACK;
								}
						}
						break;
					}
				}
				commanding_role=rpg.team_roles;
				ok=true;
				break;
			case PAL_VK_AUTO+100:
				auto_attack();
				ok=true;
				break;
			case PAL_VK_DEFEND+100:
				role_action_table[commanding_role].action=DEFENCE;
				ok=true;
				break;
			case PAL_VK_USE+100:
				use_or_throw(0,item_select,ck);
				ok=true;
				break;
			case PAL_VK_THROW+100:
				use_or_throw(1,item_select,ck);
				ok=true;
				break;
			case PAL_VK_QUIT+100:
				escape();
				ok=true;
				break;
			case PAL_VK_STATUS+100:
				status();
				break;
			case PAL_VK_FORCE+100:
				ok=true;
				for(int r=commanding_role;r<=rpg.team_roles;r++){
					//bugfix:damn force
					if(role_status_pack[r].pack.dummy)
						continue;
					role_action_table[r].target=select_an_enemy_randomly();
					int force_magic=0;
					for(int magic_id=0,role_id=rpg.team[r].role,p_max=0,max=0;magic_id<0x20;magic_id++)
						if(rpg.roles_properties.magics[magic_id][role_id]>0){
							MAGIC &magic_counter=magics[rpg.objects[rpg.roles_properties.magics[magic_id][role_id]].magic.magic];
							if(rpg.roles_properties.MP[role_id]>=magic_counter.power_used)
								if(magic_counter.behavior==9 || magic_counter.behavior<=3)
									if(magic_counter.power_used!=1){//avoid the powerest magics
										int power=magic_counter.base_damage+round(rnd1(60));
										if(power>p_max){
											p_max=max=power;
											force_magic=rpg.roles_properties.magics[magic_id][role_id];
										}
									}
						}
						if(force_magic>0 && !role_status_pack[r].pack.seal){
							role_action_table[r].action=MAGIC_TO_ENEMY;
							role_action_table[r].tool=force_magic;
						}else
							role_action_table[r].action=ATTACK;
				}
				commanding_role=rpg.team_roles;
				break;
			default:
				if(role_action_table[commanding_role].action==ESCAPE)
					commanding_role=rpg.team_roles;
				instrum_selected=0;instrum_object=0;
				draw_battle_scene_selecting();
				break;
			}
			if(!need_battle)
				break;
			if(ok){
				if(role_action_table[commanding_role].action==ESCAPE)//duplicate code,but how to eliminate...
					commanding_role=rpg.team_roles;
				commanding_role++;
				instrum_selected=0;instrum_object=0;
			}
			perframe_proc();
		}
		if(!running)
			break;
		flag_selecting=false;
		effective_y=200;
		draw_battle_scene(0,1);

		vs_table.clear();
		//enemy fill vs_tbl
		for(int commanding_enemy=0;commanding_enemy<enemy_poses_count;commanding_enemy++)
		{
			if(battle_enemy_data[commanding_enemy].HP<=0)
				continue;
			int speed=(int)((get_monster(commanding_enemy).speed+enemy_level_scaler(commanding_enemy,3))*(0.9+rnd1(0.2)));
			for(int action=0,actions=round(rnd1(get_monster(commanding_enemy).flag_twice_action)>0?1:0);action<=actions;action++)
				vs_table.insert(std::make_pair(speed,commanding_enemy+100));
		}

		//role fill vs_tbl
		for(int commanding_role=0;commanding_role<=rpg.team_roles;commanding_role++)
		{
			if(!flag_repeat)
				bak_action_table[commanding_role]=role_action_table[commanding_role];
			int speed=(int)(get_cons_attrib(rpg.team[commanding_role].role,0x14)*(0.9+rnd1(0.2)));
			switch(role_action_table[commanding_role].action)
			{
			case COSTAR:
				speed*=10;
				break;
			case DEFENCE:
				speed*=5;
				break;
			case MAGIC_TO_US:
			case USE_ITEM:
				speed*=3;
				break;
			case ESCAPE:
				speed/=2;
				break;
			default:
				if(role_status_pack[commanding_role].pack.high_speed)
					speed*=3;
				if(battle_role_data[commanding_role].frame==1)//weak
					speed/=2;
				if(battle_role_data[commanding_role].frame==2)//dead
					speed=0;
				break;
			}
			if(!speed)
				continue;
			vs_table.insert(std::make_pair(speed,commanding_role));
		}//留神因为vs_table的不同定义导致的后期动态问题

		//action loop
		for(std::multimap<int,int,std::greater<int> >::iterator vs_action=vs_table.begin(),prev_action=vs_action;vs_action!=vs_table.end();prev_action=(vs_action++))
		{
			flag_invisible=0;
			flag_second_attacking=false;
			if(vs_action->second>=100){
				//enemy action
				if(role_invisible_rounds==0){
					action_taker=vs_action->second-100;
					if(enemy_status_pack[action_taker].pack.fixed==0 && enemy_status_pack[action_taker].pack.sleep==0){
						if(vs_action!=vs_table.begin() && vs_action->second==prev_action->second)
							flag_second_attacking=true;
						if(battle_enemy_data[action_taker].HP>0)
							enemy_attack_role(action_taker,select_a_living_role_randomly());
					}
				}
			}
			else{
				//role action
				action_taker=vs_action->second;
				int attacking_role=rpg.team[action_taker].role;
				if(action_taker>=0 && (rpg.roles_properties.HP[attacking_role]>0 || role_status_pack[action_taker].pack.dummy>0)
					&& role_status_pack[action_taker].pack.sleep<=0 && role_status_pack[action_taker].pack.fixed<=0)
				{
					if(flag_autobattle>=2){
						role_action_table[action_taker].target=auto_selected_enemy;
						if(flag_autobattle==2)//菜单:围攻
							role_action_table[action_taker].action=ATTACK;
						else if(flag_autobattle==9){//自动战
							role_action_table[action_taker].action=MAGIC_TO_ENEMY;
							role_action_table[action_taker].tool=rpg.roles_properties.magics[round(rnd1(4))][attacking_role];//只能自动前4招……
						}
					}
					if(role_status_pack[action_taker].pack.crazy>0)
						role_action_table[action_taker].action=CRAZY_ATTACK;
					int &target_enemy=role_action_table[action_taker].target;
					while(battle_enemy_data[target_enemy].HP<=0)
						target_enemy=(target_enemy+1)%enemy_poses_count;
					flag_withdraw=false;flag_attacking_hero=false;battle_sfx=0;
					int enemies_before=get_enemy_alive();
					for(int i=0;i<5;i++)
						store_for_diff.enemies[i].HP=battle_enemy_data[i].HP;
					switch(int action=role_action_table[action_taker].action)//fill!!!
					{
					case ATTACK:
attack_label:
						if(rpg.roles_properties.attack_all[attacking_role])
							goto attack_all_label;
						{
							int total_damage=0;
							for(int bout=0,bouts=(role_status_pack[action_taker].pack.twice_attack>0?1:0);bout<=bouts;bout++)
							{
								int damage=calc_base_damage(get_monster(target_enemy).defence+enemy_level_scaler(target_enemy,4),get_cons_attrib(attacking_role,0x11));
								damage=damage*2/get_monster(target_enemy).weapon_defence;
								role_physical_attack(action_taker,target_enemy,damage,bouts);
								total_damage+=damage;
							}
							role_restore_position(action_taker);
							draw_battle_scene(0,1);
							battle_enemy_data[target_enemy].HP-=total_damage;
							if(battle_enemy_data[target_enemy].HP<=0)
								battle_sfx=get_monster(target_enemy).death_sfx;
							else{
								battle_enemy_data[target_enemy].pos_x=battle_enemy_data[target_enemy].pos_x_bak;
								battle_enemy_data[target_enemy].pos_y=battle_enemy_data[target_enemy].pos_y_bak;
							}
							rpg.roles_exp[1][attacking_role].count+=round(rnd1(2));
							rpg.roles_exp[3][attacking_role].count++;
						}
						break;
					case MAGIC_TO_US:
						{
							int power=get_cons_attrib(attacking_role,0x12);
							int magic_index=rpg.objects[role_action_table[action_taker].tool].magic.magic;
							role_release_magic_action(action_taker,Pal::magics[magic_index].behavior!=8);
							uint16_t &pre_script=Pal::rpg.objects[role_action_table[action_taker].tool].magic.occur;
							pre_script=process_script(pre_script,action_taker);
							if(prelimit_OK){
								if(magics[magic_index].behavior==8)
									bright_every_role(action_taker,action_taker);
								if(magics[magic_index].effect>=0)
									role_release_magic(power,role_action_table[action_taker].tool,role_action_table[action_taker].target,action_taker);
								rpg.roles_properties.MP[attacking_role]-=magics[magic_index].power_used;
								show_role_changes(action_taker,role_action_table[action_taker].tool);
								if(magics[magic_index].behavior==8){
									battle_role_data[action_taker].pos_x=battle_role_data[action_taker].pos_x_bak;
									battle_role_data[action_taker].pos_y=battle_role_data[action_taker].pos_y_bak;
									setup_role_enemy_image();
									flag_invisible=-1;
								}
								rpg.roles_exp[3][attacking_role].count+=round(rnd1(2));
								rpg.roles_exp[5][attacking_role].count++;
							}
						}
						break;
					case MAGIC_TO_ENEMY:
						{
							flag_summon=false;
							int power=get_cons_attrib(attacking_role,0x12);
							int magic_index=rpg.objects[role_action_table[action_taker].tool].magic.magic;
							role_release_magic_action(action_taker,magics[magic_index].behavior!=9);
							uint16_t &pre_script=rpg.objects[role_action_table[action_taker].tool].magic.occur;
							pre_script=process_script(pre_script,action_taker);
							if(prelimit_OK){
								if(magics[magic_index].behavior==9){
									summon_magic=magic_index;
									flag_summon=true;
									summon_imgs(role_action_table[action_taker].tool);
									role_restore_position(action_taker);
								}
								role_release_magic(power,role_action_table[action_taker].tool,role_action_table[action_taker].target,(magics[magic_index].behavior==9)?9:action_taker);
								uint16_t &post_script=rpg.objects[role_action_table[action_taker].tool].magic.post;
								post_script=process_script(post_script,role_action_table[action_taker].target);
								attack_make();
								if(check_enemy_changes())
									show_enemy_changes(5);
								rpg.roles_properties.MP[attacking_role]-=magics[magic_index].power_used;
								rpg.roles_exp[3][attacking_role].count+=round(rnd1(2));
								rpg.roles_exp[5][attacking_role].count++;
							}
						}
						break;
					case USE_ITEM:
						break;
					case THROW_ITEM:
						break;
					case DEFENCE:
						battle_role_data[action_taker].frame=3;
						battle_role_data[action_taker].prev_frame=3;
						draw_battle_scene(0,4);
						rpg.roles_exp[5][attacking_role].count+=2;
						break;
					case AUTO_ATTACK:
						flag_autobattle=2;
						goto attack_label;
						break;
					case COSTAR:
						break;
					case ESCAPE:
						break;
					case CRAZY_ATTACK:
						{
							int target=select_a_living_role_randomly();
							if(target!=action_taker)
								role_crazy_attack_team(action_taker,target);
						}
						break;
					case ATTACK_ALL:
attack_all_label:
						break;
					default:
						break;
					}
					if(get_enemy_alive()!=enemies_before){
						flag_invisible=-1;
						voc(SFX.decode(battle_sfx)).play();
					}
				}
			}

			for(int i=0;i<=4;i++)
				store_for_diff.enemies[i].HP=battle_enemy_data[i].HP;

			if(flag_invisible){
				clear_effective(0x41,1);
				flag_invisible=0;
			}

			for(int i=0;i<12;i++)
				battle_numbers[i].times=0;

			if(flag_summon){
				setup_role_enemy_image();
				flag_summon=false;
				serials_of_fade(summon_magic);
			}

			if(flag_withdraw && get_enemy_alive()>0){
				;//got back when got attacked;
				flag_withdraw=false;
			}

			setup_role_status();

			if(get_member_alive()==0){
				battle_result=ROLE_FAIL;
				return check_end_battle();
			}
			if(get_enemy_alive()==0){
				battle_result=ENEMY_FAIL;
				return check_end_battle();
			}
			if(escape_flag){
				battle_result=escape_flag;
				return check_end_battle();
			}
		}
		int times=2;

		//敌我中毒、伤害显示

		draw_battle_scene(0,10);//10注意

		//备份下一轮的diff数据

		if(role_invisible_rounds)
			if(role_invisible_rounds--)
				flag_invisible=1;

		if(flag_invisible){
			clear_effective(0x41,1);
			flag_invisible=0;
		}

		if(flag_autobattle<=2)
			if(key[KEY_ESC] || key[KEY_INSERT])
				flag_autobattle=0;
	}
	return check_end_battle();
}
void battle::show_role_changes(int action_taker,int magic)
{
	for(int i=0;i<=rpg.team_roles;i++){
		store_for_diff.roles[i].HP=rpg.roles_properties.HP[rpg.team[i].role];
		store_for_diff.roles[i].MP=rpg.roles_properties.MP[rpg.team[i].role];
	}
	uint16_t &post_script=rpg.objects[magic].magic.post;
	post_script=process_script(post_script,role_action_table[action_taker].target);
	if(check_role_changes()){
		draw_battle_scene(0,7);
		setup_role_status();
	}
}
bool battle::check_role_changes(){
	bool changed=false;
	for(int i=0;i<=rpg.team_roles;i++){
		if(store_for_diff.roles[i].HP-=rpg.roles_properties.HP[rpg.team[i].role]){
			display_damage_number((store_for_diff.roles[i].HP<0)?1:0,abs(store_for_diff.roles[i].HP),battle_role_data[i].pos_x,battle_role_data[i].pos_y-0x46);
			changed=true;
		}
		if((store_for_diff.roles[i].MP-=rpg.roles_properties.MP[rpg.team[i].role])>0){
			display_damage_number(2,store_for_diff.roles[i].MP,battle_role_data[i].pos_x,battle_role_data[i].pos_y-0x3E);
			changed=true;
		}
	}
	return changed;
}
void battle::attack_make()
{
	for(int i=0;i<enemy_poses_count;i++)
		if(twoside_affected[i]){
			battle_enemy_data[i].HP-=delayed_damage[i];
			if(battle_enemy_data[i].HP<=0)
				voc(get_monster(i).death_sfx).play();
			else{
				battle_enemy_data[i].pos_x=battle_enemy_data[i].pos_x_bak;
				battle_enemy_data[i].pos_y=battle_enemy_data[i].pos_y_bak;
			}
		}
}
void battle::show_enemy_changes(int times)
{
	int pixels=8;
	for(int i=1;i<=times;i++){
		for(int j=0;j<=1;j++){
			for(int k=0;k<enemy_poses_count;k++)
				if(twoside_affected[k]){
					battle_enemy_data[k].pos_x-=pixels;
					battle_enemy_data[k].pos_y-=pixels/2;
					if(i==1)
						affected_enemies[k]=true;
				}
			draw_battle_scene(0,1);
			pixels=-pixels;
		}
		pixels/=2;
	}
}
bool battle::check_enemy_changes(){
	bool changed=false;
	for(int i=0;i<enemy_poses_count;i++){
		if(store_for_diff.enemies[i].HP-=battle_enemy_data[i].HP){
			display_damage_number(1,store_for_diff.enemies[i].HP,battle_enemy_data[i].pos_x,battle_enemy_data[i].pos_y-0x6E);
			changed=true;
			affected_enemies[i]=true;
		}
	}
	return changed;
}
void battle::auto_attack(){
	auto_selected_enemy = select_an_enemy_randomly();
	for(int i=commanding_role;i<=rpg.team_roles;i++){
		role_action_table[i].action=AUTO_ATTACK;
		role_action_table[i].target=auto_selected_enemy;//not instantly?
	}
	flag_autobattle=1;
	flag_selecting=false;
}
void battle::use_or_throw(int itemuse_select,int &item_select,bool &refresh){
	int filter=0;
	if(itemuse_select==0)
		filter=1;
	else
		filter=4;
	int item=menu_item(item_select,filter);
	draw_battle_scene_selecting();
	if(item_select==-1){
		refresh=true;
		return;
	}
	bool need_refresh=false;
	switch(itemuse_select){
	case -1:
		need_refresh=true;
		break;
	case 0:
		{
			int target;
			if( (rpg.objects[item].item.param>>4) &1)
				target=-1;
			else
				target=select_targetting_role();
			if(target<0){
				need_refresh=true;
				break;
			}
			role_action_table[commanding_role].action=USE_ITEM;
			role_action_table[commanding_role].target=target;
		}
		break;
	case 1:
		{
			int target;
			if( (rpg.objects[item].item.param>>4) &1)
				target=commanding_role;
			else
				target=select_targetting_enemy();
			if(target<0){
				need_refresh=true;
				break;
			}
			role_action_table[commanding_role].action=THROW_ITEM;
			role_action_table[commanding_role].target=target;
		}
		break;
	}
	if(need_refresh==true){
		refresh=true;
		return;
	}
	if(item>=0){
		role_action_table[commanding_role].toolpos=item_select;
		role_action_table[commanding_role].tool=item;
		rpg.items[item_select].using_amount++;
	}else{
		refresh=true;
	}
}
void battle::escape(){
	for(int r=commanding_role;r<=rpg.team_roles;r++)
		role_action_table[r].action=ESCAPE;
}
void battle::status(){
	role_status();
	draw_battle_scene_selecting();
}
battle::END battle::check_end_battle(){
	need_battle=false;
	flag_battling=false;
	switch(battle_result){
		case ROLE_FAIL:
			setup_role_status();
			draw_battle_scene(0,1);
			break;
		case ENEMY_FAIL:
			if(enemy_money || enemy_exps){
				musicplayer->play(script_escape?3:2);
				//display_money();
				//display_exps();
			for(int i=0;i<enemy_poses_count;i++)
			{
				uint16_t &postbattle_script=battle_enemy_data[i].script.script.after;
				postbattle_script=process_script(postbattle_script,i);
			}
			//升级等
			}
		default:
			break;
	}
	return battle_result;
}
battle::~battle()
{
	memset(battle_enemy_data,0,sizeof(battle_enemy_data));
	memset(battle_role_data,0,sizeof(battle_role_data));
	memset(enemy_poison_stack,0,sizeof(enemy_poison_stack));
	memset(rpg.poison_stack,0,sizeof(rpg.poison_stack));
	memset(enemy_status_pack,0,sizeof(enemy_status_pack));
	memset(role_status_pack,0,sizeof(role_status_pack));
	flag_autobattle=0;
	thebattle=NULL;
	flag_to_load|=3;
}
enum STATUS{ NON_NORMAL,WEAK,NORMAL };
STATUS role_status_determine(int role_pos)
{
	if(role_status_pack[role_pos].pack.crazy || role_status_pack[role_pos].pack.fixed || role_status_pack[role_pos].pack.sleep || rpg.roles_properties.HP[rpg.team[role_pos].role]<=0)
		return NON_NORMAL;
	if(battle_role_data[role_pos].frame==1)
		return WEAK;
	else
		return NORMAL;
}
void battle::enemy_physical_attack(int enemy_pos,int role_pos,int force)
{
	int role_defence=get_cons_attrib(rpg.team[role_pos].role,0x13),sfx=-1;
	int final_damage=0;
	int who_care;
	if(battle_role_data[role_pos].frame==3)
		role_defence*=2;
	int modulated_who_care=-round(rnd1(0.85));
	if(role_status_determine(role_pos)<=1)
		if(modulated_who_care==-1){
			modulated_who_care=1;
			int rescurer=-1;
			for(int i=0;i<=rpg.team_roles;i++)
				if(rpg.roles_properties.rescuer[rpg.team[role_pos].role]==rpg.team[i].role)
					rescurer=i;
			if(rescurer>=0 && role_status_determine(rescurer)>1)
				modulated_who_care=-(rescurer+10);
		}else
			modulated_who_care=1;
	voc(get_monster(enemy_pos).attack_sfx).play();
	if(get_monster(enemy_pos).magic_frames>0){
		enemy_moving_semaphor=false;
		for(int i=1;i<=get_monster(enemy_pos).magic_frames;i++)
			battle_enemy_data[enemy_pos].frame=get_monster(enemy_pos).stand_frames+i-1;
		draw_battle_scene(0,2);
	}
	for(int i=1,max=3-get_monster(enemy_pos).magic_frames;i<=max;i++)
	{
		battle_enemy_data[enemy_pos].pos_x -= 2;
		battle_enemy_data[enemy_pos].pos_y -= 1; //!!!look at game
		draw_battle_scene(0,1);
	}
	voc(get_monster(enemy_pos).action_sfx).play();
	draw_battle_scene(0,1);
	enemy_moving_semaphor=false;
	battle_enemy_data[enemy_pos].pos_x=battle_role_data[role_pos].pos_x-44;
	battle_enemy_data[enemy_pos].pos_y=battle_role_data[role_pos].pos_y-16;
	sfx=get_monster(enemy_pos).yahoo_sfx;
	if(modulated_who_care==-1){
		battle_role_data[role_pos].frame=3;
		sfx=rpg.roles_properties.sfx_block[rpg.team[role_pos].role];
	}
	if(modulated_who_care<=-10){
		who_care=abs(modulated_who_care+10);
		battle_role_data[who_care].frame=3;
		sfx=rpg.roles_properties.sfx_block[rpg.team[who_care].role];
		battle_role_data[who_care].pos_x = battle_role_data[role_pos].pos_x-0x18;
		battle_role_data[who_care].pos_y = battle_role_data[role_pos].pos_y-0xC;
	}
	if(get_monster(enemy_pos).attack_frames==0){
		battle_enemy_data[enemy_pos].frame=get_monster(enemy_pos).stand_frames-1;
		draw_battle_scene(0,3);
	}else{
		for(int i=0,max=get_monster(enemy_pos).attack_frames;i<=max;i++){
			battle_enemy_data[enemy_pos].frame=get_monster(enemy_pos).stand_frames+get_monster(enemy_pos).magic_frames+i-1;
			draw_battle_scene(0,get_monster(enemy_pos).draw_times);
		}
	}
	if(modulated_who_care==0)
		battle_role_data[role_pos].frame=4;
	if(modulated_who_care>=0){
		affected_roles[role_pos]=true;
		final_damage=calc_base_damage(role_defence,force+round(rnd0()))*(1+rnd1(1.0/8))+round(rnd0());
		if(role_status_pack[role_pos].pack.high_defence)
			final_damage/=2;
		if(rpg.roles_properties.HP[rpg.team[role_pos].role] < final_damage)
			final_damage=rpg.roles_properties.HP[rpg.team[role_pos].role];
		rpg.roles_properties.HP[rpg.team[role_pos].role]-=final_damage;
		display_damage_number(1,final_damage,battle_role_data[role_pos].pos_x,battle_role_data[role_pos].pos_y-0x46);
	}
	voc(sfx).play();
	draw_battle_scene(0,1);
	if(modulated_who_care<=-10){
		battle_enemy_data[enemy_pos].pos_x -= 10;
		battle_enemy_data[enemy_pos].pos_y -= 8;
		battle_role_data[who_care].pos_x += 4;
		battle_role_data[who_care].pos_y += 2;
	}else{
		battle_role_data[role_pos].pos_x += 8;
		battle_role_data[role_pos].pos_y += 4;
	}
	draw_battle_scene(0,1);
	if(rpg.roles_properties.HP[rpg.team[role_pos].role]<=0)
		voc(rpg.roles_properties.sfx_death[rpg.team[role_pos].role]).play();
	battle_role_data[role_pos].pos_x += 2;
	battle_role_data[role_pos].pos_y += 1;
	draw_battle_scene(0,1);
	battle_enemy_data[enemy_pos].pos_x = battle_enemy_data[enemy_pos].pos_x_bak;
	battle_enemy_data[enemy_pos].pos_y = battle_enemy_data[enemy_pos].pos_y_bak;
	battle_enemy_data[enemy_pos].frame = 0;
	draw_battle_scene(0,1);
	battle_role_data[role_pos].frame = battle_role_data[role_pos].frame_bak;
	draw_battle_scene(0,1);
	enemy_moving_semaphor=true;
	int poison_defence=get_cons_attrib(rpg.team[role_pos].role,0x16);
	if(modulated_who_care>=0
		&& get_monster(enemy_pos).attack_equ_freq > rnd1(10)
		&& rnd1(100)>poison_defence){
			uint16_t &poison_script=rpg.objects[get_monster(enemy_pos).attack_equ_item].item.use;
			process_script(poison_script,role_pos);
	}
}
void battle::enemy_magical_attack(int ori_force,int magic_id,int role_pos,int enemy_pos)
{
	MAGIC &magic=magics[rpg.objects[magic_id].magic.magic];
	enemy_moving_semaphor=false;
	battle_enemy_data[enemy_pos].pos_x += 0xC;
	battle_enemy_data[enemy_pos].pos_y += 6;
	draw_battle_scene(0,1);
	battle_enemy_data[enemy_pos].pos_x += 4;
	battle_enemy_data[enemy_pos].pos_y += 2;
	draw_battle_scene(0,1);
	enemy_fire_magic(enemy_pos);
	load_theurgy_image(rpg.objects[magic_id].magic.magic);
	if(!flag_second_attacking)
		delay(20);
	int magic_begin=get_monster(enemy_pos).stand_frames+get_monster(enemy_pos).magic_frames-1,magic_end=magic_begin+get_monster(enemy_pos).attack_frames;
	if(magic.delay==0)
		for(int i=magic_begin;i<=magic_end;i++)
		{
			battle_enemy_data[enemy_pos].frame=i;
			draw_battle_scene(0,get_monster(enemy_pos).draw_times);
		}
	switch(magic.behavior)
	{
	case 3://全屏
		magic_image_occurs=1;
		x[1]=magic.x_offset+0xA0;
		y[1]=magic.y_offset+0xC8;
		s[1]=magic.summon_effect;
		break;
	case 2://一方
		magic_image_occurs=1;
		x[1]=magic.x_offset+0xF0;
		y[1]=magic.y_offset+0x96;
		s[1]=magic.summon_effect;
		break;
	case 1://三叠
		magic_image_occurs=3;
		x[1]=0xB4;
		y[1]=0xB4;
		x[2]=0xEA;
		y[2]=0xAA;
		x[3]=0x10E;
		y[3]=0x92;
		for(int i=1;i<=magic_image_occurs;i++)
			x[i]+=magic.x_offset,
			y[i]+=magic.y_offset,
			s[i]=magic.summon_effect;
		break;
	case 0://定点
		magic_image_occurs=1;
		x[1]=battle_role_data[role_pos].pos_x+magic.x_offset;
		y[1]=battle_role_data[role_pos].pos_y+magic.y_offset;
		s[1]=magic.summon_effect;
		break;
	default:
		break;
	}
	uint16_t &prev_script=rpg.objects[magic_id].magic.occur;
	process_script(prev_script,role_pos);
	if(magic.delay>0){
		battle_enemy_data[enemy_pos].frame=magic_begin;
		for(int i=0;i<magic.delay;i++)
			magic_frame=i,
			draw_battle_scene(magic.speed,1);
		for(int i=magic_begin;i<=magic_end;i++){
			battle_enemy_data[enemy_pos].frame=i;
			draw_battle_scene(0,get_monster(enemy_pos).draw_times);
		}
	}
	int role_exist[5];int defend_multiple[5];
	memset(role_exist,0,sizeof(role_exist));
	memset(defend_multiple,0,sizeof(defend_multiple));
	int rnd=0;
	if(magic.behavior>0){
		for(int r=0;r<=rpg.team_roles;r++)
			if(rpg.roles_properties.HP[rpg.team[r].role]>0){
				role_exist[r]=0;
				defend_multiple[r]=1;
				if(battle_role_data[r].frame==3)
					defend_multiple[r]=2;
				if(role_status_pack[r].pack.high_defence>0)
					defend_multiple[r]*=2;
				role_exist[r]=-1;
				rnd=-round(rnd1(0.75));
				if(role_status_determine(r)<2)
					rnd=1;
				if(rnd!=-1)
					continue;
				battle_role_data[r].frame=3;
				defend_multiple[r]++;
			}
	}else{
		role_exist[role_pos]=-1;
		defend_multiple[role_pos]=1;
		if(battle_role_data[role_pos].frame==3)
			defend_multiple[role_pos]=2;
		if(role_status_pack[role_pos].pack.high_defence>0)
			defend_multiple[role_pos]*=2;
		rnd=-round(rnd1(0.75));
		if(battle_role_data[role_pos].frame==1 || (role_status_pack[role_pos].pack.fixed>0 || role_status_pack[role_pos].pack.sleep>0))
			rnd=1;
		if(rnd==-1){
			battle_role_data[role_pos].frame=3;
			defend_multiple[role_pos]++;
		}
	}
	magic_fire(magic.delay,rpg.objects[magic_id].magic.magic);
	magic_image_occurs=0;
	for(int i=0;i<=rpg.team_roles;i++){
		int force = ori_force+round(rnd1(4));
		if(role_exist[i]){
			int damage = magic.base_damage+calc_base_damage(get_cons_attrib(i,0x13),force)/2;
			damage/=defend_multiple[i];
			switch(int elem=magic.elem_attr){
					case 6:
						elem=0;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						damage=damage*(100-get_cons_attrib(i,elem+0x16))/100;
						if(elem)
							damage=damage*(10+Pal::battlefields[rpg.battlefield].elem_property[elem])/10;
						break;
					default:
						break;
			}
			if(rpg.roles_properties.HP[rpg.team[i].role] < damage)
				damage=rpg.roles_properties.HP[rpg.team[i].role];
			if(damage<0)
				damage=0;
			rpg.roles_properties.HP[rpg.team[i].role] -= damage;
			if(rpg.roles_properties.HP[rpg.team[i].role] <= 0)
				voc(rpg.roles_properties.sfx_death[rpg.team[i].role]).play();
			display_damage_number(1,damage,battle_role_data[i].pos_x,battle_role_data[i].pos_y-0x46);
		}
	}
	for(int i=0,gap=16;i<5;i++){
		for(int r=0;r<=rpg.team_roles;r++)
			if(role_exist[r]){
				if(defend_multiple[r]<4){
					battle_role_data[r].frame=4;
					if(i>0){
						battle_role_data[r].pos_x+=gap;
						battle_role_data[r].pos_y+=gap/2;
					}
				}
				if(i<3){
					affected_roles[r]=true;
				}
			}
		gap>>=1;
		draw_battle_scene(0,1);
	}
	battle_enemy_data[enemy_pos].frame=0;
	battle_enemy_data[enemy_pos].pos_x = battle_enemy_data[enemy_pos].pos_x_bak;
	battle_enemy_data[enemy_pos].pos_y = battle_enemy_data[enemy_pos].pos_y_bak;
	draw_battle_scene(0,1);
	for(int r=0;r<=rpg.team_roles;r++)
		if(role_exist[r])
			battle_role_data[r].frame=battle_role_data[r].frame_bak;
	enemy_moving_semaphor=true;
}
void battle::magic_fire(int frame,int magic_index)
{
	MAGIC &magic=magics[magic_index];
	voc(magic.sfx).play();
	magic_waving=magic.waving;
	int frames=FIRE.slices(magic.effect);
	for(int i=0;i<=magic.action_lasting;i++){
		voc::stop();
		voc(magic.sfx).play();
		magic_frame=frame;
		while(magic_frame<frames){
			draw_battle_scene(magic.speed,1);
			magic_frame++;
		}
	}
	magic_frame--;
	shake_viewport_y=16;
	for(int i=1;i<=magic.shaking;i++){
		draw_battle_scene(0,1);
		shake_viewport_y=10+round(rnd1(6));
	}
	shake_viewport_y=0;
	magic_waving=0;
	if(battlefield_waving<9 && magic.effect_lasting){
		add_occuring_magic_to_drawlist(true);
		backupBackground();
	}
}
void battle::role_release_magic_action(int role_pos,bool not_summon)
{
	battle_role_data[role_pos].frame=0;
	for(int i=1,j=4;i<=4;i++,j--){
		battle_role_data[role_pos].pos_x-=j;
		battle_role_data[role_pos].pos_y-=j/2;
		draw_battle_scene(0,1);
	}
	draw_battle_scene(0,2);
	battle_role_data[role_pos].frame=5;
	voc(rpg.roles_properties.sfx_discharge[rpg.team[role_pos].role]).play();
	if(not_summon)
		role_release_magic_effect(role_pos);
	draw_battle_scene(0,1);
}
void battle::role_release_magic_effect(int role_pos)
{
	x[1]=battle_role_data[role_pos].pos_x;
	y[1]=battle_role_data[role_pos].pos_y;
	s[1]=100;
	magic_img=boost::shared_ptr<sprite_prim>(new sprite_prim(discharge_effects));
	magic_image_occurs=1;
	magic_frame=effect_idx[battle_role_data[role_pos].battle_avatar].magic*10+14;
	for(int i=1;i<=10;i++){
		magic_frame++;
		draw_battle_scene(-1,1);
	}
	magic_image_occurs=0;
}
void battle::role_release_magic(int power,int magic,int target,int pos)
{
	bool is_summon=false;
	if(pos==9)
		is_summon=true;
	int magic_index=rpg.objects[magic].magic.magic;
	memset(twoside_affected,0,sizeof(twoside_affected));
	if(magics[magic_index].effect>=0){
		int base_damage=magics[magic_index].base_damage;
		if(pos<=rpg.team_roles){
			load_theurgy_image(magic_index);
			if(!magics[magic_index].delay)
				battle_role_data[pos].frame=6;
		}
		if(is_summon)
			magic_index=magics[magic_index].effect;
		enemy_moving_semaphor=false;
		if(magics[magic_index].behavior<4 && magics[magic_index].behavior>0)
			for(int i=0;i<enemy_poses_count;i++)
				if(battle_enemy_data[i].HP>0)
					twoside_affected[i]=-1;
		switch(magics[magic_index].behavior){
			case 7:
			case 3:
				magic_image_occurs=1;
				x[1]=magics[magic_index].x_offset+0xA0;
				y[1]=magics[magic_index].y_offset+0xC8;
				s[1]=magics[magic_index].summon_effect;
				break;
			case 6:
				magic_image_occurs=1;
				x[1]=magics[magic_index].x_offset+0xF0;
				y[1]=magics[magic_index].y_offset+0xA0;
				s[1]=magics[magic_index].summon_effect;
				break;
			case 5:
				magic_image_occurs=rpg.team_roles;
				for(int i=0;i<=rpg.team_roles;i++){
					++magic_image_occurs;
					x[magic_image_occurs]=battle_role_data[i].pos_x+magics[magic_index].x_offset;
					y[magic_image_occurs]=battle_role_data[i].pos_y+magics[magic_index].y_offset;
					s[magic_image_occurs]=magics[magic_index].summon_effect;
					twoside_affected[i]=-1;
				}
				break;
			case 4:
				magic_image_occurs=1;
				x[1]=battle_role_data[target].pos_x+magics[magic_index].x_offset;
				y[1]=battle_role_data[target].pos_y+magics[magic_index].y_offset;
				s[1]=magics[magic_index].summon_effect;
				twoside_affected[target]=-1;
				break;
			case 2:
				magic_image_occurs=1;
				x[1]=magics[magic_index].x_offset+0x78;
				y[1]=magics[magic_index].y_offset+0x64;
				s[1]=magics[magic_index].summon_effect;
				break;
			case 1:
				magic_image_occurs=3;
				x[1]=0x46;
				y[1]=0x8C;
				x[2]=0x64;
				y[2]=0x6E;
				x[3]=0xA0;
				y[3]=0x64;
				for(int i=1;i<=magic_image_occurs;i++){
					x[i]+=magics[magic_index].x_offset;
					y[i]+=magics[magic_index].y_offset;
					s[i]=magics[magic_index].summon_effect;
				}
				break;
			case 0:
				magic_image_occurs=1;
				x[1]=battle_enemy_data[target].pos_x+magics[magic_index].x_offset;
				y[1]=battle_enemy_data[target].pos_y+magics[magic_index].y_offset;
				s[1]=magics[magic_index].summon_effect;
				twoside_affected[target]=-1;
				break;
			default:
				break;
		}
		magic_frame=0;
		if(magics[magic_index].delay){
			for(magic_frame=0;magic_frame<magics[magic_index].delay;magic_frame++)
				draw_battle_scene(magics[magic_index].speed,1);
			battle_role_data[pos].frame=6;
		}
		magic_fire(magic_frame,magic_index);
		magic_image_occurs=0;
		max_blow_away=0;
		if(magics[magic_index].behavior<=3)
			for(int i=0;i<enemy_poses_count;i++){
				int damage=power+power*rnd1(0.1);
				if(!twoside_affected[i])
					continue;
				int defence=enemy_level_scaler(i,4)+get_monster(i).defence;
				if(twoside_affected[i]){
					damage=calc_base_damage(defence,damage)/2+magics[magic_index].base_damage;
					int def_elem=0;
					switch(int elem=magics[magic_index].elem_attr){
						case STORM:
						case LIGHT:
						case FLOOD:
						case FLAME:
						case EARTH:
							def_elem=get_monster(i).elem_defences[elem-1];
							break;
						case POISON:
							def_elem=get_monster(i).poison_defence;
							break;
						default:
							break;
					}
					if(def_elem){
						damage=damage*(10-def_elem)/5;
						damage=damage*(10+Pal::battlefields[Pal::rpg.battlefield].elem_property[def_elem])/10;
					}
				}
				if(damage)
					delayed_damage[i]=damage;
			}
		else{
			draw_battle_scene(-4,1);
			if(magics[magic_index].behavior<=7)
				for(int i=2;i<=16;i++){
					delay(4);
					for(int r=0;r<=rpg.team_roles;r++)
						if(twoside_affected[r])
							team_images[r].getsprite(battle_role_data[r].frame)->blit_filter(screen,battle_role_data[r].pos_x,battle_role_data[r].pos_y,brighter_filter,16-i,true,true);
				}
		}
		enemy_moving_semaphor=true;
		stage_blow_away=0;
	}
}

void battle::add_occuring_magic_to_drawlist(bool flag)
{
	for(int i=1;i<=magic_image_occurs;i++)
		if(flag)
			magic_img->getsprite(magic_frame)->blit_middlebottom(battlescene,x[i],y[i]-shake_viewport_y);
		else
			sprites.push(boost::shared_ptr<sprite>(magic_img->getsprite(magic_frame)->clone()->setXYL(x[i],y[i]+s[i]-shake_viewport_y,s[i])));
}
void battle::load_theurgy_image(int id){
	magic_img=boost::shared_ptr<sprite_prim>(new sprite_prim(FIRE,Pal::magics[id].effect));
}
void battle::summon_imgs(int summon)
{
	MAGIC &magic=magics[rpg.objects[summon].magic.magic];
	Font->blit_to(objs(summon),screen,0xC8,0x32,0xE,true);
	enemy_moving_semaphor=false;
	bright_every_role(0,rpg.team_roles);
	magic_img=boost::shared_ptr<sprite_prim>(new sprite_prim(F,magic.summon_effect+10));
}
void battle::enemy_fire_magic(int enemy_pos)
{
	voc(get_monster(enemy_pos).magic_sfx).play();
	for(int i=get_monster(enemy_pos).stand_frames,max=i+get_monster(enemy_pos).magic_frames;i<max;i++){
		battle_enemy_data[enemy_pos].frame=i;
		draw_battle_scene(0,get_monster(enemy_pos).draw_times);
	}
	if(get_monster(enemy_pos).magic_frames==0)
		draw_battle_scene(0,1);
}
void battle::enemy_attack_role(int enemy_pos,int role_pos){
	flag_attacking_hero=true;
	int use_magic,rnd_1=0;
	if(enemy_status_pack[enemy_pos].pack.crazy){
		int enemy_target=select_an_enemy_randomly();
		if(enemy_target==enemy_pos){
			flag_attacking_hero=false;
			return;
		}
		enemy_crazy_attack_enemy(enemy_pos,enemy_target);
	}
	else{
		uint16_t &battle_script=battle_enemy_data[enemy_pos].script.script.when;
		battle_script=process_script(battle_script,enemy_pos);
		if( rnd1(10) <= get_monster(enemy_pos).magic_freq )
			use_magic=get_monster(enemy_pos).magic;
		else
			use_magic=0;
		if(use_magic<=0 || enemy_status_pack[enemy_pos].pack.seal)
			use_magic=0,
			rnd_1=rnd0();
		if(use_magic<0 || rnd_1)
			return;
		if(use_magic==0){
			//physical
			int force=get_monster(enemy_pos).force+enemy_level_scaler(enemy_pos,6);
			enemy_physical_attack(enemy_pos,role_pos,force);
		}
		else{
			//magic
			int force=get_monster(enemy_pos).power+enemy_level_scaler(enemy_pos,6);
			enemy_magical_attack(force,use_magic,role_pos,enemy_pos);
			uint16_t &post_script=rpg.objects[use_magic].magic.post;
			process_script(post_script,role_pos);
		}
		int role_need_help=-1,role_help=-1;
		for(int i=0;i<=rpg.team_roles;i++){
			int role=rpg.team[i].role;
			if(battle_role_data[i].frame_bak!=0 || rpg.roles_properties.HP[role]<=0)
				continue;
			if(rpg.roles_properties.HP[role]<50 || rpg.roles_properties.HP[role]<=rpg.roles_properties.HP_max[role]/5)
				voc(rpg.roles_properties.sfx_suffer[role]).play();
			if(rpg.roles_properties.HP[role]<10)
				if(rpg.roles_properties.HP[rpg.roles_properties.rescuer[role]]>0 && round(rnd0()))
					role_need_help=i;
		}
		setup_role_status();
		for(int i=1,offset=10;i<=5;i++){
			for(int r=0;r<=rpg.team_roles;r++)
				if(role_action_table[i].alive==-1){
					battle_role_data[role_pos].pos_x += offset;
					battle_role_data[role_pos].pos_y += offset/2;
					for(int r2=0;r2<rpg.team_roles;r2++)
						if(rpg.team[r2].role==rpg.roles_properties.rescuer[rpg.team[r].role] && round(rnd0()))
							role_help=r2;
				}
			draw_battle_scene(-1,1);
			offset/=2;
		}
		if(rpg.roles_properties.HP[rpg.team[role_help].role]>0 && role_help>=0){//wu,why not detect the dead of need_help? Somewhere I havn't seen?
			uint16_t &otherdead_script=rpg.objects[rpg.roles_properties.name[rpg.team[role_help].role]].role.other_dead;
			process_script(otherdead_script,role_help);
		}else if(role_need_help>=0){
			uint16_t &selfhurt_script=rpg.objects[rpg.roles_properties.name[rpg.team[role_need_help].role]].role.self_hurt;
			process_script(selfhurt_script,role_need_help);
		}
		flag_attacking_hero=false;
		return;
	}
}
void battle::role_physical_attack(int role_pos,int enemy_pos,int &damage,int bouts){
	int role=rpg.team[role_pos].role;
	int offset=(enemy_pos>2?(enemy_pos-role_pos)*8:0);
	int high_attack_flag=0,final_sfx=0;
	magic_image_occurs=0;
	if(bouts==0){
		battle_role_data[role_pos].frame=7;
		draw_battle_scene(0,4);
	}
	damage+=1+round(rnd0());
	if(round(rnd1(6))==3 || role_status_pack[role_pos].pack.high_attack){
		damage*=3;
		high_attack_flag=2;
		final_sfx=rpg.roles_properties.sfx_heavyattack[role];
	}else{
		high_attack_flag=1;
		final_sfx=rpg.roles_properties.sfx_attack[role];
	}
	damage=damage*(1+rnd1(1.0/8));
	if(role==LEE && round(rnd1(12))==3){
		damage*=2;
		high_attack_flag=2;
		final_sfx=rpg.roles_properties.sfx_heavyattack[role];
	}
	voc(final_sfx).play();
	battle_role_data[role_pos].frame=8;
	battle_role_data[role_pos].pos_x=battle_enemy_data[enemy_pos].pos_x-offset+0x40;
	battle_role_data[role_pos].pos_y=battle_enemy_data[enemy_pos].pos_y+offset+0x14;
	draw_battle_scene(0,2);
	battle_role_data[role_pos].unknown=4;
	battle_role_data[role_pos].pos_x-=0xA;
	battle_role_data[role_pos].pos_y-=2;
	draw_battle_scene(0,1);
	battle_role_data[role_pos].frame=9;
	battle_role_data[role_pos].pos_x-=0x10;
	battle_role_data[role_pos].pos_y-=4;
	magic_image_occurs=high_attack_flag;
	magic_img=boost::shared_ptr<sprite_prim>(new sprite_prim(discharge_effects));
	magic_frame=effect_idx[battle_role_data[role_pos].battle_avatar].attack*3;
	for(int i=1,x=battle_enemy_data[enemy_pos].pos_x,y=battle_enemy_data[enemy_pos].pos_y-battle_enemy_data[enemy_pos].length/3+10;i<=magic_image_occurs;x-=16,y+=16,i++)
	{
		::x[i]=x;
		::y[i]=y;
		s[i]=50;
	}
	voc(rpg.roles_properties.sfx_weapon[role]).play();
	draw_battle_scene(0,1);
	magic_frame++;
	display_damage_number(1,damage,battle_enemy_data[enemy_pos].pos_x,battle_enemy_data[enemy_pos].pos_y-0x6E);
	affected_enemies[enemy_pos]=true;
	draw_battle_scene(0,1);
	magic_frame++;
	battle_role_data[role_pos].pos_x+=2;
	battle_role_data[role_pos].pos_y++;
	draw_battle_scene(0,1);
	magic_image_occurs=0;
	for(int i=0,offset=8;i<=2;i++){
		battle_enemy_data[enemy_pos].pos_x-=offset;
		battle_enemy_data[enemy_pos].pos_y-=offset/=2;
		offset=-offset;
		draw_battle_scene(0,1);
	}
}
void battle::enemy_crazy_attack_enemy(int from,int target)
{}
void battle::role_crazy_attack_team(int from,int target)
{}
battle::END process_Battle(uint16_t enemy_team,uint16_t script_escape)
{
	return battle(enemy_team,script_escape).process();
}

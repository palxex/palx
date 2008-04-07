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

union _role_status role_status_pack[TEAMROLES],enemy_status_pack[TEAMENEMIES];
_battle_role_data battle_role_data[TEAMROLES];
_battle_enemy_data battle_enemy_data[TEAMENEMIES];
bool flag_autobattle=false;

bool instrum_usable[4];//指令可用扩展;从而封攻、令成为可能

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
		if(role_status_pack[i].pack.dummy)
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
				return i;
	int trend=0;
	targetting_enemy=0;
	while(running){
		switch(async_getkey())
		{
		case PAL_VK_LEFT:
			trend=-1;
			break;
		case PAL_VK_RIGHT:
			trend=1;
			break;
		case PAL_VK_MENU:
			return -1;
		case PAL_VK_EXPLORE:
			return targetting_enemy;
		default:
			trend=0;
			break;
		}
		do
			targetting_enemy=std::min(std::max(targetting_enemy+trend,0),enemy_poses_count-1);
		while(battle_enemy_data[targetting_enemy].HP<=0);
		draw_battle_scene(0,1);
	}
	return targetting_enemy=-1;
}

void battle::battle_produce_screen(BITMAP *buf)
{
	battlescene.blit_to(buf);
	for(int i=enemy_poses_count-1;i>=0;i--)
		if(enemyteams[enemy_team].enemy[i]>0){
			enemy_images[i].getsprite(battle_enemy_data[i].frame)->blit_filter(buf,battle_enemy_data[i].pos_x,battle_enemy_data[i].pos_y,brighter_filter,6,affected_enemies[i],true);
		}
	//unimplemented summon tasks
	if(role_invisible_rounds==0)
		for(int i=Pal::rpg.team_roles;i>=0;i--)//应按posY排序保证不遮挡。实现到再说
			team_images[i].getsprite(battle_role_data[i].frame)->blit_middlebottom(buf,battle_role_data[i].pos_x,battle_role_data[i].pos_y);

}
void battle::draw_battle_scene(int delaytime,int times,BITMAP *bmp)
{
	if(times==0)
		times=1;
	stage_blow_away+=rnd1(max_blow_away);
	for(int t=1;t<=times;t++)
	{
		bitmap scanline(0,SCREEN_W,effective_y);
		if(battle_wave+magic_wave)
			wave_screen(battlescene,scanline,battle_wave+magic_wave,effective_y);
		else
			battlescene.blit_to(scanline);
		for(int e=enemy_poses_count-1;e>=0;e--)
		{
			int enemy=rpg.objects[enemyteams[enemy_team].enemy[e]].enemy.enemy;
			int frames=monsters[enemy].stand_frames;
			if(battle_enemy_data[e].HP<=0 && store_for_diff.enemies[e].HP<=0)
				continue;
			if(!frames)
				continue;
			if(enemy_moving_semaphor && frames<99 && !( enemy_status_pack[e].pack.fixed || enemy_status_pack[e].pack.sleep)	&& (drawlist_parity%frames))
				battle_enemy_data[e].frame=(battle_enemy_data[e].frame+1)%monsters[enemy].stand_frames;
			if(flag_selecting && (drawlist_parity&1) && (targetting_enemy == e))
				affected_enemies[e]=1;
			int crazybits=0;
			if(enemy_status_pack[e].pack.crazy)
				crazybits=rnd1(3);
			boost::shared_ptr<sprite> it=boost::shared_ptr<sprite>(enemy_images[e].getsprite(battle_enemy_data[e].frame)->clone());
			it->setXYL(battle_enemy_data[e].pos_x+crazybits+stage_blow_away,battle_enemy_data[e].pos_y+sth_about_y+stage_blow_away/2,0);
			sprites.push(it);
		}
		if(flag_summon)
			;//add_summon_img
		else
			for(int r=rpg.team_roles;r>=0;r--)
			{
				int crazybits=0;
				if(role_status_pack[r].pack.crazy)
					crazybits=rnd1(3);
				if(role_invisible_rounds==0){
					boost::shared_ptr<sprite> it=boost::shared_ptr<sprite>(team_images[r].getsprite(battle_role_data[r].frame)->clone());
					it->setXYL(battle_role_data[r].pos_x+crazybits,battle_role_data[r].pos_y,0);
					sprites.push(it);
				}
				if(flag_selecting)
				{
					if(targetting_role == r)
						sprites.push(boost::shared_ptr<sprite>(UIpics.getsprite(66+(drawlist_parity&1))->clone()->setXYL(battle_role_data[r].pos_x-4,battle_role_data[r].pos_y,0x3D)));
					if(commanding_role == r)
						sprites.push(boost::shared_ptr<sprite>(UIpics.getsprite(68+(drawlist_parity&1))->clone()->setXYL(battle_role_data[r].pos_x-4,battle_role_data[r].pos_y,0x44)));
				}
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
battle::battle(int team,int script):enemy_team(team),script_escape(script),stage_blow_away(0),magic_wave(0),battle_wave(battlefields[Pal::rpg.battlefield].waving),endbattle_method(0),battle_result(0),escape_flag(0),
									max_blow_away(0),flag_withdraw(false),effect_height(200),battle_scene_draw(false),flag_high_attack(false),flag_summon(false),flag_selecting(false),
									role_invisible_rounds(0),enemy_poses_count(0),enemy_exps(0),enemy_money(0),need_battle(true),drawlist_parity(0),sth_about_y(0),effective_y(200)
{
	memset(&store_for_diff,0,sizeof(store_for_diff));
	memset(affected_enemies,0,sizeof(affected_enemies));
	memset(affected_roles,0,sizeof(affected_roles));

	//确保我方没有开战即死
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]<=0)
			rpg.roles_properties.HP[rpg.team[i].role]=1;

	setup_our_team_data_things();

	scene->scenemap.change(0);
	scene->scenemap.change(Pal::scenes[Pal::rpg.scene_id].id);
	flag_battling=true;
	sprites.clear_active();

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

		//role command loop
		for(commanding_role=0;commanding_role<=rpg.team_roles && running;)
		{
			enemy_moving_semaphor=true;
			flag_high_attack=false;
			if(role_status_pack[commanding_role].pack.dummy){
				role_attack_table[commanding_role].target=select_an_enemy_randomly();
				role_attack_table[commanding_role].action=ATTACK;
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
				if(role_attack_table[i].action==USE_ITEM || role_attack_table[i].action==THROW_ITEM){
					if(rpg.items[role_attack_table[i].target].using_amount)
						rpg.items[role_attack_table[i].target].using_amount--;
					role_attack_table[i].action=ATTACK;//？那R……
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


			bool ok=false;
			switch(bout_selecting(instrum_selected))
			{
			case -1:
				if(commanding_role>0)
					commanding_role--;
				else
					check_end_battle();
				break;
			case 0:
				ok=true;
				if(rpg.roles_properties.attack_all[rpg.team[commanding_role].role])
					role_attack_table[commanding_role].action=ATTACK_ALL;
				else if((targetting_enemy=select_targetting_enemy())>=0)
					role_attack_table[commanding_role].action=ATTACK;
				else
					ok=false;
				break;
			case 1:
				show_money(rpg.coins,10,7,0x15,false);
				if((magic_select=select_theurgy(rpg.team[commanding_role].role,role_status_pack[commanding_role].pack.seal?0:2,magic_select,false))>=0)
					ok=true;
				draw_battle_scene_selecting();
				break;
			case 2:
				break;
			case 3:
				switch(menu(4,0x10,5,0x38,2)(single_menu(),0))
				{
				case 1:
					switch(itemuse_select=menu(0x18,0x32,2,0x17,2)(single_menu(),itemuse_select))
					{
					draw_battle_scene_selecting();
					case 0:
						if((item_select=menu_item(item_select,1))>=0)
							ok=true;
						break;
					case 1:
						if((item_select=menu_item(item_select,4))>=0)
							ok=true;
						break;
					}
					draw_battle_scene_selecting();
					break;
				case 4:
					role_status();
					break;
				default:
					break;
				}
				break;
			case PAL_VK_REPEAT+100:
				break;
			case PAL_VK_AUTO+100:
				break;
			case PAL_VK_DEFEND+100:
				break;
			case PAL_VK_USE+100:
				break;
			case PAL_VK_THROW+100:
				break;
			case PAL_VK_QUIT+100:
				break;
			case PAL_VK_STATUS+100:
				break;
			case PAL_VK_FORCE+100:
				break;
			default:
				if(role_attack_table[commanding_role].action==ESCAPE)
					commanding_role=rpg.team_roles;
				instrum_selected=0;instrum_object=0;
				draw_battle_scene_selecting();
				break;
			}
			if(!need_battle)
				break;
			if(ok)
				commanding_role++;
			perframe_proc();
		}
		flag_selecting=false;
		effective_y=200;
		draw_battle_scene(0,1);

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

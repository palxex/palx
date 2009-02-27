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
#ifndef BATTLE_H
#define BATTLE_H

#include "scene.h"
#include "structs.h"

#define TEAMENEMIES 5

extern union _role_status{
	int list[16];
	struct {
		int crazy;
		int fixed;
		int sleep;
		int seal;
		int dummy;
		int high_attack;
		int high_defence;
		int high_speed;
		int twice_attack;
	}pack;
}role_status_pack[TEAMROLES],enemy_status_pack[TEAMENEMIES];
extern struct _battle_role_data{
	int battle_avatar;
	int pos_x,pos_y;
	int unknown;
	int pos_x_bak,pos_y_bak;
	int frame,frame_bak;
	int offset,offset_bak;
	int prev_frame;
	int face_color;
	int contract_magic;
}battle_role_data[TEAMROLES];
extern struct _battle_enemy_data{
	int battle_avatar;
	int pos_x,pos_y;
	int pos_x_bak,pos_y_bak;
	int frame,frame_bak;
	int offset;
	int length;
	int HP,prev_HP;
	int id;
#pragma pack(2)
	union{
		uint16_t scripts[3];
		struct{
			uint16_t before,when,after;
		}script;
	}script;
#pragma pack()
}battle_enemy_data[TEAMENEMIES];
extern RPG::POISON_DEF enemy_poison_stack[16][TEAMENEMIES];
extern int flag_autobattle;
enum MAGIC_PROP{ON_SCENE,ON_BATTLE,UNKNOWN,TARGET_ENEMY,OBJECT_ALL};

void role_restore_position(int role_pos);

class battle{
	enum ACTION{
		ATTACK=0,
		MAGIC_TO_US,
		MAGIC_TO_ENEMY,
		USE_ITEM,
		THROW_ITEM,
		DEFENCE,
		AUTO_ATTACK,
		COSTAR,
		ESCAPE,
		CRAZY_ATTACK,
		ATTACK_ALL
	};
	struct _number{
		int x,y;
		int num;
		int color;
		int times;
	}battle_numbers[12];

	struct {
		struct {
			int HP,MP;
		}roles[TEAMROLES];
		struct {
			int HP;
		}enemies[TEAMENEMIES];
	}store_for_diff;

	std::multimap<int,int,std::greater<int> > vs_table;

	static battle *thebattle;

	bitmap battlescene,battlebuf;
	int enemy_team;
	std::map<int,sprite_prim> team_images;
	std::map<int,sprite_prim> enemy_images;
	std::map<int,sprite_prim> magic_images;

	int stage_blow_away;

	void setup_role_enemy_image();
	void setup_role_status();
	void summon_imgs(int summon);

	void display_damage_number(int color,int num,int x,int y);
public:
	struct _role_attack{
		int target;
		ACTION action;
		int tool;
		int toolpos;
		int alive;
	}role_action_table[8],bak_action_table[8];

	bool affected_enemies[TEAMENEMIES],affected_roles[TEAMROLES];
	MONSTER enemy_data[5];
	
	MONSTER &get_monster(int pos);
	void load_enemy_pos();
	int enemy_level_scaler(int enemy,int scaler);
	int calc_final_damage(double A,int enemy,int magic);

	int script_escape;
	enum END{
		QUIT=-1,
		NOT=0,
		ROLE_FAIL,
		ROLE_ESCAPE,
		ENEMY_FAIL,
	};
	sprite_queue sprites;
	int magic_waving,battlefield_waving;
	END endbattle_method,battle_result,escape_flag;
	static battle *get(){
		if(thebattle)
			return thebattle;
		else
			return NULL;
	}

	int max_blow_away;

	battle(int team,int script);
	~battle();
	END process();
	END check_end_battle();
	void backupBackground(){
		battlescene.blit_to(battlebuf);
	}
	void restoreBackground(){
		battlebuf.blit_to(battlescene);
	}

	void load_enemy(int enemy_pos,int enemy_id);
	void battle_produce_screen(BITMAP *buf);
	void draw_battle_scene(int delay,int times,BITMAP * =screen);
	void draw_battle_scene_selecting();
	void add_occuring_magic_to_drawlist(bool flag);

	int bout_selecting(int &selected);

	int get_member_alive();
	int get_enemy_alive();

	int select_targetting_enemy();
	int select_targetting_role();
	int select_a_living_role_randomly();
	int select_an_enemy_randomly();
	void bright_every_role(int begin,int end);

	void enemy_attack_role(int,int);
	void enemy_physical_attack(int enemy_pos,int role_pos,int force);
	void enemy_magical_attack(int force,int magic,int role_pos,int enemy_pos);
	void enemy_fire_magic(int enemy_pos);
	void role_physical_attack(int role_pos,int enemy_pos,int &damage,int bouts);
	void role_release_magic_action(int role_pos,bool not_summon);
	void role_release_magic_effect(int role_pos);
	void role_release_magic(int power,int magic,int target,int pos);
	void magic_fire(int delay,int magic_id);
	bool check_role_changes();
	bool check_enemy_changes();
	void show_role_changes(int action_taker,int magic);
	void show_enemy_changes(int times);
	void attack_make();

	void enemy_crazy_attack_enemy(int from,int target);
	void role_crazy_attack_team(int from,int target);

	void auto_attack();
	void use_or_throw(int use,int &select,bool& refresh);
	void escape();
	void status();

	int flag_invisible;
	int role_invisible_rounds;
	int action_taker;
	bool flag_attacking_hero;
	int enemy_poses_count;
	void load_theurgy_image(int id);
	int enemy_exps,enemy_money;
private:
	int commanding_role;
	bool flag_withdraw;
	int effect_height;
	bool battle_scene_draw;
	int magic_image_occurs;
	bool flag_summon;
	bool flag_selecting;
	bool need_battle;
	bool flag_repeat;
	bool enemy_moving_semaphor;
	int targetting_role,targetting_enemy;
	int drawlist_parity;
	int sth_about_y;
	int effective_y;
	bool flag_second_attacking;
	int auto_selected_enemy;
	int battle_sfx;
	int magic_frame;
	int shake_viewport_y;
	boost::shared_ptr<sprite_prim> magic_img;
	int drew_scenes_summon,summon_img_x,summon_img_y;
};
battle::END process_Battle(uint16_t enemy_team,uint16_t script_escape);
#endif //BATTLE_H

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

extern union _role_status{
	int list[10];
	struct {
		int crazy;
		int fixed;
		int sleep;
		int seal;
		int dumn;
		int high_attack;
		int high_defence;
		int high_speed;
		int twice_attack;
	}pack;
}role_status_pack[TEAMROLES];
extern struct _battle_role_data{
	int battle_avatar;
	int pos_x,pos_y;
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
	int HP;
	int id;
#pragma pack(2)
	union{
		uint16_t scripts[3];
		struct{
			uint16_t before,when,after;
		}script;
	}script;
#pragma pack()
}battle_enemy_data[TEAMROLES];

extern bool flag_autobattle;

class battle{
	struct _role_attack{
		int target;
		int action;
		int tool;
		int toolpos;
		int alive;
	}role_attack_table[8];

	struct _number{
		int color;
		int num;
		int x,y;
		bool exist;
	}battle_numbers[12];

	int enemy_HP_r[5];

	int affected_enemies[TEAMROLES],affected_roles[TEAMROLES];

	static battle *thebattle;

	bitmap battlescene,battlebuf;
	int enemy_team,script_escape;
	std::map<int,sprite_prim> team_images;
	std::map<int,sprite_prim> enemy_images;
	std::map<int,sprite_prim> magic_images;

	int stage_blow_away;

	void setup_role_enemy_image();
	void setup_role_status();
	void draw(int delay,int time);
public:
	int magic_wave,battle_wave;
	int endbattle_method,battle_result,escape_flag;
	static battle *get(){
		if(thebattle)
			return thebattle;
		else
			return 0;
	}

	static int max_blow_away;

	battle(int team,int script);
	~battle();
	int process();
	void check_end_battle();

	void load_enemy(int enemy_pos,int enemy_id);
	void battle_produce_screen(BITMAP *buf);
	void draw_battle_scene();
	void draw_battle_scene_selecting();

	int bout_selecting();

	int get_member_alive();
	int get_enemy_alive();

	int select_enemy();
private:
	bool flag_withdraw;
	int effect_height;
	bool battle_scene_draw;
	bool flag_high_attack;
	bool flag_summon;
	bool flag_selecting;
	int role_invisible_rounds;
	int enemy_poses_count;
	int enemy_exps,enemy_money;
	bool need_battle;
	int role_counter;
	bool flag_repeat;
};
int process_Battle(uint16_t enemy_team,uint16_t script_escape);
#endif //BATTLE_H

/***************************************************************************
 *   Copyright (C) 2006 by pal lockheart   *
 *   palxex@Pal.Lockheart   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SSS_STRUCTS_H
#define SSS_STRUCTS_H

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

#pragma pack(2)
typedef struct event_object{
	int16_t vanish_time;
	int16_t pos_x;
	int16_t pos_y;
	int16_t	layer;
	uint16_t	trigger_script;
	uint16_t	auto_script;
	int16_t status;
	int16_t	trigger_method;
	int16_t	image;
	int16_t	frames;
	int16_t	direction;
	int16_t	curr_frame;
	int16_t	scr_jmp_count;
	int16_t	image_ptr_offset;
	int16_t	frames_auto;
	int16_t	scr_jmp_count_auto;
}EVENT_OBJECT;

typedef struct scene_def{
	int16_t id;
	uint16_t enter_script;
	uint16_t leave_script;
	int16_t prev_evtobjs;
}SCENE;

const int ALLROLES=6,TEAMROLES=5;

typedef struct object_def{
	int16_t inbeing;
	int16_t attrib;
	uint16_t script[3];
	int16_t param;
}OBJECT;

typedef struct script_def{
	int16_t func;
	int16_t param[3];
}SCRIPT;

//typedef struct 

enum DIRECTION{WEST,NORTH,EAST,SOUTH};
enum ROLE{LEE,LINGR,YUERU,WUHOU,GAI,ANU};

typedef int16_t roles[ALLROLES];
typedef struct {
	roles icon;
	roles avator;
}ROLES_PROP;

typedef struct rpg_def{
	int16_t save_times;
	int16_t viewport_x,viewport_y;
	int16_t team_roles;
	int16_t scene_id;
	int16_t palette_id;
	int16_t team_direction;
	int16_t music;
	int16_t battle_music;
	int16_t battlefield;
	int16_t wave_grade;
	int16_t reserved;
	int16_t gourd_value;
	int16_t layer;//?
	int16_t chase_speed;
	int16_t no_chase_cycles;
	int16_t team_followers;
	int16_t reserved_[3];
	int32_t coins;
	struct position{
		int16_t role;
		int16_t x,y;
		int16_t direction;
		int16_t img_handler;
	}team[TEAMROLES];
	struct track{
		int16_t x,y;
		int16_t direction;
	}team_track[TEAMROLES];
	struct _exp{
		int32_t exp;
		int16_t level;
		int16_t count;
	}roles_exp[8][ALLROLES];
	ROLES_PROP roles_properties;
	struct {
		int16_t poison;
		int16_t script;
	}poison_stack[16][TEAMROLES];
	struct {
		int16_t item;
		int16_t amount;
		int16_t using_amount;
	}items[0x100];
	SCENE	scenes[300];
	OBJECT	objects[600];
	EVENT_OBJECT	evtobjs[5332];
}RPG;

typedef struct{ int16_t item[9]; } SHOP;

typedef struct {
	int16_t stand_frames;
	int16_t magic_frames;
	int16_t attack_frames;
	int16_t unknown1;
	int16_t unknown2;
	int16_t pos_y_offset;
	int16_t attack_sfx;//?
	int16_t unknown3;
	int16_t magic_sfx;
	int16_t death_sfx;
	int16_t yahoo_sfx;
	int16_t hp;
	int16_t exp;
	int16_t coins;
	int16_t level;
	int16_t magic;
	int16_t magic_freq;
	int16_t attack_equ_item;
	int16_t attack_equ_freq;
	int16_t steal_item;
	int16_t steal_amount;
	int16_t force;
	int16_t power;
	int16_t defence;
	int16_t speed;
	int16_t lucky;
	int16_t poison_defence;
	int16_t storm_defence;
	int16_t lignt_defence;
	int16_t flood_defence;
	int16_t flame_defence;
	int16_t earth_defence;
	int16_t weapon_defence;
	int16_t flag_twice_action;
	int16_t gourd_value;
}MONSTER;

typedef struct{ int16_t enemy[5];} ENEMYTEAM;

enum element_attr{WEAPON,STORM,LIGHT,FLOOD,FLAME,EARTH,POISON};
typedef struct {
	int16_t effect;
	int16_t behavior;
	int16_t x_offset;
	int16_t y_offset;
	int16_t summon_effect;//?
	int16_t speed;
	int16_t effect_lasting;
	int16_t delay;
	int16_t action_lasting;
	int16_t shaking;//?
	int16_t waving;//?
	int16_t unknown;
	int16_t power_used;
	int16_t base_damage;
	int16_t elem_attr;
	int16_t sfx;
}MAGIC;

typedef struct {
	int16_t waving;
	int16_t storm;
	int16_t light;
	int16_t flood;
	int16_t flame;
	int16_t earth;
}BATTLE_FIELD;

struct _LEARN{
	int16_t level;
	int16_t magic;
};
typedef struct{
	_LEARN learning[ALLROLES];
}UPGRADE_LEARN;

struct _POS{
	int16_t x,y;
};
typedef struct{
	_POS pos[TEAMROLES][TEAMROLES];
}ENEMY_POSES;

typedef struct{
	int16_t level;
	int16_t exp;
}UPGRADE_EXP;
#pragma pack()

#endif

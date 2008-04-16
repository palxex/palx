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
#ifndef CONSTANTS
#define CONSTANTS

#include "structs.h"
#include <vector>
#include <map>

struct Scene;
struct BattleScene;
class playrix;
class Game;
class global_init;
extern Scene *scene;
extern BattleScene *battle_scene;
extern global_init *global;

extern bool flag_battling;

extern int flag_to_load;
extern int rpg_to_load;
extern int map_toload;

extern int step_off_x,step_off_y;
extern int coordinate_x_max,coordinate_y_max;
extern int x_scrn_offset,y_scrn_offset;
extern int abstract_x_bak,abstract_y_bak;
extern int viewport_x_bak,viewport_y_bak;
extern int direction_offs[4][2];
extern bool key_enable;
extern int fadegap[6];

extern bool flag_parallel_mutex;
extern int redraw_flag;
extern int flag_pic_level;

extern int dialog_type;

extern int current_dialog_lines;
extern int glbvar_fontcolor;
extern int font_color_yellow;
extern int font_color_red;
extern int font_color_cyan;
extern int font_color_cyan_1;
extern int frame_pos_flag;
extern int dialog_x;
extern int dialog_y;
extern int frame_text_x;
extern int frame_text_y;
extern int CARD;
extern bool mask_use_CD;

class sprite_prim;
extern std::vector<sprite_prim> mgos;
extern std::map<int,int> team_mgos;
extern std::map<int,int> npc_mgos;

void randomize();
float rnd0();
int rnd1(double);

extern int scale;
extern int x_off,y_off;
extern bool running,is_out;

void Load_Data();
void GameLoop_OneCycle(bool);
bool process_Menu();
void process_Explore();
uint16_t process_script(uint16_t script,int16_t object);
uint16_t process_autoscript(uint16_t script,int16_t object);

void load_team_mgo();
void load_NPC_mgo();
void setup_our_team_data_things();
void record_step();
void calc_trace_frames();
void team_walk_one_step();
void NPC_walk_one_step(EVENT_OBJECT &obj,int speed);
void stop_and_update_frame();
void calc_followers_screen_pos();
int calc_faceto(int x_diff,int y_diff);
bool barrier_check(uint16_t self,int x,int y,bool =true);
extern bool no_barrier;

extern int CARD;
extern int mutex_switching;
void switch_proc();

extern int RNG_num;
void play_RNG(int begin,int end,int gap);

extern bool prelimit_OK;

#endif


#ifndef CONSTANTS
#define CONSTANTS

#include "integer.h"
#include "structs.h"
#include <vector>
#include <map>

struct Scene;
struct BattleScene;
class playrix;
class Game;
extern Scene *scene;
extern BattleScene *battle_scene;
extern playrix *rix;
extern Game *game;
extern RPG rpg;

extern bool flag_battling;

extern int flag_to_load;
extern int rpg_to_load;
extern int map_toload;

extern int step_off_x,step_off_y;
extern int coordinate_x_max,coordinate_y_max;
extern int x_scrn_offset,y_scrn_offset;
extern int abstract_x_bak,abstract_y_bak;
extern int viewport_x_bak,viewport_y_bak;
extern int direction_offs[][2];
extern bool key_enable;

extern bool flag_parallel_mutex;
extern int redraw_flag;
extern int flag_pic_level;

extern int dialog_type;

extern bool mutux_setpalette;

extern uint32_t current_dialog_lines;
extern uint32_t glbvar_fontcolor;
extern uint32_t font_color_yellow;
extern uint32_t font_color_red;
extern uint32_t font_color_cyan;
extern uint32_t font_color_cyan_1;
extern uint32_t frame_pos_flag;
extern uint32_t dialog_x;
extern uint32_t dialog_y;
extern uint32_t frame_text_x;
extern uint32_t frame_text_y;

class sprite_prim;
extern std::vector<sprite_prim> mgos;
extern std::map<int,int> team_mgos;
extern std::map<int,int> npc_mgos;

extern void randomize();
extern float rnd0();

extern void Load_Data();
extern void GameLoop_OneCycle(bool);
extern bool process_Menu();
extern void process_Explore();
extern uint16_t process_script(uint16_t script,int16_t object);
extern uint16_t process_autoscript(uint16_t script,int16_t object);

extern void load_team_mgo();
extern void load_NPC_mgo();
extern void redraw_everything(int gap=1);
extern void calc_trace_frames();
extern void team_walk_one_step();
extern void stop_and_update_frame();
extern void store_team_frame_data();
extern int calc_faceto(int x_diff,int y_diff);
extern bool barrier_check(uint16_t self,int x,int y);

extern bool mutex_can_change_palette;
extern void pal_fade_out(int gap);
extern void pal_fade_in(int gap);

extern int RNG_num;
extern void play_RNG(int begin,int end,int gap);
#endif


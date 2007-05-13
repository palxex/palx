#ifndef CONSTANTS
#define CONSTANTS

#include "config.h"
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

extern bool flag_battling;

extern int flag_to_load;
extern int rpg_to_load;
extern int step_off_x,step_off_y;
extern int coordinate_x_max,coordinate_y_max;
extern int x_scrn_offset,y_scrn_offset;
extern int abstract_x_bak,abstract_y_bak;
extern int viewport_x_bak,viewport_y_bak;
extern int direction_offs[][2];

extern bool flag_parallel_mutex;
extern int redraw_flag;

extern int dialog_type;

extern bool mutux_setpalette;
extern volatile int time_interrupt_occers; 

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

class sprite_prim;
extern std::vector<sprite_prim> mgos;
extern std::map<int,int> team_mgos;
extern std::map<int,int> npc_mgos;

extern void randomize();
extern float rnd0();

extern void Load_Data(int &flag);
extern void GameLoop_OneCycle(bool);
extern bool process_Menu();
extern void process_Explore();
extern uint16_t process_script(uint16_t script,int16_t object);
extern uint16_t process_autoscript(uint16_t script,int16_t object);

extern void load_team_mgo();
extern void redraw_everything(int gap=1);
extern void team_walk_one_step();
extern void stop_and_update_frame();

extern void show_wait_icon();
extern void dialog_firstline(char *);
extern void dialog_string(char *str,int);

#endif
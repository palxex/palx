#ifndef CONSTANTS
#define CONSTANTS

#include "config.h"
#include <list>
#include <vector>

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

extern bool mapchanged;

extern bool flag_parallel_mutex;
extern int redraw_flag;

extern bool mutux_setpalette;
extern volatile int time_interrupt_occers; 

class sprite_prim;
extern std::list<sprite_prim> mgos;
extern std::vector<sprite_prim *> team_prims;

extern void randomize();
extern float rnd0();

extern void Load_Data(int &flag);
extern void GameLoop_OneCycle(bool);
extern bool process_Menu();
extern void process_Explore();
extern uint16_t process_script(uint16_t script,int16_t object);
extern uint16_t process_autoscript(uint16_t script,int16_t object);

extern void load_team_mgo();
extern void redraw_everything();
extern void team_walk_one_step();
extern void stop_and_update_frame();

#endif
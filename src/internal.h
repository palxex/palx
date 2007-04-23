#ifndef CONSTANTS
#define CONSTANTS

#include "allegdef.h"
#include "scene.h"
#include "game.h"

extern Scene *scene;
extern Scene *battle_scene;
extern playrix *rix;
extern Game *game;

extern bool flag_battling;

extern int flag_to_load;
extern int step_off_x,step_off_y;
extern int coordinate_x_max,coordinate_y_max;
extern bool flag_parallel_mutex;

extern void randomize();
extern float rnd0();

extern void Load_Data(int flag);
extern void GameLoop_OneCycle(bool);
extern void process_scrn_drawing(int);
extern bool process_Menu();
extern void process_Explore();
extern void process_script(uint16_t &script,uint16_t object);
extern void process_autoscript(uint16_t &script,uint16_t object);

#endif
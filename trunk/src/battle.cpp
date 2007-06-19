#include "internal.h"
#include "game.h"
#include "timing.h"

std::map<int,sprite_prim> team_images;
std::map<int,sprite_prim> enemy_images;
std::map<int,sprite_prim> magic_images;

void setup_role_enemy_image()
{
}

void draw_battle_scene(int enemy_team)
{
	bitmap battlescene(FBP.decode(game->rpg.battlefield),SCREEN_W,SCREEN_H);
	
	for(int i=0;i<=game->rpg.team_roles;i++)
		team_images[i].getsprite(0)->blit_middle(battlescene.bmp,240,170);
	for(int i=0;i<1;i++)
		if(game->enemyteams[enemy_team].enemy[i]>0)
			enemy_images[i].getsprite(0)->blit_to(battlescene.bmp,game->enemyposes.pos[i][5].x,game->enemyposes.pos[i][5].y);

	battlescene.blit_to(screen,0,0,0,0);
}

int process_Battle(uint16_t enemy_team,uint16_t script_escape)
{
	rix->play(game->rpg.battle_music);
	flag_battling=true;
	for(int i=0;i<=game->rpg.team_roles;i++)
		team_images[i]=sprite_prim(F,game->rpg.roles_properties.battle_avator[game->rpg.team[i].role]);
	for(int i=0;i<5;i++)
		if(game->enemyteams[enemy_team].enemy[i]>0)
			enemy_images[i]=sprite_prim(ABC,game->rpg.objects[game->enemyteams[enemy_team].enemy[i]].inbeing);
	draw_battle_scene(enemy_team);while(!keypressed()) wait(10);
	rix->play(game->rpg.music);
	return 0;
}

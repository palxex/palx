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

std::map<int,sprite_prim> team_images;
std::map<int,sprite_prim> enemy_images;
std::map<int,sprite_prim> magic_images;

void setup_role_enemy_image()
{
}

void draw_battle_scene(int enemy_team)
{
	bitmap battlescene(FBP.decode(game->rpg.battlefield),SCREEN_W,SCREEN_H);
	perframe_proc();

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

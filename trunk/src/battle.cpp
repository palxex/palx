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
#include "battle.h"

using namespace res;

int battle::max_blow_away=0;
battle *battle::thebattle=NULL;

void battle::setup_role_enemy_image()
{
}

static struct{int x,y;} role_poses[4][4]={{{240,170}},{{200,176},{256,152}},{{180,180},{234,170},{270,146}},{{160,184},{204,175},{246,160},{278,144}}};

void battle::draw(int delay,int time)
{
	stage_blow_away+=rnd1(max_blow_away);

	for(int i=1;i<=time;i++)
	{
		bitmap fbp(FBP.decode(res::rpg.battlefield),320,200);
		bitmap battlescene(0,SCREEN_W,SCREEN_H);fbp.blit_to(battlescene);
		int enemies=4-std::count(res::enemyteams[enemy_team].enemy,res::enemyteams[enemy_team].enemy+5,0)-std::count(res::enemyteams[enemy_team].enemy,res::enemyteams[enemy_team].enemy+5,-1);

		for(int i=res::rpg.team_roles;i>=0;i--)
			team_images[i].getsprite(0)->blit_middle(battlescene,role_poses[res::rpg.team_roles][i].x,role_poses[res::rpg.team_roles][i].y-team_images[i].getsprite(0)->height/2);
		for(int i=4;i>=0;i--)
			if(res::enemyteams[enemy_team].enemy[i]>0)
				enemy_images[i].getsprite(0)->blit_to(battlescene,res::enemyposes.pos[i][enemies].x-enemy_images[i].getsprite(0)->width/2,res::enemyposes.pos[i][enemies].y-enemy_images[i].getsprite(0)->height);

		battlescene.blit_to(screen,0,0,0,0);
	}
}
battle::battle(int team,int script):enemy_team(team),script_escape(script),stage_blow_away(0),magic_wave(0)
{
	//确保我方没有开战即死
	for(int i=0;i<=rpg.team_roles;i++)
		if(rpg.roles_properties.HP[rpg.team[i].role]<=0)
			rpg.roles_properties.HP[rpg.team[i].role]=1;

	//清除上战物品使用记录
	for(int i=0;i<=99;i++);

	rix->play(res::rpg.battle_music);
	flag_battling=true;
	for(int i=0;i<=res::rpg.team_roles;i++)
		team_images[i]=sprite_prim(F,res::rpg.roles_properties.battle_avator[res::rpg.team[i].role]);
	for(int i=0;i<5;i++)
		if(res::enemyteams[enemy_team].enemy[i]>0)
			enemy_images[i]=sprite_prim(res::ABC,res::rpg.objects[res::enemyteams[enemy_team].enemy[i]].general.inbeing);

	thebattle=this;
}
battle::~battle()
{
	thebattle=NULL;
	flag_to_load|=3;
}
int battle::process()
{
	do{
		draw(0,1);
		wait(1);
	}while(running && sync_getkey()==PAL_VK_NONE);
	return 0;
}

int process_Battle(uint16_t enemy_team,uint16_t script_escape)
{
	return battle(enemy_team,script_escape).process();
}

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

using namespace res;


class battle{
	int enemy_team,script_escape;
	std::map<int,sprite_prim> team_images;
	std::map<int,sprite_prim> enemy_images;
	std::map<int,sprite_prim> magic_images;

	void setup_role_enemy_image()
	{
	}

	void draw()
	{
		static struct{int x,y;} role_poses[4][4]={{{240,170}},{{200,176},{256,152}},{{180,180},{234,170},{270,146}},{{160,184},{204,175},{246,160},{278,144}}};
		bitmap battlescene(FBP.decode(res::rpg.battlefield),SCREEN_W,SCREEN_H);
		perframe_proc();
		int enemies=4-std::count(res::enemyteams[enemy_team].enemy,res::enemyteams[enemy_team].enemy+5,0)-std::count(res::enemyteams[enemy_team].enemy,res::enemyteams[enemy_team].enemy+5,-1);

		for(int i=res::rpg.team_roles;i>=0;i--)
			team_images[i].getsprite(0)->blit_middle(battlescene,role_poses[res::rpg.team_roles][i].x,role_poses[res::rpg.team_roles][i].y-team_images[i].getsprite(0)->height/2);
		for(int i=4;i>=0;i--)
			if(res::enemyteams[enemy_team].enemy[i]>0)
				enemy_images[i].getsprite(0)->blit_to(battlescene,res::enemyposes.pos[i][enemies].x-enemy_images[i].getsprite(0)->width/2,res::enemyposes.pos[i][enemies].y-enemy_images[i].getsprite(0)->height);

		battlescene.blit_to(screen,0,0,0,0);
	}
public:
	battle(int team,int script):enemy_team(team),script_escape(script)
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
				enemy_images[i]=sprite_prim(ABC,res::rpg.objects[res::enemyteams[enemy_team].enemy[i]].inbeing);
	}
	~battle()
	{
		flag_to_load|=3;
	}
	int process()
	{
		VKEY keygot;
		do{
			draw();
			wait(10);
			SAFE_GETKEY(keygot);
		}while(keygot==VK_NONE);
		return 0;
	}
};

int process_Battle(uint16_t enemy_team,uint16_t script_escape)
{
	return battle(enemy_team,script_escape).process();
}

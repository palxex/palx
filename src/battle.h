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
#ifndef BATTLE_H
#define BATTLE_H
class battle{
	static battle *thebattle;

	int enemy_team,script_escape;
	std::map<int,sprite_prim> team_images;
	std::map<int,sprite_prim> enemy_images;
	std::map<int,sprite_prim> magic_images;

	int stage_blow_away;

	void setup_role_enemy_image();
	void draw(int delay,int time);
public:
	static battle *get(){
		if(thebattle)
			return thebattle;
		else
			throw;
	}

	static int max_blow_away;
	int magic_wave;

	battle(int team,int script);
	~battle();
	int process();
};
int process_Battle(uint16_t enemy_team,uint16_t script_escape);
#endif //BATTLE_H
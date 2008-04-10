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
#ifndef Game_H_
#define Game_H_

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>

#include "allegdef.h"

namespace Pal{
    extern SETUP_DEF setup;
	extern palette pat;
	extern RPG rpg;
	//data
	extern std::vector<SHOP> shops;
	extern std::vector<MONSTER> monsters;
	extern std::vector<ENEMYTEAM> enemyteams;
	extern std::vector<MAGIC> magics;
	extern std::vector<BATTLE_FIELD> battlefields;
	extern std::vector<UPGRADE_LEARN> learns;
	extern sprite_prim UIpics;
	extern sprite_prim discharge_effects;
	extern sprite_prim message_handles;
	extern ENEMY_POSES enemyposes;
	extern UPGRADE_EXP upgradexp;
	extern EFFECT_IDX effect_idx;

	//sss
	extern std::vector<EVENT_OBJECT> evtobjs;
	extern std::vector<SCENE>   scenes;
	extern std::vector<int32_t> msg_idxes;
	extern std::vector<SCRIPT>  scripts;

	void init_resource();
	void destroy_resource();
	bool load(int id=rpg_to_load);
	void save(int id);
	int run();
};

#endif //Game_H_


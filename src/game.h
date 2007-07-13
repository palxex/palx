/***************************************************************************
 *   Copyright (C) 2006 by Pal Lockheart   *
 *   palxex@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef Game_H_
#define Game_H_

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>

#include "allegdef.h"
#include "internal.h"

class Game{
public:
	palette pat;
	RPG &rpg;
	//data
	std::vector<SHOP> shops;
	std::vector<MONSTER> monsters;
	std::vector<ENEMYTEAM> enemyteams;
	std::vector<MAGIC> magics;
	std::vector<BATTLE_FIELD> battlefields;
	std::vector<UPGRADE_LEARN> learns;
	sprite_prim UIpics;
	sprite_prim discharge_effects;
	sprite_prim message_handles;
	ENEMY_POSES enemyposes;
	std::vector<UPGRADE_EXP> upgradexp;
	
	//sss
	std::vector<EVENT_OBJECT> evtobjs;
	std::vector<SCENE>   scenes;
	std::vector<int32_t> msg_idxes;
	std::vector<SCRIPT>  scripts;
	Game(int = 0);
	~Game();
	void load(int id=rpg_to_load);
	void save(int id);
	int run();
};

#endif //Game_H_


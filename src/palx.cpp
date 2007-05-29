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

#include "Game.h"
#include "begin.h"
#include "pallib.h"

#include <iostream>
#include <cstdlib>
using namespace std;

#include <boost/lexical_cast.hpp>

#include "internal.h"
#include "game.h"
#include "scene.h"
#include "allegdef.h"
int main(int argc, char *argv[])
{
	//allegro init
	allegro_init();
	install_timer();
	install_keyboard();
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,argc>=4?boost::lexical_cast<int>(argv[2]):320,argc>=4?boost::lexical_cast<int>(argv[3]):200,0,0);
	set_color_depth(8);

	randomize();
	Game  thegame(argc>=2? boost::lexical_cast<int>(argv[1]) : begin_scene()()); game=&thegame;
	Scene normal;	scene=&normal;
	BattleScene battle;	battle_scene=&battle;
	playrix player;				rix=&player;
	return thegame.run();
}
END_OF_MAIN();
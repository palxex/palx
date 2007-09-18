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
extern bool running;

#pragma warning(disable: 4819)
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "scene.h"
#include "game.h"
#include "pallib.h"
#include "begin.h"

#include <iostream>
#include <cstdlib>
using namespace std;


int CARD=0;
void close_button_handler(void)
{
	//if(!yesno_dialog()) return;
	running=false;
	remove_timer();
}
END_OF_FUNCTION(close_button_handler)

int main(int argc, char *argv[])
{
	//boost::program_options��ȡ����,boost::regex��ȡ�浵��
	CARD=(argc>=5?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED);

	//allegro init
	allegro_init();
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
	install_timer();
	install_keyboard();
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	set_gfx_mode(CARD,argc>=4?boost::lexical_cast<int>(argv[2]):320,argc>=4?boost::lexical_cast<int>(argv[3]):200,0,0);
	set_color_depth(8);

	randomize();
	playrix player;				rix=&player;

	char conv_buf[16]="0";
	if(argc>=2){
		boost::regex expression("(([[:alpha:]]:)(.+)\\Q\\\\E)?(\\d+)(.rpg|.RPG)?");
		boost::cmatch what;
		boost::regex_match(argv[1],what,expression);
		strncpy(conv_buf,what[4].first,what[4].second-what[4].first);
	}
	boost::shared_ptr<Game> thegame;
	try{
	    using namespace res;
		init_resource();
        //load save
        int save=boost::lexical_cast<int>(conv_buf);
        if(save!=0 || (save=begin_scene()()))
            rpg_to_load=save;
        else
            map_toload=1;
        Scene normal;	scene=&normal;
        load();
        run();
	}catch(exception *){
	}
    res::destroy_resource();
	return 0;
}
END_OF_MAIN();
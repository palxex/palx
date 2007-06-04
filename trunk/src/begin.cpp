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
#include "begin.h"
#include "UI.h"
#include "timing.h"
#include "internal.h"

int begin_scene::operator()(Game *game){
	::game=game;
	RNG_num=6;
	game->pat.read(3);
	game->pat.set(rpg.palette_offset);
	play_RNG(0,999,25);
	wait_key(180);
	pal_fade_out(1);

	//ÔÆ¹Èº×·å
	rix->play(5);

	game->pat.read(0);
	bitmap(FBP.decode(60),320,200).blit_to(screen,0,0,0,0);
	rix->play(4);
	pal_fade_in(1);
	bool ok=false;
	int save=0;
	BITMAP *cache=create_bitmap(SCREEN_W,SCREEN_H);
	do{
		keygot=VK_NONE;
		static int menu_selected=0;
		static bool changed=true;
		if(changed)
			for(int i=7;i<9;i++)
				ttfont(cut_msg_impl("word.dat")(i*10,i*10+10)).blit_to(screen,0x70,0x54+(i-7)*0x12,i-7==menu_selected?0xFA:0x4E,true);
		changed=false;
		get_key();
		switch(keygot){
			case VK_UP:
				changed=true;
				menu_selected++;
				break;
			case VK_DOWN:
				changed=true;
				menu_selected--;
				break;
			case VK_MENU:
				continue;
			case VK_EXPLORE:
				ttfont(cut_msg_impl("word.dat")((menu_selected+7)*10,(menu_selected+7)*10+10)).blit_to(screen,0x70,0x54+menu_selected*0x12,0x2B,true);
				blit(screen,cache,0,0,0,0,SCREEN_W,SCREEN_H);
				if(menu_selected==0){
					ok=true;
					save = 0;
				}else if(rpg_to_load=select_rpg(0,screen)+1){
					ok=true;
					save = rpg_to_load;
				}else{
					blit(cache,screen,0,0,0,0,SCREEN_W,SCREEN_H);
					changed=true;
					continue;
				}
				break;
		}
		menu_selected+=2;menu_selected%=2;
	}while(!ok);
	destroy_bitmap(cache);
	pal_fade_out(1);
	return save;
}



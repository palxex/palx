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
#include "begin.h"
#include "UI.h"
#include "timing.h"
#include "scene.h"
#include "fade.h"

#ifdef PAL_WIN95
#   define SPLASH 3
#   define REEL 2
#else
#   define SPLASH 0x26
#   define REEL 60
#endif

using namespace Pal;

void startup_splash()
{
	clear_keybuf();
	pat.read(1);
	bitmap cat(0,SCREEN_W,SCREEN_H*2);
	fbp(SPLASH).blit_to(cat,0,0,0,0);
	fbp(SPLASH+1).blit_to(cat,0,0,0,200);

	sprite_prim goose(MGO,0x49),title(MGO,0x47);
	uint16_t &title_height=((uint16_t *)MGO.decode(0x47))[3],max_height=title_height,temp_height=8;
	int poses[9][3];
	for(int i=0;i<9;i++){
		poses[i][0]=round(rnd1(260))+420;
		poses[i][1]=round(rnd1(80));
		poses[i][2]=round(rnd1(8));
	}
	musicplayer->play(5);

	PALETTE pal;
	get_palette(pal);
	memset(pal,0,0xF0*sizeof(RGB));
	set_palette(pal);

	bitmap scrn_buf(0,SCREEN_W,SCREEN_H*2);
	int prog_lines=200,prog_pale=0,prog_goose=0,begin_pale=40,add_pale=16;
	do{
		blit(cat,scrn_buf,0,std::max(prog_lines--,0),0,0,SCREEN_W,SCREEN_H);
		for(int i=0;i<9;i++)
			if(poses[i][0]-2*i>-40)
				goose.getsprite(poses[i][2]=(poses[i][2]+(prog_goose&1))%8)->blit_to(scrn_buf,poses[i][0]-=2,poses[i][1]+=(prog_lines>0 && prog_lines&1?1:0));
		prog_goose++;
		if(temp_height<max_height)
			temp_height++;
		title_height=temp_height;
		title.getsprite(0)->blit_to(scrn_buf,0xFE,10);
		blit(scrn_buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
		delay(10);
		prog_pale=begin_pale/10;
		begin_pale+=add_pale;
		if(add_pale>=3)
			add_pale--;
		if(prog_pale<=0x40){
			for(int i=0;i<0xF0;i++){
				pal[i].r=pat.get(0)[i].r*prog_pale/0x40;
				pal[i].g=pat.get(0)[i].g*prog_pale/0x40;
				pal[i].b=pat.get(0)[i].b*prog_pale/0x40;
			}
			set_palette(pal);
			perframe_proc();
		}
	}while(running && async_getkey()==PAL_VK_NONE);
	title_height=max_height;
	title.getsprite(0)->blit_to(screen,0xFE,10);
	if(prog_pale<0x40){
		for(int i=prog_pale;i<0x40;i++){
			for(int j=0;j<0xF0;j++){
				pal[j].r=pat.get(0)[j].r*i/0x40;
				pal[j].g=pat.get(0)[j].g*i/0x40;
				pal[j].b=pat.get(0)[j].b*i/0x40;
			}
			set_palette(pal);
			delay(1);
		}
		wait_key(90);
	}
	mutex_can_change_palette=false;
	pal_fade_out(2);
}

int select_scene()
{
	pat.read(0);
	show_fbp(REEL,0);
	musicplayer->play(4);
	pal_fade_in(1);
	bool ok=false;
	int save=0;
	BITMAP *cache=create_bitmap(SCREEN_W,SCREEN_H);
	do{
		static int menu_selected=0;
		static bool changed=true;
		if(changed)
			for(int i=7;i<9;i++)
				Font->blit_to(objs(i),screen,0x7D,0x60+(i-7)*0x12,i-7==menu_selected?0xFA:0x4E,true);
		changed=false;
		switch(sync_getkey()){
			case PAL_VK_UP:
				changed=true;
				menu_selected++;
				break;
			case PAL_VK_DOWN:
				changed=true;
				menu_selected--;
				break;
			case PAL_VK_MENU:
				continue;
			case PAL_VK_EXPLORE:
				Font->blit_to(objs(menu_selected+7),screen,0x7D,0x60+menu_selected*0x12,0x2B,true);
				blit(screen,cache,0,0,0,0,SCREEN_W,SCREEN_H);
				if(menu_selected==0){
					ok=true;
					save = 0;
				}else if(rpg_to_load=select_rpg(0,screen)){
					ok=true;
					save = rpg_to_load;
				}else{
					blit(cache,screen,0,0,0,0,SCREEN_W,SCREEN_H);
					changed=true;
					continue;
				}
				break;
            default:
                break;
		}
		menu_selected+=2;menu_selected%=2;
	}while(running && !ok);
	destroy_bitmap(cache);
	pal_fade_out(1);
	return save;
}

int begin_scene::operator()(){
	RNG_num=6;
	pat.read(3);
	pat.set(rpg.palette_offset);
	play_RNG(0,999,25);
	wait_key(180);
	pal_fade_out(1);

	//ÔÆ¹Èº×·å
	startup_splash();

	MGO.clear();
	FBP.clear();
	RNG.clear();
	rpg.chase_range=1;
	return select_scene();
}



/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL   *
 *   Copyleft (C) 2006 by Pal Lockheart   *
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
#include "allegdef.h"
#include "internal.h"
#include "game.h"
#include "timing.h"

bool mutex_can_change_palette=false;
void pal_fade_out(int t)
{
	if(!mutex_can_change_palette)
	{
		mutex_can_change_palette=true;
		PALETTE pal;
		if(t)
			for(int i=0x3f;i>=3;i--)
			{
				perframe_proc();
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=game->pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=game->pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=game->pat.get(rpg.palette_offset)[j].b*i/0x40;
				}
				set_palette(pal);
				delay(t*10);
			}
		memset(&pal,0,sizeof(PALETTE));
		set_palette(pal);
	}
}
void pal_fade_in(int t)
{
	clear_keybuf();
	if(mutex_can_change_palette)
	{
		mutex_can_change_palette=false;
		PALETTE pal;
		if(t)
			for(int i=4;i<=0x3f;i++)
			{
				perframe_proc();
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=game->pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=game->pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=game->pat.get(rpg.palette_offset)[j].b*i/0x40;
				}
				set_palette(pal);
				delay(t*10);
			}
			set_palette(game->pat.get(rpg.palette_offset));
	}
}
uint8_t normalize(uint8_t i)
{
	int a=i%0x10,b=i/0x10;
	if(b>a)
		b--;
	else if(b<a)
		b++;
	return b*0x10+a;
}
void normalize_fade()
{
	PALETTE pal;
	for(int i=0;i<0x100;i++)
	{
		pal[i].r=normalize(game->pat.get(rpg.palette_offset)[i].r);
		pal[i].g=normalize(game->pat.get(rpg.palette_offset)[i].g);
		pal[i].b=normalize(game->pat.get(rpg.palette_offset)[i].b);
	}
	set_palette(pal);
}
void CrossFadeOut()
{
	perframe_proc();
}
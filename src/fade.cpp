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
#include "allegdef.h"
#include "internal.h"
#include "game.h"
#include "timing.h"

using namespace res;

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
					pal[j].r=res::pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=res::pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=res::pat.get(rpg.palette_offset)[j].b*i/0x40;
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
					pal[j].r=res::pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=res::pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=res::pat.get(rpg.palette_offset)[j].b*i/0x40;
				}
				set_palette(pal);
				delay(t*10);
			}
			set_palette(res::pat.get(rpg.palette_offset));
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
		pal[i].r=normalize(res::pat.get(rpg.palette_offset)[i].r);
		pal[i].g=normalize(res::pat.get(rpg.palette_offset)[i].g);
		pal[i].b=normalize(res::pat.get(rpg.palette_offset)[i].b);
	}
	set_palette(pal);
}
int fadegap[6]={0,3,1,5,2,4};
void crossFade_assimilate(int gap,int time,bitmap &dst,bitmap &jitter)
{
	uint8_t *d=(uint8_t*)(((BITMAP*)dst)->dat)+gap, *s=(uint8_t*)(((BITMAP*)jitter)->dat)+gap;
	do
		*d=((*s)&0x0F)|((*s)&0xF0);
	while(time-- && (d+=6) && (s+=6) && d<(uint8_t*)(((BITMAP*)dst)->dat)+((BITMAP*)dst)->w*((BITMAP*)dst)->h & s<(uint8_t*)(((BITMAP*)jitter)->dat)+((BITMAP*)jitter)->w*((BITMAP*)jitter)->h);
}
void crossFade_desault(int gap,int time,bitmap &dst,bitmap &jitter)
{
	uint8_t *d=(uint8_t*)(((BITMAP*)dst)->dat)+gap, *s=(uint8_t*)(((BITMAP*)jitter)->dat)+gap;
	do
		*d=((*s)>(*d)? (*d)+1 : ((*s)<(*d)? (*d)-1 : (*d)));
	while(time-- && (d+=6) && (s+=6) && d<(uint8_t*)(((BITMAP*)dst)->dat)+((BITMAP*)dst)->w*((BITMAP*)dst)->h & s<(uint8_t*)(((BITMAP*)jitter)->dat)+((BITMAP*)jitter)->w*((BITMAP*)jitter)->h);
}
void CrossFadeOut(int u,int times,int gap,bitmap &buf)
{
	bitmap dst(NULL,SCREEN_W,SCREEN_H);
	blit(screen,dst,0,0,0,0,SCREEN_W,SCREEN_H);
	for(int i=0;i<times;i++)
	{
		perframe_proc();
		int arg=i%6;
		if(i<6)
			crossFade_assimilate(fadegap[arg],u,buf,dst);
		else
			crossFade_desault(fadegap[arg],u,buf,dst);
		blit(buf,screen,0,0,0,0,SCREEN_W,SCREEN_H);
		delay(gap);
	}
	buf.blit_to(screen,0,0,0,0);
}
void crossFade_F(int gap,int time,bitmap &dst,bitmap &jitter)
{}

void palette_fade()
{/*
	PALETTE pal;
	pal[0].r=res::pat.get(rpg.palette_offset)[0].r;
	pal[0].g=res::pat
	for(int i=1;i<0x100;i++)
		pal[i].r=res::pat.get(rpg.palette_offset)[i].r>res::*/
}

void show_fbp(int pic,int gap)
{
	bitmap buf(NULL,320,200);
	clear_bitmap(buf);
	if(pic>0)
		bitmap(FBP.decode(pic),320,200).blit_to(buf,0,0,0,0);
	if(gap)
		;//CrossFadeOut(0x29B0,0x5F,gap,buf);
	buf.blit_to(screen,0,0,0,0);
}
int shake_times=0,shake_grade=0;
void ShakeScreen()
{
    if(--shake_times<0x10)
        blit(screen,screen,0,shake_grade=shake_grade*15/16,0,0,SCREEN_W,SCREEN_H);
}

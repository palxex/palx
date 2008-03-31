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
#include "scene.h"

using namespace Pal;

bool mutex_can_change_palette=true;
void pal_fade_out(int t)
{
	if(!mutex_can_change_palette)
	{
		mutex_can_change_palette=true;
		PALETTE pal;
		if(t>0)
			for(int i=0x3f;i>=3;i--)
			{
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=Pal::pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=Pal::pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=Pal::pat.get(rpg.palette_offset)[j].b*i/0x40;
				}
				set_palette(pal);
				perframe_proc();
				delay(t);
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
		if(t>0)
			for(int i=4;i<=0x3f;i++)
			{
				for(int j=0;j<0x100;j++)
				{
					pal[j].r=Pal::pat.get(rpg.palette_offset)[j].r*i/0x40;
					pal[j].g=Pal::pat.get(rpg.palette_offset)[j].g*i/0x40;
					pal[j].b=Pal::pat.get(rpg.palette_offset)[j].b*i/0x40;
				}
				set_palette(pal);
				perframe_proc();
				delay(t);
			}
		set_palette(Pal::pat.get(rpg.palette_offset));
	}
}
void fade_inout(int t)
{
	int arg=t?t:1;
	clear_keybuf();
	mutex_can_change_palette=false;

	PALETTE pal;
	int begin=(arg>0?0:0x40),end=(arg>0?0x40:0);
	for(int i=begin;i!=end;i+=arg)
	{
		for(int j=0;j<0x100;j++)
		{
			pal[j].r=Pal::pat.get(rpg.palette_offset)[j].r*i/0x40;
			pal[j].g=Pal::pat.get(rpg.palette_offset)[j].g*i/0x40;
			pal[j].b=Pal::pat.get(rpg.palette_offset)[j].b*i/0x40;
		}
		set_palette(pal);
		perframe_proc();
		GameLoop_OneCycle(false);
		redraw_everything(0);
	}
	mutex_can_change_palette=(arg<0);
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
		pal[i].r=normalize(Pal::pat.get(rpg.palette_offset)[i].r);
		pal[i].g=normalize(Pal::pat.get(rpg.palette_offset)[i].g);
		pal[i].b=normalize(Pal::pat.get(rpg.palette_offset)[i].b);
	}
	set_palette(pal);
}
int fadegap[6]={0,3,1,5,2,4};
void crossFade_assimilate(int gap,int time,bitmap &src,bitmap &dst)
{
	BITMAP *srcbmp(src),*dstbmp(dst);
	uint8_t *dstbegin=(uint8_t*)(dstbmp->dat),*srcbegin=(uint8_t*)(srcbmp->dat);
	uint8_t *srcptr=srcbegin+gap, *dstptr=dstbegin+gap;
	do
		*srcptr=((*srcptr)&0x0F)|((*dstptr)&0xF0);
	while(time-- && (srcptr+=6) && (dstptr+=6) && srcptr<srcbegin+srcbmp->w*srcbmp->h && dstptr<dstbegin+dstbmp->w*dstbmp->h);
}
void crossFade_desault(int gap,int time,bitmap &src,bitmap &dst)
{
	BITMAP *srcbmp(src),*dstbmp(dst);
	uint8_t *dstbegin=(uint8_t*)(dstbmp->dat),*srcbegin=(uint8_t*)(srcbmp->dat);
	uint8_t *srcptr=srcbegin+gap, *dstptr=dstbegin+gap;
	do
		*srcptr=((*dstptr)>(*srcptr)? (*srcptr)+1 : ((*dstptr)<(*srcptr)? (*srcptr)-1 : (*srcptr)));
	while(time-- && (srcptr+=6) && (dstptr+=6) && srcptr<srcbegin+srcbmp->w*srcbmp->h && dstptr<dstbegin+dstbmp->w*dstbmp->h);
}
void crossFade_self(int gap,bitmap &src)
{
	static int i=0;
	int time=65536/6;
	bitmap myscreen(screen);
	BITMAP *srcbmp(src),*dstbmp(myscreen);
	uint8_t *dstbegin=(uint8_t*)(dstbmp->dat),*srcbegin=(uint8_t*)(srcbmp->dat);
	uint8_t *srcptr=srcbegin+gap, *dstptr=dstbegin+gap;
	do
		*dstptr=*srcptr;
	while(time-- && (srcptr+=6) && (dstptr+=6) && srcptr<srcbegin+srcbmp->w*srcbmp->h && dstptr<dstbegin+dstbmp->w*dstbmp->h);
	myscreen.blit_to(screen);
    perframe_proc();
}
void CrossFadeOut(int u,int times,int gap,const bitmap &_src)
{
    bitmap &dst(const_cast<bitmap &>(_src));
	bitmap src(NULL,SCREEN_W,SCREEN_H);
	blit(screen,src,0,0,0,0,SCREEN_W,SCREEN_H);
	for(int i=0;i<times;i++)
	{
		int arg=i%6;
		if(i<6)
			crossFade_assimilate(fadegap[arg],u,src,dst);
		else
			crossFade_desault(fadegap[arg],u,src,dst);
		crossFade_self(fadegap[arg],src);
		delay(gap);
	}
	dst.blit_to(screen,0,0,0,0);
}

void palette_fade(PALETTE &src,const PALETTE &dst)
{
	uint8_t *usrc=(uint8_t*)src,*udst=(uint8_t*)dst;
	for(size_t i=0;i<sizeof(src);i++)
		if(udst[i]<usrc[i])
			usrc[i]--;
		else if(udst[i]>usrc[i])
			usrc[i]++;
	set_palette(src);
}

void show_fbp(int pic,int gap)
{
	bitmap buf(NULL,320,200);
	clear_bitmap(buf);
	if(pic>0)
		fbp(pic).blit_to(buf,0,0,0,0);
	if(gap)
		CrossFadeOut(0x29B0,0x5F,gap,buf);
	buf.blit_to(screen,0,0,0,0);
}
int shake_times=0,shake_grade=0;
void shake_screen()
{
#undef screen
	if(shake_times>0){
		if(--shake_times<0x10)
			shake_grade=(int)(shake_grade*(15.0/16));
	}else
        shake_grade=0;
    flush_screen();
}
void flush_screen()
{
	static bitmap fakebmp(NULL,SCREEN_W,SCREEN_H);
#undef screen
    //vsync();
    if(is_out) return;
    mutex_blitting=true;
    blit(fakescreen,fakebmp,0,(shake_times&1)*shake_grade,0,0,SCREEN_W,SCREEN_H);
	//int sy=(shake_times&1)*shake_grade;
    stretch_blit(fakebmp,screen,0,0,fakescreen->w,fakescreen->h,0,0,screen->w,screen->h);
    mutex_blitting=false;
}
struct calc_waving
{
	int result[32];
	calc_waving(int grade)
	{
		int ac=0,st=60+8,stt=8;
		for(int i=0;i<16;i++)
			result[i]=(ac+=(st-=stt))*grade/256,
			result[i+16]=-result[i];
	}
};
void wave_screen(bitmap &buffer,bitmap &dst,int grade,int height)
{
	static int index=0;
	calc_waving calc(grade);
	blit(buffer,dst,0,0,0,0,SCREEN_W,SCREEN_H);
	for(int i=0,t=index;i<height*scale;i++,t=(t+1)%32)
		if(calc.result[t]>=0){
			blit(buffer,dst,0,i,calc.result[t],i,SCREEN_W-calc.result[t]+1,1);
			blit(buffer,dst,SCREEN_W-calc.result[t]+1,i,0,i+1,calc.result[t],1);
		}else{
			blit(buffer,dst,-calc.result[t],i,0,i,SCREEN_W+calc.result[t]+1,1);
			blit(buffer,dst,0,i,SCREEN_W+calc.result[t]+1,i-1,-calc.result[t],1);
		}
	index=(index+1)%32;
}

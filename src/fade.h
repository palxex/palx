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
#ifndef _FADE_H
#define _FADE_H

class bitmap;
extern bool mutex_can_change_palette;
extern int mutex_paletting,mutex_blitting;
extern int shake_times,shake_grade;
extern void pal_fade_out(int gap);
extern void pal_fade_in(int gap);
extern void fade_inout(int);
extern int wave_progression;
void crossFade_assimilate(int gap,int time,bitmap &dst,bitmap &jitter);
void crossFade_desault(int gap,int time,bitmap &dst,bitmap &jitter);
void CrossFadeOut(int u,int times,int gap,const bitmap &buf);
void crossFade_self(int gap,bitmap &src);
void serials_of_fade(int thurgy);
void show_fbp(int,int);
void shake_screen();
void flush_screen();
void wave_screen(bitmap &buffer,bitmap &dst,int grade,int height,int y=0);
void clear_effective(int16_t p1,int16_t p2);

void perframe_proc();

#endif //_FADE_H

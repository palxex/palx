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
#include "resource.h"
#include "pallib.h"
#include "timing.h"
#include "internal.h"

int RNG_num;
void play_RNG(int begin,int end,int gap)
{
	decoder_func olddecoder=RNG.setdecoder(de_mkf);
	int total_clips=((uint32_t *)RNG.decode(RNG_num))[0]/4-2;
	RNG.setdecoder(olddecoder);
	BITMAP *cache=create_bitmap(320,200);
	blit(screen,cache,0,0,0,0,cache->w,cache->h);
	for(int i=begin;i<=std::min(total_clips-1,end);i++){
		perframe_proc();
		Pal::Tools::DecodeRNG(RNG.decode(RNG_num,i),cache->dat);
		blit(cache,screen,0,0,0,0,cache->w,cache->h);
		pal_fade_in(1);
		wait(100/gap);
	}
}



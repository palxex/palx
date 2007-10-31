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
#include "integer.h"

#include <time.h>

uint64_t seed;
uint32_t a=214013,c=2531011,d=16777216;

void randomize()
{
	time_t t;
	time(&t);
	double f=t;
	uint64_t u=*reinterpret_cast<uint64_t*>(&f);
	seed=((u>>48)^((u>>32)&0xffff))<<8;
}
float rnd0()
{
	seed=(seed*a+c)%d;
	return (float)seed/d;
}

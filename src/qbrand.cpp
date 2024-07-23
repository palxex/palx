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
#include <algorithm>
#include <assert.h>
#include <time.h>

#include "integer.h"
#include "config.h"
#include "internal.h"

namespace{
    uint32_t seed;
    uint32_t a=214013,c=2531011,d=16777216;
    bool is_random=true;
    double fixed_random=0;
}

void randomize()
{
	time_t t;
	time(&t);
	struct tm *local;
	local=localtime(&t);
	double f=(local->tm_hour*60+local->tm_min)*60+local->tm_sec;
	uint64_t u=*reinterpret_cast<uint64_t*>(&f);
	seed=((u>>48)^((u>>32)&0xffff))<<8;

	if(global->get<bool>("debug","random")==false){
		is_random=false;
		fixed_random=global->get<double>("debug","random");
	}
}
int roundto(double f)//四舍五入；到偶数？算了吧
{
	return ((f-(int)f)>0.5)?((int)f+1):(int)f;
}
double rnd0()
{
	if(!is_random)
		return fixed_random;
	seed=seed*a+c;
	return (double)seed/d;
}
double rnd1(double s)
{
    return s*rnd0();
}

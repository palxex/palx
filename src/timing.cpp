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
#include "timing.h"
#include "allegdef.h"
#include "internal.h"

extern bool running;
void wait(uint8_t gap)
{
	time_interrupt_occurs=0;
	rest(gap*8);
}


void wait_key(uint8_t gap)
{
	clear_keybuf();flush_screen();
	time_interrupt_occurs=0;
	while(!keypressed() && time_interrupt_occurs<gap && running)
		rest(10);
}

void wait_for_key()
{
	clear_keybuf();flush_screen();
	while(!keypressed() && running)
		rest(10);
	clear_keybuf();
}

void delay(uint8_t gap)
{
	rest(gap);
}

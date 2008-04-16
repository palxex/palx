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
#include "game.h"
#include "timing.h"
#include "UI.h"
#include "fade.h"

int current_dialog_lines = 0;
int glbvar_fontcolor  = 0x4F;
int font_color_yellow = 0x2D;
int font_color_red    = 0x1A;
int font_color_cyan   = 0x8D;
int font_color_cyan_1 = 0x8C;
int frame_pos_flag = 1;
int dialog_x = 12;
int dialog_y = 8;
int frame_text_x = 0x2C;
int frame_text_y = 0x1A;

int icon_x=0;
int icon_y=0;
int icon=0;

int delay_centisecond=3;
bool keepon_delay=true;

void show_wait_icon()
{
	keepon_delay=true;
	if(frame_pos_flag)
		Pal::message_handles.getsprite(icon)->blit_to(screen,icon_x,icon_y);
	wait_for_key();
	current_dialog_lines=0;
}

void dialog_string(const char *str,int x,int y,int color,bool shadow,BITMAP *bmp)
{
	char word[3];
	memset(word,0,3);
	for(size_t i=0,len=strlen(str);i<len;i++)
        if(((const unsigned char *)str)[i]>=0xA0)//big5 determination
        {
			strncpy(word,str+i,2);
            Font->blit_to(word,bmp,x,y,color,shadow);
            x+=16;
            i++;
        }
        else
            x+=8;
}

void draw_oneline_m_text(char *str,int x,int y)
{
	static char word[3];
	icon=0;
	int text_x=x;
	for(size_t i=0,len=strlen(str);i<len;i++){
		switch(str[i]){
			case '-':
				std::swap(glbvar_fontcolor, font_color_cyan);
				break;
			case '\'':
				std::swap(glbvar_fontcolor, font_color_red);
				break;
			case '\"':
				std::swap(glbvar_fontcolor, font_color_yellow);
				break;
			case '$':
				delay_centisecond=atoi(str+i+1)*10/7;
				i+=2;
				break;
			case '~':
				current_dialog_lines=0;
				keepon_delay=true;
				wait(atoi(str+i+1)*10/7);
				i=len;
				break;
			case ')':
				icon=1;
				break;
			case '(':
				icon=2;
				break;
			default:
				strncpy(word,str+i,2);
				Font->blit_to(word,screen,text_x,y,glbvar_fontcolor,true);
				if(async_getkey()==PAL_VK_EXPLORE)
					keepon_delay=false;
				perframe_proc();
				if(keepon_delay)
					delay(delay_centisecond);
				icon_x=text_x+=16;
				i++;
		}
	}
	icon_y=y;
}

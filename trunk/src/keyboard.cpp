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
#include "config.h"
#include "game.h"
#include <stack>
 class __scancode_map{
		std::map<int,int> mymap;
	public:
		__scancode_map(){
			mymap[KEY_LEFT]=0x4b;
			mymap[KEY_UP]=0x48;
			mymap[KEY_RIGHT]=0x4d;
			mymap[KEY_DOWN]=0x50;
		}
		int operator[](int n){
		    if(mymap.find(n)!=mymap.end())
                return mymap[n];
            else
                return 0;
		}
	};
int scancode_translate(int allegro_scancode)
{
	static __scancode_map scancode_map;
	return scancode_map[allegro_scancode];
}

int mykey[256];
int mykey_lowlevel[256];
std::stack<int> keys;

VKEY async_getkey()
{
	VKEY keygot=VK_NONE;
	int k;
	if(keypressed()){
		switch(k=(readkey()>>8)){
		case KEY_ESC:
		case KEY_INSERT:
		case KEY_ALT:
		case KEY_ALTGR:
			keygot = VK_MENU;
			break;
		case KEY_ENTER:
		case KEY_SPACE:
		case KEY_LCONTROL:
		case KEY_RCONTROL:
			keygot = VK_EXPLORE;
			break;
		case KEY_LEFT:
			keygot = VK_LEFT;
			break;
		case KEY_UP:
			keygot = VK_UP;
			break;
		case KEY_RIGHT:
			keygot = VK_RIGHT;
			break;
		case KEY_DOWN:
			keygot = VK_DOWN;
			break;
		case KEY_PGUP:
			keygot = VK_PGUP;
			break;
		case KEY_PGDN:
			keygot = VK_PGDN;
			break;
		case KEY_R:
			keygot = VK_REPEAT;
			break;
		case KEY_A:
			keygot = VK_AUTO;
			break;
		case KEY_D:
			keygot = VK_DEFEND;
			break;
		case KEY_E:
			keygot = VK_USE;
			break;
		case KEY_W:
			keygot = VK_THROW;
			break;
		case KEY_Q:
			keygot = VK_QUIT;
			break;
		case KEY_S:
			keygot = VK_STATUS;
			break;
		case KEY_F:
			keygot = VK_FORCE;
			break;
		case KEY_P:
		case KEY_PRTSCR:
			keygot = VK_PRINTSCREEN;
			break;
		default:
			keygot = VK_NONE;
		}
		clear_keybuf();
	}
	return keygot;
}
VKEY sync_getkey()
{
	VKEY x;
		extern bool running,is_out;
		x=VK_NONE;
		while(running && !is_out && !(x=async_getkey()))
			rest(10);
        return x;
}
int make_layer(int key)
{
	if(2<=key && key<=3)
		return 4-key;
	return 0;
}
extern int x_off,y_off;
void reproduct_key();
int examine_mutex=0,_examine_mutex=0;
VKEY get_key_lowlevel()
{
	VKEY keygot=VK_NONE;
	if(key[KEY_ESC] || key[KEY_INSERT] || key[KEY_ALT] || key[KEY_ALTGR])
		keygot = VK_MENU;
	else if(key[KEY_ENTER] || key[KEY_SPACE] || key[KEY_LCONTROL] || key[KEY_RCONTROL])
		keygot = VK_EXPLORE;
	/*else if
		case KEY_PGUP:
			keygot = VK_PGUP;
			break;
		case KEY_PGDN:
			keygot = VK_PGDN;
			break;
		case KEY_R:
			keygot = VK_REPEAT;
			break;
		case KEY_A:
			keygot = VK_AUTO;
			break;
		case KEY_D:
			keygot = VK_DEFEND;
			break;
		case KEY_E:
			keygot = VK_USE;
			break;
		case KEY_W:
			keygot = VK_THROW;
			break;
		case KEY_Q:
			keygot = VK_QUIT;
			break;
		case KEY_S:
			keygot = VK_STATUS;
			break;
		case KEY_F:
			keygot = VK_FORCE;
			break;
		case KEY_P:
		case KEY_PRTSCR:
			keygot = VK_PRINTSCREEN;
			break;
		default:
			keygot = VK_NONE;
			if(!clear)
				simulate_keypress(k<<8);
		}
		if(clear)
			clear_keybuf();
	}*/
	/*reproduct_key();
	int key_updown=0,key_leftright=0;
	int up=make_layer(mykey[KEY_UP]),down=make_layer(mykey[KEY_DOWN]),left=make_layer(mykey[KEY_LEFT]),right=make_layer(mykey[KEY_RIGHT]);
	if(!up && !down)
		key_updown=0,
		key_leftright=(left>right?-1:(right>left?1:0));
	if(!left && !right)
		key_leftright=0,
		key_updown=(up>down?-1:(down>up?1:0));

	if(up==2)
		key_updown=-1,key_leftright=0;
	if(down==2)
		key_updown=1,key_leftright=0;
	if(left==2)
		key_updown=0,key_leftright=-1;
	if(right==2)
		key_updown=0,key_leftright=1;

	if(!right && key_leftright==1)
		key_leftright=0;
	if(!left && key_leftright==-1)
		key_leftright=0;
	if(!down && key_updown==1)
		key_updown=0;
	if(!up && key_updown==-1)
		key_updown=0;

	x_off=((key_updown<0||key_leftright>0)?1:((key_updown>0||key_leftright<0)?-1:0));
	y_off=((key_updown>0||key_leftright>0)?1:((key_updown<0||key_leftright<0)?-1:0));*/

	while(_examine_mutex) rest(1);
	examine_mutex=1;
	if(!keys.empty())
	{
		x_off=((keys.top()==res::setup.key_left||keys.top()==res::setup.key_down)?-1:((keys.top()==res::setup.key_right||keys.top()==res::setup.key_up)?1:0));
		y_off=((keys.top()==res::setup.key_down||keys.top()==res::setup.key_right)?1:((keys.top()==res::setup.key_left||keys.top()==res::setup.key_up)?-1:0));
	}
	else
		x_off=0,y_off=0;
	examine_mutex=0;
	return keygot;
}
void keyhook(int);
void key_watcher(int scancode)
{
	/*memset(mykey,0,sizeof(mykey));
	for(int i=0;i<127;i++)
		if(key[i])
			mykey[i]=2;
	if(scancode<127 && mykey[scancode]==2)	return;*/
	keyhook(scancode);
	scancode=(scancode&0x80)|scancode_translate(scancode&0x7f);
	if(scancode>127){
		while(examine_mutex) rest(1);
		_examine_mutex=1;
		std::stack<int> another;
		for(size_t i=0;i<keys.size();){
			if(keys.top()!=(scancode&0x7f))
				another.push(keys.top());
			keys.pop();
		}
		for(size_t i=0;i<another.size();){
			keys.push(another.top());
			another.pop();
		}
		_examine_mutex=0;
	}else
		if(keys.empty() || keys.top()!=scancode && scancode)
			keys.push(scancode);
	mykey_lowlevel[scancode&0x7f]=(scancode>127?2:1);
}
END_OF_FUNCTION(key_watcher);
void reproduct_key()
{
	for(int i=0;i<127;i++)
	{
		int &key=mykey[i],&scan_key=mykey_lowlevel[i];
		if(scan_key==0)
			key=0;
		else if(scan_key==1)
			if(key<2)
				key=2;
			else
				key=2;
		else
			if(key<2)
				key=0;
			else
				key=1;
	}
}

extern bool no_barrier;
void keyhook(int)
{
	if(key[KEY_PRTSCR] || key[KEY_P]){
		static PALETTE pal;
		static char filename[80],mkdircmd[80];
		static int i=0;
		get_palette(pal);
		sprintf(filename,"%s/ScrnShot/%d.bmp",global->get<std::string>("config","path").c_str(),i++);
		sprintf(mkdircmd,"mkdir %s/ScrnShot",global->get<std::string>("config","path").c_str());
		system(mkdircmd);
		save_bitmap(filename,screen,pal);
	}
	if(key[KEY_F3])
        no_barrier=!no_barrier;
	//rest(1);
}

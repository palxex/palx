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
#ifndef GRAPH_HEADER
#define GRAPH_HEADER


#include <vector>
#include <boost/shared_ptr.hpp>

#include "pallib.h"
#include "resource.h"

#include "alfont.h"

#if !defined(WIN32)
#   define set_palette(x) set_palette_range(x,0,255,FALSE)
#endif

#include "adplug/opl.h"
#include "adplug/rix.h"

#define screen fakescreen

extern BITMAP *fakescreen,*backbuf,*bakscreen;
extern void redraw_everything(int gap=1,BITMAP * =screen);

class bitmap{
protected:
	BITMAP *bmp;
public:
	int width,height;
	bitmap(const uint8_t *,int,int);
	bitmap(BITMAP *);
	uint8_t *getdata();
	virtual ~bitmap();
	operator BITMAP *() const {return bmp;}
	bool blit_to(BITMAP *dest,int source_x=0,int source_y=0,int dest_x=0,int dest_y=0);
	friend void draw_battle_scene(int);
};
class sprite{
	uint8_t *buf;
public:
	int x,y,l;
	int width,height;
	sprite(uint8_t *);
	~sprite();
	sprite *clone();
	void setXYL(int,int,int);
	void blit_middle(BITMAP*,int,int);
	bool blit_to(BITMAP *);
	bool blit_to(BITMAP *dest,int,int,bool =false,int =6, int =6);
	friend bool operator<(const sprite &lhs,const sprite &rhs);
};
class sprite_prim{
	int id;
	std::vector<boost::shared_ptr<sprite> > sprites;
public:
	sprite_prim();
	sprite_prim(int);
	sprite_prim(cached_res &,int);
	sprite_prim(const sprite_prim &);
	sprite_prim &getsource(cached_res &,int =-1);
	sprite *getsprite(int);
	void blit(int i,BITMAP *bmp);
	friend bool operator==(const sprite_prim&,const sprite_prim&);
	friend class map;
};
class ttfont{
	const char *msg;
public:
	static ALFONT_FONT *glb_font;
	ttfont(const char *_msg):msg(_msg){}
	void blit_to(BITMAP *dest,int x,int y,uint8_t color,bool shadow);
};
class Game;extern Game *game;
class palette{
	struct myRGB{
		uint8_t r,g,b;
	};
	PALETTE pat[2];
	int pal;
	int day;
public:
	palette();
	void read(uint32_t i);
	PALETTE &get(int);
	void set(int t);
};
class CEmuopl;
class CrixPlayer;
class playrix
{
	boost::shared_ptr<Copl> 	opl;
	CrixPlayer 	rix;
	volatile int subsong;
	static const int samples=40;
	static const int sample_len=630;
	short 		*Buffer;
	friend void playrix_timer(void *);
	friend void update_cache(playrix*);
	static char mus[80];
	AUDIOSTREAM *stream;
	int max_vol;
public:
	playrix();
	~playrix();
	static void set(const char *name)
	{
		strcpy(mus,name);
	}
	void play(int sub_song,int =0);
	void stop(int =0);
	void setvolume(int);
};

class voc{
	SAMPLE *spl;
	SAMPLE *load_voc_mem(uint8_t *f);
	int max_vol;
public:
	voc(uint8_t *);
	void play();
};

enum VKEY { VK_NONE=0,VK_MENU=1,VK_EXPLORE,VK_DOWN,VK_LEFT,VK_UP,VK_RIGHT,VK_PGUP,VK_PGDN,VK_REPEAT,VK_AUTO,VK_DEFEND,VK_USE,VK_THROW,VK_QUIT,VK_STATUS,VK_FORCE,VK_PRINTSCREEN};
VKEY get_key(bool =true);
VKEY get_key_lowlevel();
void key_watcher(int scancode);

void SAFE_GETKEY(VKEY &,bool =false);

#endif

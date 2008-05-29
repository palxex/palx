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

#include "config.h"
#include "internal.h"

#include "alfont.h"

#if !defined(WIN32)
#   define set_palette(x) set_palette_range(x,0,255,FALSE)
#endif

#include "adplug/opl.h"
#include "adplug/rix.h"

extern BITMAP *realscreen;
#define screen fakescreen

#undef SCREEN_W
#undef SCREEN_H
#define SCREEN_W (global->get<int>("display","width"))
#define SCREEN_H (global->get<int>("display","height"))

#include "config.h"

extern BITMAP *fakescreen,*backbuf,*bakscreen;
extern void redraw_everything(int gap=1,BITMAP * =screen);

class bitmap{
protected:
	BITMAP *bmp;
public:
	int width,height;
	bitmap(const uint8_t * =NULL,int=SCREEN_W,int=SCREEN_H);
	bitmap(BITMAP *);
	bitmap(const bitmap &rhs);
	uint8_t *getdata();
	virtual ~bitmap();
	operator BITMAP *() const {return bmp;}
	bool blit_to(BITMAP *dest,int source_x=0,int source_y=0,int dest_x=0,int dest_y=0);
};
class sprite{
	uint8_t *buf;
	typedef Pal::Tools::PDECODERLECALLBACK filter_func;
	filter_func filter;
	int filt_data;
	friend bool operator==(const boost::shared_ptr<sprite> &lhs,const boost::shared_ptr<sprite> &rhs);
public:
	int x,y,l;
	int width,height;
	sprite(uint8_t *);
	~sprite();
	sprite *clone();
	void setfilter(filter_func r,int data);
	void setfilter();
	sprite *setXYL(int,int,int);
	void blit_middle(BITMAP*,int,int);
	void blit_middlebottom(BITMAP*,int,int);
	bool blit_to(BITMAP *);
	bool blit_to(BITMAP *dest,int,int,bool =false,int =6, int =6);
	void blit_filter(BITMAP *dest,int,int,filter_func r,int data,bool is,bool m=false);
};
class sprite_prim{
	int id;
	int frame;
	std::vector<boost::shared_ptr<sprite> > sprites;
public:
	int frames() const;
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
class def_font{
public:
	virtual void blit_to(const char *msg,BITMAP *dest,int x,int y,uint8_t color,bool shadow=true)=0;
};
class ttfont : public def_font{
	static ALFONT_FONT *glb_font;
public:
	ttfont();
	void blit_to(const char *msg,BITMAP *dest,int x,int y,uint8_t color,bool shadow);
};
class pixfont : public def_font{
	static uint16_t worbuf[10000];
	static uint8_t fonbuf[300000];
public:
	pixfont();
	void blit_to(const char *msg,BITMAP *dest,int x,int y,uint8_t color,bool shadow);
};
extern boost::shared_ptr<def_font> Font;

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
struct player{
	volatile int subsong;
	virtual void play(int sub_song,int =0)=0;
	virtual void stop(int =0)=0;
	virtual void setvolume(int)=0;
};
class playrix:public player
{
	boost::shared_ptr<Copl> 	opl;
	CrixPlayer 	rix;
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
	virtual ~playrix();
	static void set(const char *name)
	{
		strcpy(mus,name);
	}
	void play(int sub_song,int =0);
	void stop(int =0);
	void setvolume(int);
};
class playmidi:public player{
	MIDI *pmidi;
public:
	playmidi();
	virtual ~playmidi();
	void play(int sub_song,int =0);
	void stop(int =0);
	void setvolume(int);
};

class voc{
	SAMPLE *spl;
	SAMPLE *load_voc_mem(uint8_t *f);
	int max_vol;
	void init(uint8_t *);
public:
	voc(uint8_t *);
	voc(int);
	void play();
	static void stop();
};
extern boost::shared_ptr<player> musicplayer,cdplayer;

enum PAL_VKEY { PAL_VK_NONE=0,PAL_VK_MENU=1,PAL_VK_EXPLORE,PAL_VK_DOWN,PAL_VK_LEFT,PAL_VK_UP,PAL_VK_RIGHT,PAL_VK_PGUP,PAL_VK_PGDN,PAL_VK_REPEAT,PAL_VK_AUTO,PAL_VK_DEFEND,PAL_VK_USE,PAL_VK_THROW,PAL_VK_QUIT,PAL_VK_STATUS,PAL_VK_FORCE,PAL_VK_PRINTSCREEN};
PAL_VKEY async_getkey();
PAL_VKEY sync_getkey();

PAL_VKEY get_key_lowlevel();
void key_watcher(int scancode);
#endif

#pragma warning(disable:4312)
#ifndef GRAPH_HEADER
#define GRAPH_HEADER


#include <vector>
#include <boost/shared_ptr.hpp>

#include "pallib.h"
#include "resource.h"

#if defined _DEBUG
	#define ALFONT_DLL
#endif
#include <alfont.h>

#include "adplug/emuopl.h"
#include "adplug/rix.h"

class bitmap{
protected:
	BITMAP *bmp;
public:
	bitmap(const uint8_t *,int,int);
	virtual ~bitmap();
	bool blit_to(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y);
	friend void draw_battle_scene(int);
};
class sprite{
	uint8_t *buf;
public:
	int x,y,l;
	int width,height;
	sprite(uint8_t *);
	~sprite();
	void setXYL(int,int,int);
	void blit_middle(BITMAP*,int,int);
	bool blit_to(BITMAP *);
	bool blit_to(BITMAP *dest,int,int);
	friend bool sprite_comp(sprite *lhs,sprite *rhs);
};
class sprite_prim{
	int id;
	std::vector<boost::shared_ptr<sprite> > sprites;	
	int determain_smkfs(uint8_t *src)
	{
		uint16_t *usrc=(uint16_t*)src;
		return usrc[0]-(usrc[usrc[0]-1]==0?1:0);
	}
public:
	sprite_prim();
	sprite_prim(int);
	sprite_prim(cached_res &,int);
	sprite_prim(int,uint8_t *src);
	sprite_prim(const sprite_prim &);
	sprite_prim &getsource(uint8_t *src);
	sprite *getsprite(int);
	void blit(int i,BITMAP *bmp);
	friend bool operator==(const sprite_prim&,const sprite_prim&);
	friend class map;
};
bool operator==(const sprite_prim&,const sprite_prim&);
class ttfont{
	const char *msg;
public:
	static ALFONT_FONT *glb_font;
	ttfont(const char *_msg):msg(_msg){}
	void blit_to(BITMAP *dest,int x,int y,uint8_t color){
		alfont_textout(dest, glb_font, msg, x, y, color);
	}
	void shadow_blit(BITMAP *dest,int x,int y,uint8_t color){
		blit_to(dest, x+1, y+1, 0);
		blit_to(dest, x, y, color);
	}
};
class palette{
	struct myRGB{
		uint8_t r,g,b;
	};
	PALETTE pat[2];
	int pal;
	int day;
public:
	palette()
		:pal(-1),day(-1)
	{
	}
	void set(uint32_t i,int offset)
	{
		if(i==pal)
			return;
		pal=i;
		long len;
		myRGB *buf=(myRGB *)PAT.decode(i,len);
		RGB   *p=(RGB*)pat;
		for(int t=0;t<len/3;t++)
			p[t].r=buf[t].r,
			p[t].g=buf[t].g,
			p[t].b=buf[t].b;
		set_to(offset!=0);
	}
	PALETTE &get()
	{
		return pat[day];
	}
	void set_to(int t)
	{
		if(day==t)
			return;
		day=t;
		set_palette(pat[day]);
	}
	void switch_daytime()
	{
		day=!day;
		set_to(day);
	}
};
class CEmuopl;
class CrixPlayer;
class playrix
{
	CEmuopl 	opl;
	CrixPlayer 	rix;
	AUDIOSTREAM *stream;
	short 		*Buffer, *buf;
	volatile long leaving;
	volatile long 	slen,slen_buf, BufferLength;
	volatile int tune,subsong;
	friend void playrix_timer(void *);
public:
	playrix();
	~playrix();
	void play(int sub_song);
	void stop();
};

enum VKEY { VK_MENU=1,VK_EXPLORE,VK_DOWN,VK_LEFT,VK_UP,VK_RIGHT,VK_PGUP,VK_PGDN,VK_REPEAT,VK_AUTO,VK_DEFEND,VK_USE,VK_THROW,VK_QUIT,VK_STATUS,VK_FORCE,VK_NONE};
extern int get_key();
#endif
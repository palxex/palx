#pragma warning(disable:4312)
#ifndef GRAPH_HEADER
#define GRAPH_HEADER


#include <vector>
#include <boost/shared_ptr.hpp>

#include "pallib.h"
#include "resource.h"

#define ALFONT_DLL
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
};
class sprite{
	uint8_t *buf;
public:
	sprite(uint8_t *);
	~sprite();
	bool blit_to(BITMAP *dest,int dest_x,int dest_y);
};
class sprite_action{
	std::vector<boost::shared_ptr<sprite> > sprites;	
	int determain_smkfs(uint8_t *src)
	{
		uint16_t *usrc=(uint16_t*)src;
		return usrc[0]-(usrc[usrc[0]-1]==0?1:0);
	}
public:
	void getsource(uint8_t *src){
		for(int i=0;i<determain_smkfs(src);i++)
			sprites.push_back(boost::shared_ptr<sprite>(new sprite(src+((uint16_t *)src)[i])));
	}
	void blit(int i,BITMAP *bmp,int dest_x,int dest_y)
	{
		sprites[i]->blit_to(bmp,dest_x,dest_y);
	}
};

class ttfont{
	const char *msg;
public:
	static ALFONT_FONT *glb_font;
	ttfont(const char *_msg):msg(_msg){}
	void blit_to(BITMAP *dest,int start,int end,int x,int y){
		alfont_textout(dest, glb_font, msg, x, y, 0xef);
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
	void set(uint32_t i)
	{
		if(i==pal)
			return;
		pal=i;
		long len;
		myRGB *buf=(myRGB *)PAT.decode(i,0,len);
		RGB   *p=(RGB*)pat;
		for(int t=0;t<len/3;t++)
			p[t].r=buf[t].r,
			p[t].g=buf[t].g,
			p[t].b=buf[t].b;
		set_to(0);
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

enum VKEY { VK_MENU=1,VK_EXPLORE,VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_PGUP,VK_PGDN,VK_REPEAT,VK_AUTO,VK_DEFEND,VK_USE,VK_THROW,VK_QUIT,VK_STATUS,VK_FORCE};
extern int get_key();
#endif
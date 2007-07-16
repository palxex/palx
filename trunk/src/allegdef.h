#pragma warning(disable:4312)
#ifndef GRAPH_HEADER
#define GRAPH_HEADER


#include <vector>
#include <boost/shared_ptr.hpp>

#include "pallib.h"
#include "resource.h"

#include <alfont.h>

#include "adplug/emuopl.h"
#include "adplug/rix.h"

class bitmap{
protected:
	BITMAP *bmp;
public:
	bitmap(const uint8_t *,int,int);
	virtual ~bitmap();
	operator BITMAP *(){return bmp;}
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
	sprite *clone();
	void setXYL(int,int,int);
	void blit_middle(BITMAP*,int,int);
	bool blit_to(BITMAP *);
	bool blit_to(BITMAP *dest,int,int,bool =false);
	friend bool operator<(const sprite &lhs,const sprite &rhs);
};
class sprite_prim{
	int id;
	std::vector<boost::shared_ptr<sprite> > sprites;	
	int determain_smkfs(uint8_t *src);
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
	void switch_daytime();
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
	bool playing;
	friend void playrix_timer(void *);
public:
	playrix();
	~playrix();
	void play(int sub_song);
	void stop();
};

class voc{
	SAMPLE *spl;
	SAMPLE *load_voc_mem(uint8_t *f);
public:
	voc(uint8_t *);
	~voc();
	void play();
};

enum VKEY { VK_NONE=0,VK_MENU=1,VK_EXPLORE,VK_DOWN,VK_LEFT,VK_UP,VK_RIGHT,VK_PGUP,VK_PGDN,VK_REPEAT,VK_AUTO,VK_DEFEND,VK_USE,VK_THROW,VK_QUIT,VK_STATUS,VK_FORCE,VK_PRINTSCREEN};
VKEY get_key(bool =true);
VKEY get_key_lowlevel();
#endif

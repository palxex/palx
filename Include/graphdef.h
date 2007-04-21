#pragma warning(disable:4312)
#ifndef GRAPH_HEADER
#define GRAPH_HEADER


#include <list>
#include <boost/shared_ptr.hpp>

#include "pallib.h"
#include "resource.h"

#define ALFONT_DLL
#include <alfont.h>
class bitmap{
	BITMAP *bmp;
public:
	bitmap(const uint8_t *,int,int);
	~bitmap();
	bool blit_to(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y);
};
class sprite{
	boost::shared_array<uint8_t> buf;
public:
	sprite(uint8_t *);
	~sprite();
	bool blit_to(BITMAP *dest,int dest_x,int dest_y);
};
class scene{
	BITMAP *wholescene;
	BITMAP *scene_buf;
	typedef std::list<boost::shared_ptr<sprite> > s_list;
	s_list sprites;
	s_list active_list;
	struct position{
		int x,y,h;
		bool status;
		position(int x_,int y_,int h_):x(x_),y(y_),h(h_),status(true){}
		position(int x_,int y_):x(x_),y(y_),status(false){}
		void toXYH(){	if(!status){	h=(x%32!=0);x=x/32;y=y;	}}
		void toXY(){	if(status){		x=x*32+h*16;y=y*32+h*8;	}}
	}user_pos,camera_pos;
public:
	scene();
	~scene();
	virtual void draw_scene();
};
class battle_scene:scene{};
class cut_msg_impl
{
	FILE *fp;
	boost::shared_array<char> glb_buf;
	char buf[100];
public:
	cut_msg_impl(const char *fname="m.msg")
		:fp(fopen(fname,"rb"))
	{
		long len;fseek(fp,0,SEEK_END);len=ftell(fp);rewind(fp);
		glb_buf=boost::shared_array<char>(new char[len]);
		fread(glb_buf.get(),len,1,fp);
		fclose(fp);
	}
	char *operator()(int start,int end)
	{
		assert(end>start);assert(start>=0);
		memset(buf,0,sizeof(buf));
		memcpy(buf,glb_buf.get()+start,end-start);
		return buf;
	}
};

class ttfont{
	cut_msg_impl cut_msg;
public:
	static ALFONT_FONT *glb_font;
	ttfont(const char *file):cut_msg(file){}
	void blit_to(BITMAP *dest,int start,int end,int x,int y){
		alfont_textout(dest, glb_font, cut_msg(start,end), x, y, 0xef);
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
		myRGB *buf=(myRGB *)PAT.decode(i,0,len).get();
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
#endif
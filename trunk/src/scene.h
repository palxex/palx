#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "resource.h"
#include "allegdef.h"

#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

struct scene_map:public bitmap
{
	virtual void change(int p)=0;
	scene_map(const uint8_t *a,int b,int c):bitmap(a,b,c){}
protected: 
	long len;
};
class fbp:public scene_map{
public: 
	fbp():scene_map(0,320,200){}
	void change(int p){
		memcpy(bmp->dat,FBP.decode(p,0),bmp->w*bmp->h);
	}
};
class map:public scene_map{
public: 
	map():scene_map(0,2032,2040){}
	void change(int p){
		uint8_t *mapbuf=MAP.decode(p,0);
		for(int y=0;y<0x80;y++)
			for(int x=0;x<0x40;x++)
				for(int h=0;h<2;h++)
				{
					uint8_t *_buf=mapbuf+y*0x200+x*8+h*4;
					int index=(_buf[1]&0x10)<<4|_buf[0],index2=(_buf[3]&0x10)<<4|_buf[2];
					sprite(GOP.decode(p,index)).blit_to(bmp,x*32+h*16-16,y*16+h*8-8);
					if(index2)
						sprite(GOP.decode(p,index2-1)).blit_to(bmp,x*32+h*16-16,y*16+h*8-8);
				}
	}
};

struct Scene{
	::map scenemap;
	BITMAP *scene_buf;
	int current,toload;
	std::vector<EVENT_OBJECT>::iterator sprites_begin,sprites_end;
	typedef std::list<boost::shared_ptr<sprite_action> > s_list;
	s_list active_list;
	struct position{
		int x,y,h;
		bool status;
		position(int x_,int y_,int h_):x(x_),y(y_),h(h_),status(true){}
		position(int x_,int y_):x(x_),y(y_),status(false){}
		position():x(0),y(0),status(false){}
		position &toXYH(){	if(!status){	h=(x%32!=0);x=x/32;y=y/16;	status=true;} return *this;}
		position &toXY(){	if(status){		x=x*32+h*16;y=y*16+h*8;status=false;}    return *this;}
	}team_pos,camera_pos;
	Scene();
	~Scene();
	void clear_scanlines();
	void clear_active();
	void calc_team_walking(int key);
	void our_team_setdraw();
	void visible_NPC_movment_setdraw();
	void Redraw_Tiles_or_Fade_to_pic();
	void move_usable_screen();
	void get_sprites();
	void produce_one_screen();
	void process_scrn_drawing(int);
};
struct BattleScene{
	fbp background;
	typedef std::list<sprite *> s_list;
	s_list active_list;
	void draw();
};


class cut_msg_impl
{
	FILE *fp;
	char *glb_buf;
	char buf[100];
public: 
	cut_msg_impl(const char *fname="m.msg")
		:fp(fopen(fname,"rb"))
	{
		long len;fseek(fp,0,SEEK_END);len=ftell(fp);rewind(fp);
		glb_buf=new char[len];
		fread(glb_buf,len,1,fp);
		fclose(fp);
	}
	~cut_msg_impl()
	{
		delete[] glb_buf;
	}
	char *operator()(int start,int end)
	{
		assert(end>start);assert(start>=0);
		memset(buf,0,sizeof(buf));
		memcpy(buf,glb_buf+start,end-start);
		return buf;
	}
};

#endif
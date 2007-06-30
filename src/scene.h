#ifndef MAINLOOP_H
#define MAINLOOP_H

#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "resource.h"
#include "allegdef.h"

struct scene_map:public bitmap
{
	virtual void change(int p)=0;
	scene_map(const uint8_t *a,int b,int c):bitmap(a,b,c){}
protected: 
	long len;
};
class fbp:public scene_map{
public: 
	fbp():scene_map(0,SCREEN_W,SCREEN_H){}
	void change(int p){
		memcpy(bmp->dat,FBP.decode(p,0),bmp->w*bmp->h);
	}
};
struct tile{
	boost::shared_ptr<sprite> image;
	bool blocked;
	int layer;
	bool valid;
	tile():image((sprite*)0),blocked(0),layer(0),valid(false){}
};
class palmap:public scene_map{
	boost::multi_array<tile,4> sprites;
	sprite &getsprite(int x,int y,int h,int l,uint8_t *src,bool throu,int layer);
public: 
	void make_tile(uint8_t*,int,int,int,BITMAP*);
	void make_onescreen(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y);
	void blit_to(BITMAP *dest,int sx,int sy,int dx,int dy);
	tile &gettile(int x,int y,int h,int l);
	palmap();
	void change(int p);
};

struct Scene{
	palmap scenemap;
	BITMAP *scene_buf;
	std::vector<EVENT_OBJECT>::iterator sprites_begin,sprites_end;
	typedef std::vector<boost::shared_ptr<sprite> > s_list;
	s_list active_list;
	struct position{
		friend struct Scene;
		int x,y,h;
		bool status;
		position(int x_,int y_,int h_):x(x_),y(y_),h(h_),status(true){}
		position(int x_,int y_):x(x_),y(y_),status(false){}
		position():x(0),y(0),status(false){}
		position &toXYH(){	if(!status){	h=(x%32!=0);x=x/32;y=y/16;	status=true;} return *this;}
		position &toXY(){	if(status){		x=x*32+h*16;y=y*16+h*8;status=false;}    return *this;}
		position operator+(const position &rhs){
			if(rhs.status)
				return position(toXYH().x+rhs.x,toXYH().y+rhs.y,toXYH().h+rhs.h);
			else
				return position(toXY().x+rhs.x,toXY().y+rhs.y);
		}
		position &operator=(position &rhs)
		{
			x=rhs.x;y=rhs.y;h=rhs.h;status=rhs.status;
			return *this;
		}
	}team_pos;
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
	void draw_normal_scene(int);
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
	char *operator()(int start,int end=-1)
	{
		if(end==-1)
			end=start+10;
		assert(end>start);assert(start>=0);
		memset(buf,0,sizeof(buf));
		memcpy(buf,glb_buf+start,end-start);
		return buf;
	}
};

#endif

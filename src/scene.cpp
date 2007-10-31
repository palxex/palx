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
#include "scene.h"
#include "internal.h"
#include "game.h"
#include <algorithm>
#include <set>

tile non_valid;

inline sprite &palmap::getsprite(int x,int y,int h,int l,uint8_t *src,bool throu,int layer)
{
	tile &t=sprites[x][y][h][l];
	if(!t.valid){
		t.image=boost::shared_ptr<sprite>(new sprite(src));
		t.blocked=throu;
		t.layer=layer;
		t.valid=true;
	}
	return *t.image.get();
}
tile &palmap::gettile(int x,int y,int h,int l)
{
	if(x<0||y<0||h<0||x>0x40-1||y>0x80-1||h>1) return non_valid;
	return sprites[x][y][h][l];
}
palmap::palmap():scene_map(0,SCREEN_W,SCREEN_H),sprites(boost::extents[0x40][0x80][2][2])
{
	for(int i=0;i<0x40;i++)
		for(int j=0;j<0x80;j++)
			for(int k=0;k<2;k++)
				for(int t=0;t<2;t++)
					sprites[i][j][k][t].valid=false;
}
int t=-1;
void palmap::change(int p){
	if(t!=p)
		for(int i=0;i<0x40;i++)
			for(int j=0;j<0x80;j++)
				for(int k=0;k<2;k++)
					for(int t=0;t<2;t++)
						sprites[i][j][k][t].valid=false;
	t=p;
}
inline void palmap::make_tile(uint8_t *_buf,int x,int y,int h,BITMAP *dest)
{
	if(x<0||y<0||h<0||x>0x40-1||y>0x80-1||h>1) return;
	if(sprites[x][y][h][0].valid && sprites[x][y][h][1].valid){
		sprites[x][y][h][0].image->blit_to(dest,x*32+h*16-16-res::rpg.viewport_x,y*16+h*8-8-res::rpg.viewport_y);
		if(sprites[x][y][h][1].valid)
			sprites[x][y][h][1].image->blit_to(dest,x*32+h*16-16-res::rpg.viewport_x,y*16+h*8-8-res::rpg.viewport_y);
		return;
	}
	int index=(_buf[1]&0x10)<<4|_buf[0],index2=(_buf[3]&0x10)<<4|_buf[2];
	bool throu=(_buf[1]&0x20)?true:false,throu2=(_buf[3]&0x20)?true:false;
	int layer=_buf[1]&0xf,layer2=_buf[3]&0xf;
	getsprite(x,y,h,0,GOP.decode(t,index),throu,layer).blit_to(dest,x*32+h*16-16-res::rpg.viewport_x,y*16+h*8-8-res::rpg.viewport_y);
	if(index2)
		getsprite(x,y,h,1,GOP.decode(t,index2-1),throu2,layer2).blit_to(dest,x*32+h*16-16-res::rpg.viewport_x,y*16+h*8-8-res::rpg.viewport_y);
}
void palmap::make_onescreen(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y)
{
	uint8_t *mapbuf=MAP.decode(t,0);
	for(int y=source_y/16;y<dest_y/16+2;y++)
		for(int x=source_x/32;x<dest_x/32+1;x++)
			for(int h=0;h<2;h++)
				make_tile(mapbuf+y*0x200+x*8+h*4,x,y,h,bmp);
	blit(bmp,dest,source_x-res::rpg.viewport_x,source_y-res::rpg.viewport_y,source_x-res::rpg.viewport_x,source_y-res::rpg.viewport_y,dest_x-source_x,dest_y-source_y);
}
void palmap::blit_to(BITMAP *dest,int sx,int sy,int dx,int dy)
{
	bitmap::blit_to(dest,sx,sy,dx,dy);
	blit(bmp,bmp,sx,sy,dx,dy,bmp->w,bmp->h);
}

Scene::Scene():scene_buf(create_bitmap(SCREEN_W,SCREEN_H)),team_pos(res::rpg.viewport_x+x_scrn_offset,res::rpg.viewport_y+y_scrn_offset)
{}
Scene::~Scene()
{}
void Scene::clear_scanlines()
{
	//clear_bitmap(screen);
}
void sprite_queue::clear_active()
{
	active_list.clear();
}
void sprite_queue::calc_team_walking()
{
	int16_t &direction=res::rpg.team_direction;
	abstract_x_bak=scene->team_pos.toXY().x;
	abstract_y_bak=scene->team_pos.toXY().y;
	viewport_x_bak=res::rpg.viewport_x;
	viewport_y_bak=res::rpg.viewport_y;
	if(x_off && y_off && key_enable){
		direction=calc_faceto(x_off,y_off);
		position target=scene->team_pos+position(direction_offs[direction][0],direction_offs[direction][1]);
		if(!barrier_check(0,target.toXY().x,target.toXY().y)&&
			target.toXY().x>=0 && target.toXY().x<=coordinate_x_max+x_scrn_offset &&
			target.toXY().y>=0 && target.toXY().y<=coordinate_y_max+y_scrn_offset)
		{
			res::rpg.viewport_x+=direction_offs[direction][0];
			res::rpg.viewport_y+=direction_offs[direction][1];
			scene->team_pos.toXY()=target;
			team_walk_one_step();
			return;
		}
	}
	stop_and_update_frame();
	scene->team_pos.x=res::rpg.viewport_x+x_scrn_offset;
	scene->team_pos.y=res::rpg.viewport_y+y_scrn_offset;
}
void sprite_queue::our_team_setdraw()
{
	for(int i=0;i<=res::rpg.team_roles+res::rpg.team_followers;i++){
		s_list::value_type it=boost::shared_ptr<sprite>(mgos[team_mgos[i]].getsprite(res::rpg.team[i].frame)->clone());
		it->setXYL(res::rpg.team[i].x,res::rpg.team[i].y+res::rpg.layer+10,res::rpg.layer+6);
		active_list.push_back(it);
	}
}
void sprite_queue::visible_NPC_movment_setdraw()
{
	int t=0;
	for(std::vector<EVENT_OBJECT>::iterator i=scene->sprites_begin;i!=scene->sprites_end;i++,t++)
		if(i->pos_x-res::rpg.viewport_x>-64*scale && i->pos_x-res::rpg.viewport_x<0x180*scale &&
		   i->pos_y-res::rpg.viewport_y>0   && i->pos_y-res::rpg.viewport_y<0x148*scale &&
		   i->image && i->status && i->vanish_time==0)
		{
			int frame=i->curr_frame;
			if(i->frames==3){
				//if(i->curr_frame==2)
				//	frame=0;
				if(frame==3)
					frame=2;
			}
			s_list::value_type it=boost::shared_ptr<sprite>(mgos[npc_mgos[t]].getsprite(i->direction*i->frames+frame)->clone());
			it->setXYL(i->pos_x-res::rpg.viewport_x,i->pos_y-res::rpg.viewport_y+i->layer*8+9,i->layer*8+2);
			active_list.push_back(it);
		}
}
void sprite_queue::Redraw_Tiles_or_Fade_to_pic()
{
	s_list redraw_list;s_list::value_type masker;
	switch(redraw_flag)
	{
	case 0:
		for(s_list::iterator i=active_list.begin();i!=active_list.end()&&(masker=*i);i++)
			for(int vx=(res::rpg.viewport_x+masker->x-masker->width/2)/32;vx<=(res::rpg.viewport_x+masker->x+masker->width/2)/32;vx++)
				for(int vy=(res::rpg.viewport_y+masker->y-masker->height)/16-1,vh=(res::rpg.viewport_y+masker->y)%16/8;vy<=(res::rpg.viewport_y+masker->y)/16+1;vy++)
					for(int x=vx-1,y=vy;x<=vx+1 && y>=0;x++)
						for(int h=0;h<2;h++)
						{
							tile &tile0=scene->scenemap.gettile(x,y,h,0);
							tile &tile1=scene->scenemap.gettile(x,y,h,1);
							if(tile0.valid && tile0.layer>0 && 16*(y+tile0.layer)+8*h+8>=masker->y){
								s_list::value_type it=boost::shared_ptr<sprite>(tile0.image.get()->clone());
								it->setXYL(32*x+16*h-res::rpg.viewport_x,16*y+8*h+7+tile0.layer*8-res::rpg.viewport_y,tile0.layer*8);
								redraw_list.push_back(it);
							}
							if(tile1.valid && tile1.layer>0 && 16*(y+tile1.layer)+8*h+8>=masker->y){
								s_list::value_type it=boost::shared_ptr<sprite>(tile1.image.get()->clone());
								it->setXYL(32*x+16*h-res::rpg.viewport_x,16*y+8*h+7+tile1.layer*8+1-res::rpg.viewport_y,tile1.layer*8+1);
								redraw_list.push_back(it);
							}
						}
		std::copy(redraw_list.begin(),redraw_list.end(),std::back_inserter(active_list));
		break;
	case 1:
		return;
	case 2:
		/*many many*/
		redraw_flag=1;
		break;
	}
}
void sprite_queue::flush(bitmap &scanline)
{
	std::set<s_list::value_type> greper(active_list.begin(),active_list.end());
	active_list.clear();
	std::copy(greper.begin(),greper.end(),back_inserter(active_list));
	std::sort(active_list.begin(),active_list.end());
	for(s_list::iterator i=active_list.begin();i!=active_list.end();i++)
		(*i)->blit_to(scanline);
}
void Scene::move_usable_screen()
{
	if(redraw_flag==0)
	{
		/*produce_one_screen();*/
		if(viewport_x_bak!=res::rpg.viewport_x || viewport_y_bak!=res::rpg.viewport_y)
		{
			int x1=0,x2=0,y1=0,y2=0,vx1=0,vy1=0,vx2=0,vy2=0,tx=abs(res::rpg.viewport_x-viewport_x_bak),ty=abs(res::rpg.viewport_y-viewport_y_bak);
			if(res::rpg.viewport_y > viewport_y_bak)
				y1=SCREEN_H-ty,y2=SCREEN_H,	vy1=ty,vy2=0;
			else if(res::rpg.viewport_y < viewport_y_bak)
				y1=0,y2=ty,					vy1=0,vy2=ty;
			if(res::rpg.viewport_x > viewport_x_bak)
				x1=SCREEN_W-tx,x2=SCREEN_W,	vx1=tx,vx2=0;
			else if(res::rpg.viewport_x < viewport_x_bak)
				x1=0,x2=tx,					vx1=0,vx2=tx;

  			scenemap.blit_to(scene_buf,vx1,vy1,vx2,vy2);

			short &vx=res::rpg.viewport_x,&vy=res::rpg.viewport_y;
 			scenemap.make_onescreen(scene_buf,vx,vy+y1,vx+SCREEN_W,vy+y2);
			scenemap.make_onescreen(scene_buf,vx+x1,vy,vx+x2,vy+SCREEN_H);
		}
	}
	key_enable=true;
}
void Scene::get_sprites()
{
	load_NPC_mgo();
}
void Scene::produce_one_screen()
{
	viewport_x_bak=res::rpg.viewport_x;
	viewport_y_bak=res::rpg.viewport_y;
	scenemap.make_onescreen(scene_buf,res::rpg.viewport_x,res::rpg.viewport_y,res::rpg.viewport_x+SCREEN_W,res::rpg.viewport_y+SCREEN_H);
}
template<typename T>
bool operator<(boost::shared_ptr<T> &lhs,boost::shared_ptr<T> &rhs)
{
	return *(lhs.get())<*(rhs.get());
}
void Scene::scanline_draw_normal_scene(sprite_queue &sprites,int gap)
{
	perframe_proc();
	static bitmap scanline(0,SCREEN_W,SCREEN_H);
	blit(scene_buf,scanline,0,0,0,0,SCREEN_W,SCREEN_H);
	sprites.flush(scanline);
	blit(scanline,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	pal_fade_in(gap);
}

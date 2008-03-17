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
#include "internal.h"
#include "resource.h"
#include "game.h"
#include "scene.h"
#include <algorithm>

int step_off_x=16,step_off_y=8;
int coordinate_x_max=1696,coordinate_y_max=1840;
int x_scrn_offset;
int y_scrn_offset;
int abstract_x_bak=0,abstract_y_bak=0;
int viewport_x_bak=0,viewport_y_bak=0;
int direction_offs[4][2]={{-16,8},{-16,-8},{16,-8},{16,8}};
int this_step_frame=0;
int step_frame_follower=0,step_frame_leader=0;

std::vector<sprite_prim> mgos;
std::map<int,int> team_mgos;
std::map<int,int> npc_mgos;

int load_mgo(int id)
{
	mgos.push_back(sprite_prim(res::MGO,id));
	return std::find(mgos.begin(),mgos.end(),sprite_prim(id))-mgos.begin();
}
void load_team_mgo()
{
	team_mgos.clear();
	for(int i=0;i<=res::rpg.team_roles;i++)
	{
		int id=res::rpg.roles_properties.avator[res::rpg.team[i].role];
		team_mgos[i]=load_mgo(id);
	}
	for(int i=1;i<=res::rpg.team_followers;i++)
		team_mgos[res::rpg.team_roles+i]=load_mgo(res::rpg.team[res::rpg.team_roles+i].role);
}
void load_NPC_mgo()
{
	npc_mgos.clear();
	for(std::vector<EVENT_OBJECT>::iterator i=scene->sprites_begin;i!=scene->sprites_end;i++)
		if(i->image)
			npc_mgos[i-scene->sprites_begin]=load_mgo(i->image);
}
void store_step()
{
	/*
	memcpy(res::rpg.team_track+sizeof(RPG::track),res::rpg.team_track,4*sizeof(RPG::track));
	/*///
	res::rpg.team_track[4]=res::rpg.team_track[3];
	res::rpg.team_track[3]=res::rpg.team_track[2];
	res::rpg.team_track[2]=res::rpg.team_track[1];
	res::rpg.team_track[1]=res::rpg.team_track[0];
	//*/
}
void record_step()
{
	store_step();
	res::rpg.team_track[0].x=abstract_x_bak;
	res::rpg.team_track[0].y=abstract_y_bak;
	res::rpg.team_track[0].direction=res::rpg.team_direction;
}
void calc_trace_frames()
{
	this_step_frame=(this_step_frame+1)&3;
	if(this_step_frame & 1)
		step_frame_leader=(this_step_frame+1)/2,
		step_frame_follower=3-step_frame_leader;
	else
		step_frame_leader=step_frame_follower=0;

	store_step();
}
void calc_followers_screen_pos()
{
	res::rpg.team[0].frame=(res::rpg.roles_properties.walk_frames[res::rpg.team[0].role]==4? res::rpg.team_direction*4+this_step_frame : res::rpg.team_direction*3+step_frame_leader);
	res::rpg.team[0].x=x_scrn_offset;
	res::rpg.team[0].y=y_scrn_offset;
	res::rpg.team_track[0].direction=res::rpg.team_direction;
	res::rpg.team_track[0].x=abstract_x_bak;
	res::rpg.team_track[0].y=abstract_y_bak;

	for(int i=1;i<=res::rpg.team_roles;i++)
	{
		res::rpg.team[i].x=res::rpg.team_track[1].x-res::rpg.viewport_x;
		res::rpg.team[i].y=res::rpg.team_track[1].y-res::rpg.viewport_y;
		if(i==2)
		{
			res::rpg.team[i].y+=8;
			res::rpg.team[i].x+=(res::rpg.team_track[1].direction&1?-16:16);
		}else{
			res::rpg.team[i].x-=direction_offs[res::rpg.team_track[1].direction][0];
			res::rpg.team[i].y-=direction_offs[res::rpg.team_track[1].direction][1];
		}
		if(barrier_check(0,res::rpg.team[i].x+res::rpg.viewport_x,res::rpg.team[i].y+res::rpg.viewport_y))
			res::rpg.team[i].x=res::rpg.team_track[1].x-res::rpg.viewport_x,
			res::rpg.team[i].y=res::rpg.team_track[1].y-res::rpg.viewport_y;
		res::rpg.team[i].frame=(res::rpg.roles_properties.walk_frames[res::rpg.team[i].role]==4? res::rpg.team_track[2].direction*4+this_step_frame : res::rpg.team_track[2].direction*3+step_frame_follower);
	}
	for(int i=1;i<=res::rpg.team_followers;i++)
	{
		res::rpg.team[res::rpg.team_roles+i].x=res::rpg.team_track[i+2].x-res::rpg.viewport_x;
		res::rpg.team[res::rpg.team_roles+i].y=res::rpg.team_track[i+2].y-res::rpg.viewport_y;
		res::rpg.team[res::rpg.team_roles+i].frame=res::rpg.team_track[i+2].direction*3+step_frame_follower;
	}
}
void team_walk_one_step()
{
	scene->team_pos.toXY().x=res::rpg.viewport_x+x_scrn_offset;
	scene->team_pos.toXY().y=res::rpg.viewport_y+y_scrn_offset;
	if(scene->team_pos.toXY().x!=abstract_x_bak || scene->team_pos.toXY().y!=abstract_y_bak)
		calc_trace_frames();
	else
		step_frame_follower=0,step_frame_leader=0,
		this_step_frame&=2,this_step_frame^=2;
	calc_followers_screen_pos();
}
void stop_and_update_frame()
{
	res::rpg.team[0].frame=res::rpg.team_direction*(res::rpg.roles_properties.walk_frames[res::rpg.team[0].role]? res::rpg.roles_properties.walk_frames[res::rpg.team[0].role] : 3);
	for(int i=1;i<=res::rpg.team_roles;i++)
		res::rpg.team[i].frame=res::rpg.team_track[2].direction*(res::rpg.roles_properties.walk_frames[res::rpg.team[i].role]? res::rpg.roles_properties.walk_frames[res::rpg.team[i].role] : 3);
	for(int i=1;i<=res::rpg.team_followers;i++)
		res::rpg.team[res::rpg.team_roles+i].frame=res::rpg.team_track[i+2].direction*3;
	this_step_frame&=2,this_step_frame^=2;
}

int calc_faceto(int x_diff,int y_diff)
{
	if(x_diff>=0 && y_diff>=0)
		return 3;
	else if(x_diff>=0 && y_diff<=0)
		return 2;
	else if(x_diff<=0 && y_diff<=0)
		return 1;
	else if(x_diff<=0 && y_diff>=0)
		return 0;
	return -1;
}
bool no_barrier=false;
bool barrier_check(uint16_t self,int x,int y,bool check)
{
    if(no_barrier)
        return false;
	bool ret= scene->scenemap.gettile(x/32,y/16,x%32?1:0,0).blocked;
	if(check)
		for(std::vector<EVENT_OBJECT>::iterator it=scene->sprites_begin;it<scene->sprites_end;it++)
			if(it-res::evtobjs.begin()!=self && it->status>1 && abs(it->pos_x-x)+abs(it->pos_y-y)*2<16){
				ret=ret||true;break;
			}
	return ret;
}
void NPC_walk_one_step(EVENT_OBJECT &obj,int speed)
{
	obj.pos_x+=direction_offs[obj.direction][0]/8*speed;
	obj.pos_y+=direction_offs[obj.direction][1]/8*speed;
	if(obj.frames>0)
		obj.curr_frame=(obj.curr_frame+1)%(obj.frames==3?4:obj.frames);
	else if(obj.frames_auto>0)
		obj.curr_frame=(obj.curr_frame+1)%obj.frames_auto;
}

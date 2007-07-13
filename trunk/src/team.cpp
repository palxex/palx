#include "internal.h"
#include "resource.h"
#include "game.h"
#include "scene.h"
#include <algorithm>

int step_off_x=16,step_off_y=8;
int coordinate_x_max=1696,coordinate_y_max=1840;
int x_scrn_offset=0xA0;
int y_scrn_offset=0x70;
int abstract_x_bak=0,abstract_y_bak=0;
int viewport_x_bak=0,viewport_y_bak=0;
int direction_offs[][2]={{-16,8},{-16,-8},{16,-8},{16,8}};
int this_step_frame=0;
int step_frame_follower=0,step_frame_leader=0;

std::vector<sprite_prim> mgos;
std::map<int,int> team_mgos;
std::map<int,int> npc_mgos;

int load_mgo(int id)
{
	bool decoded;
	uint8_t *buf=MGO.decode(id,decoded);
	if(!decoded){
		mgos.push_back(sprite_prim(id,buf));
		return mgos.end()-mgos.begin()-1;
	}else
		return std::find(mgos.begin(),mgos.end(),sprite_prim(id))-mgos.begin();
}
void load_team_mgo()
{
	team_mgos.clear();
	for(int i=0;i<=game->rpg.team_roles;i++)
	{
		int id=game->rpg.roles_properties.avator[game->rpg.team[i].role];
		team_mgos[i]=load_mgo(id);
	};
}
void load_NPC_mgo()
{
	npc_mgos.clear();
	for(std::vector<EVENT_OBJECT>::iterator i=scene->sprites_begin;i!=scene->sprites_end;i++)
		if(i->image)
			npc_mgos[i-scene->sprites_begin]=load_mgo(i->image);
}
void calc_trace_frames()
{
	this_step_frame=(this_step_frame+1)&3;
	if(this_step_frame & 1)
		step_frame_leader=(this_step_frame+1)/2,
		step_frame_follower=3-step_frame_leader;
	else
		step_frame_leader=step_frame_follower=0;
	/*
	memcpy(game->rpg.team_track+sizeof(RPG::track),game->rpg.team_track,4*sizeof(RPG::track));
	/*///
	game->rpg.team_track[4]=game->rpg.team_track[3];
	game->rpg.team_track[3]=game->rpg.team_track[2];
	game->rpg.team_track[2]=game->rpg.team_track[1];
	game->rpg.team_track[1]=game->rpg.team_track[0];
	//*/
}
void store_team_frame_data()
{
	game->rpg.team[0].frame=(game->rpg.roles_properties.walk_frames[game->rpg.team[0].role]==4? game->rpg.team_direction*4+this_step_frame : game->rpg.team_direction*3+step_frame_leader);
	game->rpg.team[0].x=x_scrn_offset;
	game->rpg.team[0].y=y_scrn_offset;
	game->rpg.team_track[0].direction=game->rpg.team_direction;
	game->rpg.team_track[0].x=abstract_x_bak;
	game->rpg.team_track[0].y=abstract_y_bak;

	for(int i=1;i<=game->rpg.team_roles;i++)
	{
		game->rpg.team[i].x=game->rpg.team_track[1].x-game->rpg.viewport_x;
		game->rpg.team[i].y=game->rpg.team_track[1].y-game->rpg.viewport_y;
		if(i==2)
		{
			game->rpg.team[i].y+=8;
			game->rpg.team[i].x+=(game->rpg.team_track[1].direction&1?-16:16);
		}else{
			game->rpg.team[i].x-=direction_offs[game->rpg.team_track[1].direction][0];
			game->rpg.team[i].y-=direction_offs[game->rpg.team_track[1].direction][1];
		}
		if(barrier_check(0,game->rpg.team[i].x+game->rpg.viewport_x,game->rpg.team[i].y+game->rpg.viewport_y))
			game->rpg.team[i].x=game->rpg.team_track[1].x-game->rpg.viewport_x,
			game->rpg.team[i].y=game->rpg.team_track[1].y-game->rpg.viewport_y;
		game->rpg.team[i].frame=(game->rpg.roles_properties.walk_frames[game->rpg.team[i].role]==4? game->rpg.team_track[2].direction*4+this_step_frame : game->rpg.team_track[2].direction*3+step_frame_follower);
	}
	for(int i=1;i<=game->rpg.team_followers;i++)
	{
		game->rpg.team[game->rpg.team_roles+i].x=game->rpg.team_track[i+game->rpg.team_roles].x-game->rpg.viewport_x;
		game->rpg.team[game->rpg.team_roles+i].y=game->rpg.team_track[i+game->rpg.team_roles].y-game->rpg.viewport_y;
		game->rpg.team[game->rpg.team_roles+i].frame=game->rpg.team_track[i+game->rpg.team_roles].direction*3+step_frame_follower;
	}
}
void team_walk_one_step()
{
	scene->team_pos.toXY().x=game->rpg.viewport_x+x_scrn_offset;
	scene->team_pos.toXY().y=game->rpg.viewport_y+y_scrn_offset;
	if(scene->team_pos.toXY().x!=abstract_x_bak || scene->team_pos.toXY().y!=abstract_y_bak)
		calc_trace_frames();
	else
		step_frame_follower=0,step_frame_leader=0,
		this_step_frame&=2,this_step_frame^=2;
	store_team_frame_data();
}
void stop_and_update_frame()
{
	game->rpg.team[0].frame=game->rpg.team_direction*(game->rpg.roles_properties.walk_frames[game->rpg.team[0].role]? game->rpg.roles_properties.walk_frames[game->rpg.team[0].role] : 3);
	for(int i=1;i<=game->rpg.team_roles;i++)
		game->rpg.team[i].frame=game->rpg.team_track[2].direction*(game->rpg.roles_properties.walk_frames[game->rpg.team[i].role]? game->rpg.roles_properties.walk_frames[game->rpg.team[i].role] : 3);
	for(int i=1;i<=game->rpg.team_followers;i++)
		game->rpg.team[game->rpg.team_roles+i].frame=game->rpg.team_track[i+2].direction*3;
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
bool barrier_check(uint16_t self,int x,int y)
{
	bool ret= scene->scenemap.gettile(x/32,y/16,x%32?1:0,0).blocked;
	for(std::vector<EVENT_OBJECT>::iterator it=scene->sprites_begin;it<scene->sprites_end;it++)
		if(it-game->evtobjs.begin()!=self && it->status>1 && abs(it->pos_x-x)+abs(it->pos_y-y)*2<16){
			ret=ret||true;break;
		}
	return ret;
}
void NPC_walk_one_step(uint16_t object,int speed)
{
	EVENT_OBJECT &obj=game->evtobjs[object];
	obj.pos_x+=direction_offs[obj.direction][0]/8*speed;
	obj.pos_y+=direction_offs[obj.direction][1]/8*speed;
	if(obj.frames>0)
		obj.curr_frame=(obj.curr_frame+1)%obj.frames;
	else if(obj.frames_auto>0)
		obj.curr_frame=(obj.curr_frame+1)%obj.frames_auto;
}

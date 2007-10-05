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
#include "structs.h"
#include "game.h"
#include "scene.h"
#include "timing.h"
#include "UI.h"

#include "stdlib.h"

#include <algorithm>

using namespace res;

extern int scale;
extern bool prelimit_OK=false;

extern int process_Battle(uint16_t,uint16_t);

BITMAP *backup=0;
void destroyit()
{
    destroy_bitmap(backup);
}
void restore_screen()
{
    blit(backup,screen,0,0,0,0,SCREEN_W,SCREEN_H);
    flag_pic_level=0;
}
void backup_screen()
{
    blit(screen,backup,0,0,0,0,SCREEN_W,SCREEN_H);
}

inline void sync_viewport()
{
    viewport_x_bak=rpg.viewport_x;
    viewport_y_bak=rpg.viewport_y;
    rpg.viewport_x=scene->team_pos.toXY().x-x_scrn_offset;
    rpg.viewport_y=scene->team_pos.toXY().y-y_scrn_offset;
}
inline void backup_position()
{
    abstract_x_bak=scene->team_pos.toXY().x;
    abstract_y_bak=scene->team_pos.toXY().y;
}
inline int16_t absdec(int16_t &s)
{
    int16_t s0=s;
    if (s>0)	s--;
    else if (s<0) s++;
    return s0;
}
typedef std::vector<EVENT_OBJECT>::iterator evt_obj;
void GameLoop_OneCycle(bool trigger)
{
    if (!mutex_can_change_palette)
    {
        if (trigger)
            for (evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&rpg.scene_id==map_toload;++iter)
                if (absdec(iter->vanish_time)==0)
                    if (iter->status>0 && iter->trigger_method>=4)
                    {
                        if (abs(scene->team_pos.x-iter->pos_x)+abs(scene->team_pos.y-iter->pos_y)*2<(iter->trigger_method-4)*32+16)// in the distance that can trigger
                        {
                            if (iter->frames)
                            {
                                clear_keybuf();
                                stop_and_update_frame();
                                iter->curr_frame=0;
                                iter->direction=calc_faceto(scene->team_pos.toXY().x-iter->pos_x,scene->team_pos.toXY().y-iter->pos_y);
                                redraw_everything();
                            }
                            uint16_t &triggerscript=iter->trigger_script;
                            triggerscript=process_script(triggerscript,(int16_t)(iter-evtobjs.begin()));
                        }
                    }
                    else if (iter->status<0) //&& in the screen
						if(iter->pos_x<rpg.viewport_x || iter->pos_x>rpg.viewport_x+320*scale || iter->pos_y<rpg.viewport_y || iter->pos_y>rpg.viewport_y+220*scale){
							iter->status=abs(iter->status);
							iter->curr_frame=0;
						}
        for (evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&rpg.scene_id==map_toload;++iter)
        {
            if (iter->status!=0)
                if (uint16_t &autoscript=iter->auto_script)
                    autoscript=process_autoscript(autoscript,(int16_t)(iter-evtobjs.begin()));
            if (iter->status==2 && iter->image>0 && trigger
                    && abs(iter->pos_x-scene->team_pos.toXY().x)+2*abs(iter->pos_y-scene->team_pos.toXY().y)<0xD)//&& beside role && face to it
            {
                //check barrier;this means, role status 2 means it takes place
                backup_position();
                for (int direction=(iter->direction+1)%4,i=0;i<4;direction=(direction+1)%4,i++)
                    if (!barrier_check(0,scene->team_pos.toXY().x+direction_offs[direction][0],scene->team_pos.toXY().y+direction_offs[direction][1]))
                    {
                        scene->team_pos.toXY().x+=direction_offs[direction][0];
                        scene->team_pos.toXY().y+=direction_offs[direction][1];
                        break;
                    }
                sync_viewport();
                scene->move_usable_screen();
            }
        }
		if(!--rpg.chasespeed_change_cycles)
			rpg.chase_range=1;
    }
}
void process_Explore()
{
    position poses[13];
    int x=scene->team_pos.toXY().x,y=scene->team_pos.toXY().y;
    int (&off)[2]=direction_offs[rpg.team_direction];
    poses[0]=position(x,y).toXYH();
    for (int i=0;i<4;i++)
    {
        poses[i*3+1]=position(x+off[0],y+off[1]).toXYH();
        poses[i*3+2]=position(x       ,y+off[1]*2).toXYH();
        poses[i*3+3]=position(x+off[0]*2,y       ).toXYH();
        x+=off[0];
        y+=off[1];
    }
    for (int i=0;i<13;i++)
        for (evt_obj iter=scene->sprites_begin;iter!=scene->sprites_end&&rpg.scene_id==map_toload;++iter)
            if (iter->status>0 && iter->trigger_method<=3
                    && position(iter->pos_x,iter->pos_y).toXYH()==poses[i].toXYH()
                    && iter->trigger_method*6-4>i)
            {
                if (iter->curr_frame < iter->frames*4 )
                {
                    iter->curr_frame=0;
                    iter->direction=(rpg.team_direction+2)&3;
                    for (int t=0;t<=rpg.team_roles;t++)//跟随者不用转？
                        rpg.team[t].frame=rpg.team_direction*3;
                    redraw_everything(0);
                }
                iter->trigger_script=process_script(iter->trigger_script,iter-evtobjs.begin());
                //my def
                clear_keybuf();
                rest(50);
                return;
            }
}
void clear_effective(int16_t p1,int16_t p2)
{
    redraw_everything();
}
extern void NPC_walk_one_step(EVENT_OBJECT &obj,int speed);
extern void add_goods_to_list(int,int);
void process_script_entry(uint16_t func,int16_t param[],uint16_t &id,int16_t object)
{
    //printf("%s\n",scr_desc(func,param).c_str());
    int16_t &param1=param[0],&param2=param[1],&param3=param[2];
    EVENT_OBJECT &obj=evtobjs[object];
#define curr_obj (param1<0?obj:evtobjs[param1])
    int role=rpg.team[(object>=0&&object<=4)?object:0].role;
    char addition[100];
    memset(addition,0,sizeof(addition));
    int npc_speed,role_speed;
    switch (func)
    {
    case 0xB:
        obj.direction=0;
        NPC_walk_one_step(obj,2);
        break;
    case 0xC:
        obj.direction=1;
        NPC_walk_one_step(obj,2);
        break;
    case 0xD:
        obj.direction=2;
        NPC_walk_one_step(obj,2);
        break;
    case 0xE:
        obj.direction=3;
        NPC_walk_one_step(obj,2);
        break;
    case 0xF:
        if (param1>=0)
            obj.direction=param1;
        if (param2>=0)
            obj.curr_frame=param2;
        break;
    case 0x10:
        npc_speed=3;
__walk_npc:
        {
            int16_t x_diff=obj.pos_x-(param1*32+param3*16),y_diff=obj.pos_y-(param2*16+param3*8);
            obj.direction=calc_faceto(-x_diff,-y_diff);
            if (abs(x_diff)<npc_speed*2 || abs(y_diff)<npc_speed)
            {
                obj.pos_x = param1*32+param3*16;
                obj.pos_y = param2*16+param3*8;
            }
            else
                NPC_walk_one_step(obj,npc_speed);

            //afterward check;MUST have,or will not match dospal exactly
            if (obj.pos_x==param1*32+param3*16 && obj.pos_y==param2*16+param3*8)
                obj.curr_frame=0;//printf(addition,"完成");
            else
            {
                //printf(addition,"当前X:%x,Y:%x  目的X:%x,Y:%x",obj.pos_x,obj.pos_y,(param1*32+param3*16),(param2*16+param3*8));
                --id;
            }
        }
        break;
    case 0x11:
        if ((object&1) != flag_parallel_mutex)
        {
            //printf(addition,"被堵塞 当前X:%x,Y:%x  目的X:%x,Y:%x",obj.pos_x,obj.pos_y,(param1*32+param3*16),(param2*16+param3*8));
            --id;
            break;
        }
        npc_speed=2;
        goto __walk_npc;
    case 0x12:
        curr_obj.pos_x=param2+scene->team_pos.toXY().x;
        curr_obj.pos_y=param3+scene->team_pos.toXY().y;
        break;
    case 0x13:
        evtobjs[param1].pos_x=param2;
        evtobjs[param1].pos_y=param3;
        break;
    case 0x14:
        obj.curr_frame=param1;
        obj.direction=0;
        break;
    case 0x15:
        rpg.team_direction=param1;
        rpg.team[param3].frame=param1*3+param2;
        break;
    case 0x16:
		if(param1==0)
			break;
        curr_obj.direction=param2;
        curr_obj.curr_frame=param3;
        break;
    case 0x17:
        //not implemented
        break;
    case 0x18:
        //not implemented
        break;
    case 0x19:
        rpg.role_prop_tables[param1][param3>0?param3-1:role]+=param2;
        break;
    case 0x1a:
        if (param3<=0)
            ;//battle time;
        else
            rpg.role_prop_tables[param1][param3-1]=param2;
        break;
    case 0x1b:
        for (int i=(param1?0:object);i<=(param1?rpg.team_roles:object);i++)
            if (rpg.roles_properties.HP[i]>0)
            {
                rpg.roles_properties.HP[i]+=param2;
                if (rpg.roles_properties.HP[i]<0)
                    rpg.roles_properties.HP[i]=0;
                if (rpg.roles_properties.HP[i]>rpg.roles_properties.HP_max[i])
                    rpg.roles_properties.HP[i]=rpg.roles_properties.HP_max[i];
            }
        break;
    case 0x1c:
        for (int i=(param1?0:object);i<=(param1?rpg.team_roles:object);i++)
            if (rpg.roles_properties.HP[i]>0)
            {
                rpg.roles_properties.MP[i]+=param2;
                if (rpg.roles_properties.MP[i]<0)
                    rpg.roles_properties.MP[i]=0;
                if (rpg.roles_properties.MP[i]>rpg.roles_properties.MP_max[i])
                    rpg.roles_properties.MP[i]=rpg.roles_properties.MP_max[i];
            }
        break;
    case 0x1d:
        for (int i=(param1?0:object);i<=(param1?rpg.team_roles:object);i++)
            if (rpg.roles_properties.HP[i]>0)
            {
                rpg.roles_properties.HP[i]+=param2;
                if (rpg.roles_properties.HP[i]<0)
                    rpg.roles_properties.HP[i]=0;
                if (rpg.roles_properties.HP[i]>rpg.roles_properties.HP_max[i])
                    rpg.roles_properties.HP[i]=rpg.roles_properties.HP_max[i];
                rpg.roles_properties.MP[i]+=param2;
                if (rpg.roles_properties.MP[i]<0)
                    rpg.roles_properties.MP[i]=0;
                if (rpg.roles_properties.MP[i]>rpg.roles_properties.MP_max[i])
                    rpg.roles_properties.MP[i]=rpg.roles_properties.MP_max[i];
            }
        break;
    case 0x1e:
        if (param1<0 && rpg.coins<-param1)
        {
            id=param2-1;
            break;
        }
        else
            rpg.coins+=param1;
        break;
    case 0x1f:
        compact_items();
        add_goods_to_list(param1,param2==0?1:param2);
        break;
    case 0x20:
        //not implemented
        break;
    case 0x21:
        //not implemented
        break;
    case 0x22:
        //not implemented
        break;
    case 0x23:
        for (int i=(param2?param2:0xB);i<=(param2?param2:0x10);i++)
            if(rpg.role_prop_tables[i][param1])
            {
                add_goods_to_list(rpg.role_prop_tables[i][param1],1);
                rpg.role_prop_tables[i][param1]=0;
            }
        break;
    case 0x24:
		if(param1==0)
			break;
        curr_obj.auto_script= param2;
        break;
    case 0x25:
		if(param1==0)
			break;
        curr_obj.trigger_script= param2;
        break;
    case 0x26:
        shop(param1);
        break;
    case 0x27:
        hockshop();
        break;
    case 0x28:
        //not implemented
        break;
    case 0x29:
        //not implemented
        break;
    case 0x2a:
        //not implemented
        break;
    case 0x2b:
        //not implemented
        break;
    case 0x2c:
        //not implemented
        break;
    case 0x2d:
        //not implemented
        break;
    case 0x2e:
        //not implemented
        break;
    case 0x2f:
        //not implemented
        break;
    case 0x30:
        //not implemented
        break;
    case 0x31:
        //not implemented
        break;
    case 0x32:
        if(flag_battling)
            id=param1-1;
        else
            id=param2-1;
        break;
    case 0x33:
        //not implemented
        break;
    case 0x34:
        //not implemented
        break;
    case 0x35:
        shake_times=param1;
        shake_grade=(param2?param2:4);
        break;
    case 0x36://Set RNG
        RNG.clear();
        RNG_num=param1;
        flag_to_load|=0x10;
        break;
    case 0x37:
        play_RNG(param1,param2>0?param2:999,param3>0?param3:16);
        break;
	case 0x38:
		if(scenes[rpg.scene_id].leave_script && !flag_battling)
			scenes[rpg.scene_id].leave_script = process_script(scenes[rpg.scene_id].leave_script,0);
		else{
			prelimit_OK=false;
			id = param1 -1;
		}
		break;
	case 0x39:
        //not implemented
		break;
	case 0x3a:
        //not implemented
		break;
    case 0x3b:
        frame_pos_flag=0;
        frame_text_x=0x50;
        frame_text_y=0x28;
        if (param1)
            glbvar_fontcolor=param1;
        break;
    case 0x3c:
        frame_pos_flag=1;
        if (param1)
        {
            if (param3)
            {
                flag_pic_level=param3;
                backup_screen();
            }
            sprite_prim().getsource(RGM.decode(param1,0)).getsprite(0)->blit_middle(screen,0x30*scale,0x37*scale);
        }
        dialog_x=(param1?0x50:0xC)*scale;
        dialog_y=8*scale;
        frame_text_x=(param1?0x60:0x2C)*scale;
        frame_text_y=0x1A*scale;
        if (param2)
            glbvar_fontcolor=param2;
        break;
    case 0x3d:
        frame_pos_flag=2;
        if (param1)
        {
            if (param3)
            {
                flag_pic_level=param3;
                backup_screen();
            }
            sprite_prim().getsource(RGM.decode(param1,0)).getsprite(0)->blit_middle(screen,0x10E*scale,0x90*scale);
        }
        dialog_x=(param1?4:0xC)*scale;
        dialog_y=0x6C*scale;
        frame_text_x=(param1?0x14:0x2C)*scale;
        frame_text_y=0x7E*scale;
        if (param2)
            glbvar_fontcolor=param2;
        break;
    case 0x3e:
        frame_pos_flag=10;
        frame_text_x=0xA0;
        frame_text_y=0x28;
        if (param1)
            glbvar_fontcolor=param1;
        break;
    case 0x3f:
        npc_speed=2;
        goto __ride;
    case 0x40:
		if(param1==0)
			break;
        curr_obj.trigger_method=param2;
        break;
	case 0x41:
		prelimit_OK=false;
		break;
	case 0x42:
        //not implemented
		break;
    case 0x43:
        if (param1)
        {
            if(rpg.music!=param1)
                rix->play(rpg.music=param1,param2);
        }
        else
            rix->stop();
        break;
    case 0x44:
        npc_speed=4;
__ride:
        {
            position dest(param1,param2,param3);
            int x_diff=dest.toXY().x-scene->team_pos.toXY().x,y_diff=dest.toXY().y-scene->team_pos.toXY().y;
            while (x_diff || y_diff)
            {
                int direction=calc_faceto(x_diff,y_diff);
                int x_off=npc_speed*2*(direction_offs[direction][0]/16),y_off=npc_speed*(direction_offs[direction][1]/8);
                backup_position();
                if (x_diff)
                    scene->team_pos.x+=x_off;
                if (y_diff)
                    scene->team_pos.y+=y_off;
                sync_viewport();
                obj.direction=direction;
                obj.pos_x+=x_off;
                obj.pos_y+=y_off;
                record_step();
                GameLoop_OneCycle(false);
                scene->move_usable_screen();
                redraw_everything();
                x_diff=dest.toXY().x-scene->team_pos.toXY().x,y_diff=dest.toXY().y-scene->team_pos.toXY().y;
            }
        }
        break;
    case 0x45:
        rpg.battle_music=param1;
        break;
    case 0x46:
        scene->team_pos.toXYH().x=param1;
        scene->team_pos.toXYH().y=param2;
        scene->team_pos.toXYH().h=param3;
        sync_viewport();
        scene->produce_one_screen();
        break;
    case 0x47:
        voc(VOC.decode(param1)).play();
        break;
	case 0x48: //lost script...
        //not implemented
		break;
    case 0x49:
        evtobjs[param1!=-1?param1:object].status=param2;
        break;
    case 0x4a:
        rpg.battlefield=param1;
        break;
	case 0x4b:
        //not implemented
		break;
	case 0x4c://gogogo
		{
			npc_speed=0;
			int guard_field=(param1?param1:8);
			int chase_speed=(param2?param2:4);
			if(rpg.chase_range){
				int x_off=obj.pos_x-scene->team_pos.toXY().x,y_off=obj.pos_y-scene->team_pos.toXY().y;
				position pos(obj.pos_x,obj.pos_y);pos.toXYH();pos.toXY();
				if(abs(x_off)+2*abs(y_off)<guard_field*32*rpg.chase_range){
					DIRECTION d=(DIRECTION)calc_faceto(x_off?-x_off:(rnd0()?-1:1),y_off?-y_off:(rnd0()?-1:1));
					if(param3)
						obj.direction=d,
						npc_speed=chase_speed;
					else if(!barrier_check(object,obj.pos_x+direction_offs[d][0],obj.pos_y+direction_offs[d][1])){
						if(barrier_check(0,obj.pos_x,obj.pos_y,false)){
							//allegro_message("Hei,you are on the volcano(%x,%x), NPC #%X!",obj.pos_x,obj.pos_y,object);
							obj.status=0;
						}
						obj.direction=d;
						npc_speed=chase_speed;
					}else
						obj.pos_x=pos.x,obj.pos_y=pos.y;
					if(!param3)
						for(int D=WEST;D<=SOUTH;D++)
							if(barrier_check(0,obj.pos_x+=direction_offs[D][0]/4,obj.pos_y+=direction_offs[D][1]/4,false))
								obj.pos_x=pos.x,obj.pos_y=pos.y;
				}
			}else
				obj.direction=(obj.direction+flag_parallel_mutex)&3;
			NPC_walk_one_step(obj,npc_speed);
		}
		break;
    case 0x4d:
        wait_for_key();
        break;
	case 0x4e:
        flag_to_load|=0x20;
		rpg.scene_id=0;
		break;
	case 0x4f:
        //not implemented
		break;
    case 0x50:
        pal_fade_out(param1==0?1:param1);
        break;
    case 0x51:
        pal_fade_in(param1==0?1:param1);
        break;
    case 0x52:
        obj.status=-obj.status;
        obj.vanish_time=(param1?param1:0x320);
        break;
    case 0x53:
        rpg.palette_offset=0;
        break;
    case 0x54:
        rpg.palette_offset=0x180;
        break;
	case 0x55:
        learnmagic(false,param1,object);
		break;
	case 0x56:
        //not implemented
		break;
	case 0x57:
        //not implemented
		break;
	case 0x58:
        //not implemented
		break;
    case 0x59:
        if (param1>0 && rpg.scene_id!=param1)
        {
            map_toload=param1;
            flag_to_load|=0xC;
            rpg.layer=0;
        }
        break;
	case 0x62:
		rpg.chasespeed_change_cycles=param1;
		rpg.chase_range=0;
		break;
	case 0x63:
		rpg.chasespeed_change_cycles=param1;
		rpg.chase_range=3;
    case 0x65:
        rpg.roles_properties.avator[param1]=param2;
        if (!flag_battling && param3)
            load_team_mgo();
        break;
    case 0x6c:
        curr_obj.pos_x+=param2;
        curr_obj.pos_y+=param3;
        NPC_walk_one_step(curr_obj,0);
        break;
    case 0x6d:
        if (param1)
        {
            if (param2)
                scenes[param1].enter_script=param2;
            if (param3)
                scenes[param1].leave_script=param3;
            if (!param2 && !param3)
                scenes[param1].enter_script=0,
                                                  scenes[param1].leave_script=0;
        }
        break;
    case 0x6e:
        backup_position();
        sync_viewport();
        rpg.viewport_x+=param1;
        rpg.viewport_y+=param2;
        rpg.layer=param3*8;
        if (param1||param2)
        {
            team_walk_one_step();
            scene->move_usable_screen();
        }
        break;
    case 0x6f:
        if (evtobjs[param1].status==param2)
        {
            obj.status=param2;
            //printf(addition,"成功");
        }//else
        //printf(addition,"失败");
        break;
    case 0x70:
        role_speed=2;
__walk_role:
        {
            int16_t x_diff,y_diff;
            while ((x_diff=scene->team_pos.x-(param1*32+param3*16)) && (y_diff=scene->team_pos.y-(param2*16+param3*8)))
            {
                backup_position();
                rpg.team_direction=calc_faceto(-x_diff,-y_diff);
                scene->team_pos.toXY().x += role_speed*(x_diff<0 ? 2 : -2);
                scene->team_pos.toXY().y += role_speed*(y_diff<0 ? 1 : -1);
                rpg.team_direction=calc_faceto(scene->team_pos.x-abstract_x_bak,scene->team_pos.y-abstract_y_bak);
                sync_viewport();
                team_walk_one_step();
                GameLoop_OneCycle(false);
                scene->move_usable_screen();
                redraw_everything();
            }
        }
        break;
    case 0x73:
        clear_effective(param1?param1:1,param2);
        break;
    case 0x75:
        rpg.team[0].role=(param1-1<0?0:param1-1);
        rpg.team[1].role=(param2-1<0?0:param2-1);
        rpg.team[2].role=(param3-1<0?0:param3-1);
        rpg.team_roles=(param1?1:0)+(param2?1:0)+(param3?1:0)-1;
        load_team_mgo();
        //call    setup_our_team_data_things
        store_team_frame_data();
        break;
    case 0x76:
        show_fbp(param1,param2);
        break;
    case 0x77:
		rix->stop(param1>0?param1:1);
        if (!param2)
            ;//CD stop
		if(!flag_battling)
			rpg.music=0;
        break;
    case 0x78:
        flag_battling=false;
        Load_Data();
        break;
    case 0x79:
    {
        bool in=false;
        for (int i=0;i<=rpg.team_roles;i++)
            if (rpg.roles_properties.name[i]==param1)
                in=true,i=5;
        if (in)
            id=param2-1;
    }
    break;
    case 0x7a:
        role_speed=4;
        goto __walk_role;
    case 0x7b:
        role_speed=8;
        goto __walk_role;
    case 0x7c:
        if (!flag_parallel_mutex)
        {
            --id;
            break;
        }
        npc_speed=4;
        goto __walk_npc;
    case 0x7d:
        curr_obj.pos_x+=param2;
        curr_obj.pos_y+=param3;
        break;
	case 0x7e:
		curr_obj.layer=param2;
		break;
	case 0x7f:
		{
			if (param1==0 && param2==0 && param3==-1)
			{
				x_scrn_offset=0xA0*scale;
				y_scrn_offset=0x70*scale;
				rpg.viewport_x=scene->team_pos.toXY().x-x_scrn_offset;
				rpg.viewport_y=scene->team_pos.toXY().y-y_scrn_offset;
			}
			int t=param3;
			for (int i=0;i<(t>0?t:1);i++)
			{
				int b1=x_scrn_offset,b2=y_scrn_offset;
				viewport_x_bak=rpg.viewport_x;
				viewport_y_bak=rpg.viewport_y;
				if (!param1 && !param2 && !param3)
				{
					x_scrn_offset=0xA0*scale;
					y_scrn_offset=0x70*scale;
					rpg.viewport_x=scene->team_pos.toXY().x-x_scrn_offset;
					rpg.viewport_y=scene->team_pos.toXY().y-y_scrn_offset;
					scene->produce_one_screen();
					t=-1;
				}
				else
				{
					if (param3<0)
					{
						rpg.viewport_x=param1*32-0xA0*scale;
						rpg.viewport_y=param2*16-0x70*scale;
						scene->produce_one_screen();
					}
					else
					{
						rpg.viewport_x+=param1;
						rpg.viewport_y+=param2;
					}
					x_scrn_offset=scene->team_pos.toXY().x-rpg.viewport_x;
					y_scrn_offset=scene->team_pos.toXY().y-rpg.viewport_y;
				}
				rpg.team[0].x=x_scrn_offset;
				rpg.team[0].y=y_scrn_offset;
				for (int t=1;t<=rpg.team_roles;t++)
				{
					rpg.team[t].x+=(x_scrn_offset-b1);
					rpg.team[t].y+=(y_scrn_offset-b2);
				}
				GameLoop_OneCycle(false);
				if (param3>=0)
					scene->move_usable_screen();
				redraw_everything();
			}
		}
        break;
    case 0x80://todo:
        GameLoop_OneCycle(false);
        redraw_everything();
        mutex_can_change_palette=false;
        break;
    case 0x81:
        prelimit_OK=false;
        if (param1>=scene->sprites_begin-evtobjs.begin() && param1<scene->sprites_end-evtobjs.begin() && curr_obj.status>0
            && abs(evtobjs[param1].pos_x-direction_offs[rpg.team_direction][0]-scene->team_pos.toXY().x)+abs(evtobjs[param1].pos_y-direction_offs[rpg.team_direction][1]-scene->team_pos.toXY().y)*2<param2*32+16)
        {
            if(param2>0)
                evtobjs[param1].trigger_method=5+param2;
            prelimit_OK=true;
        }
        else
            id=param3-1;
        break;
    case 0x82:
        npc_speed=8;
        goto __walk_npc;
    case 0x85:
        wait(param1*10);
        break;
    case 0x87:
        NPC_walk_one_step(obj,0);
        break;
    case 0x8b:
        pat.read(param1);
        rpg.palette_offset=0;
        if (mutex_can_change_palette==0)
            pat.set(rpg.palette_offset);
        break;
    case 0x8e:
        restore_screen();
        break;
    case 0x92:
        //clear_effective(1,0x41);
        break;
    case 0x93:
        break;
	case 0x94:
		if(evtobjs[param1].status == param2)
			id = param3 -1;
		break;
    case 0x97:
        npc_speed=8;
        goto __ride;
    case 0x98:
        load_team_mgo();
        store_team_frame_data();
        break;
	case 0x9a:
		for(int i=param1;i<=param2;i++)
			evtobjs[i].status=param3;
		break;
    case 0x9b:
        scene->produce_one_screen();
        break;
    case 0x9d:
        //clear_effective(2,0x4E);
        //clear_effective(1,0x2A);
        break;
    case 0x9e:
        //clear_effective(1,0x4E);
        //clear_effective(1,0x2A);
        break;
    case 0x9f:
        //clear_effective(1,0x48);
        break;
    case 0xa0:
        clear_bitmap(screen);
        throw;
        break;
	case 0xa3:
		rix->play(param2);
		break;
    case 0xa5:
        break;
    case 0xa6:
        backup_screen();
        break;
    }
}

cut_msg_impl msges("m.msg");
cut_msg_impl objs("word.dat");
uint16_t process_script(uint16_t id,int16_t object)
{
    static int _t_=atexit(destroyit);
    if (!backup)
        backup=create_bitmap(SCREEN_W,SCREEN_H);
    static char *msg,colon[3];
    static int i=sprintf(colon,msges(0xc94,0xc96));
	prelimit_OK=true;
    EVENT_OBJECT &obj=evtobjs[object];
    uint16_t next_id=id;
    current_dialog_lines = 0;
    glbvar_fontcolor  = 0x4F;
    font_color_yellow = 0x2D;
    font_color_red    = 0x1A;
    font_color_cyan   = 0x8D;
    font_color_cyan_1 = 0x8C;
    frame_pos_flag = 1;
    dialog_x = 12;
    dialog_y = 8;
    frame_text_x = 0x2C;
    frame_text_y = 0x1A;
    bool ok=true;
    while (id && ok)
    {
        SCRIPT &curr=scripts[id];
		const int16_t &param1=curr.param[0],&param2=curr.param[1],&param3=curr.param[2];
        //printf("独占脚本%04x:%04x %04x %04x %04x ;",id,curr.func,(uint16_t)param1,(uint16_t)param2,(uint16_t)param3);
        switch (curr.func)
        {
        case 0:
            id = next_id-1;
            //printf("停止执行\n");
            ok=false;
            break;
        case -1:
            //printf("显示对话 `%s`\n",cut_msg(rpg.msgs[param1],rpg.msgs[param1+1]));
            if (current_dialog_lines>3)
            {
                show_wait_icon();
                current_dialog_lines=0;
                restore_screen();
            }
            else if (current_dialog_lines==0 && flag_pic_level==0)
                backup_screen();
            msg=msges(msg_idxes[param1],msg_idxes[param1+1]);
            if (frame_pos_flag==10)
            {
                frame_text_x-=strlen(msg)/2*8;
                single_dialog(frame_text_x,frame_text_y,strlen(msg)/2,bitmap(screen)).to_screen();
                dialog_string(msg,frame_text_x+10,frame_text_y+10,0,false);
                wait_key(140);
            }
            else if (current_dialog_lines==0 && memcmp(msg+strlen(msg)-2,&colon,2)==0)
                dialog_string(msg,dialog_x,dialog_y,0x8C,true);
            else
                draw_oneline_m_text(msg,frame_text_x,frame_text_y+(current_dialog_lines++)*(frame_pos_flag?16:18));
            break;
        case 1:
            //printf("停止执行，将调用地址替换为下一条命令\n");
            ok=false;
            break;
        case 2:
            //printf("停止执行，将调用地址替换为脚本%x:",param1);
            if (param2==0)
            {
                //printf("成功\n");
                id = param1-1;
                ok=false;
                break;
            }
            else if (++obj.scr_jmp_count<param2)
            {
                //printf("第%x次成功\n",obj.scr_jmp_count);
                id = param1-1;
                ok=false;
                break;
            }
            else
            {
                //printf("过期失效\n");
                obj.scr_jmp_count = 0;
            }
            break;
        case 3:
            //printf("跳转到脚本%x:",param1);
            if (param2==0)
            {
                //printf("成功\n");
                id = param1;
                continue;
            }
            else if (++obj.scr_jmp_count<param2)
            {
                //printf("第%x次成功\n",obj.scr_jmp_count);
                id = param1;
                continue;
            }
            else
            {
                //printf("过期失效\n",param1);
                obj.scr_jmp_count = 0;
            }
            break;
        case 4:
            //printf("调用脚本%x %x\n",param1,param2);
            process_script(param1,param2?param2:object);
            break;
        case 5:
            //printf("清屏 方式%x 延迟%x,更新角色信息:%s\n",param1,param2,param3?"是":"否");
            if (current_dialog_lines>0)
                show_wait_icon(),current_dialog_lines=0;
            if (flag_pic_level==0)
            {
                if (param3)
                    stop_and_update_frame();
                redraw_everything();
            }
            else
                restore_screen();
            break;
        case 6:
            //printf("以%d%%几率跳转到脚本%x:",param1,param2);
            if (rnd0()*100<param1)
            {
                //printf("成功\n");
				if(!param2){
					ok=false;
					id--;
					break;
				}
                id = param2;
                continue;
            }
            else
                //printf("失败\n");
                break;
        case 7:
            //printf("开战 第%x组敌人 胜利脚本%x 逃跑脚本%x\n",param1,param2,param3);
            if (current_dialog_lines>0)
                show_wait_icon();
            i=process_Battle(param1,param3);
            if ( param2 && i == 1)
            {
                id = param2;
                continue;
            }
            if ( param3 && i == 2)
            {
                id = param3;
                continue;
            }
            break;
        case 8:
            next_id = id+1;
            //printf("将调用地址替换为脚本%x\n",next_id);
            break;
        case 9:
            //printf("空闲%x循环\n",param1);
            if (current_dialog_lines>0)
                show_wait_icon(),current_dialog_lines=0;
            for (int cycle=1;cycle<=(param1?param1:1);++cycle)
            {
                //printf("第%x循环:\n",cycle);
                if (param3)
                    calc_trace_frames(),
                    store_team_frame_data();
                GameLoop_OneCycle(param2!=0);
                redraw_everything();
            }
            break;
        case 0xA:
            //printf("选择:选否则继续(y/n)");
            current_dialog_lines=0;
            char sele;
            scanf("%c",&sele);
            if (sele=='y')
            {
                id = param1;
                //printf("跳转\n");
                continue;
            }
            //printf("继续\n");
            break;
        default:
            if (current_dialog_lines>0)
                show_wait_icon();
            process_script_entry(curr.func,curr.param,id,object);
        }
        ++id;
    }
exit:
    if (current_dialog_lines>0)
        show_wait_icon();
    return id;
}
uint16_t process_autoscript(uint16_t id,int16_t object)
{
    SCRIPT &curr=scripts[id];
    EVENT_OBJECT &obj=evtobjs[object];
    const int16_t &param1=curr.param[0],&param2=curr.param[1],&param3=curr.param[2];
    //printf("触发场景:%s,触发对象:(%04x,%s)\n",scr_desc.getdesc("SceneID",scene_curr).c_str(),object,scr_desc.getdesc("ObjectID",object).c_str());
    //printf("自动脚本%04x:%04x %04x %04x %04x ;",id,curr.func,(uint16_t)param1,(uint16_t)param2,(uint16_t)param3);
    switch (curr.func)
    {
    case 0:
        //printf("停止执行\n");
        id--;
    case 2:
        //printf("停止执行，将调用地址替换为脚本%x:",param1);
        if (param2==0)
        {
            //printf("成功\n");
            id = param1 - 1;
        }
        else if (++obj.scr_jmp_count_auto<param2)
        {
            //printf("第%x次成功\n",obj.scr_jmp_count_auto);
            id = param1 - 1;
        }
        else
        {
            //printf("失败\n");
            obj.scr_jmp_count_auto = 0;
        }
        break;
    case 3:
        //printf("跳转到脚本%x",param1);
        if (param2==0)
            //printf("成功\n");
            id = process_autoscript(param1,object) - 1;
        else if (++obj.scr_jmp_count_auto<param2)
            //printf("第%x次成功\n",obj.scr_jmp_count_auto);
            id = process_autoscript(param1,object) - 1;
        else
            //printf("失败\n");
            obj.scr_jmp_count_auto = 0;
        break;
    case 4:
        //printf("调用脚本%x %x\n",param1,param2);
        process_script(param1,param2?param2:object);
        break;
    case 6:
        //printf("以%d%%几率跳转到脚本%x:",param1,param2);
        if (rnd0()*100<param1 && param2)
            //printf("成功\n");
            id = process_autoscript(param2,object) - 1;
        break;
    case 9:
        //printf("自动脚本空闲第%x循环:\n",++obj.scr_jmp_count_auto);
        if (++obj.scr_jmp_count_auto<param1)
            id--;
        else
            obj.scr_jmp_count_auto = 0;
        break;
    default:
        process_script_entry(curr.func,curr.param,id,object);
    }
    return id+1;
}

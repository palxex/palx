#include "internal.h"

void redraw_everything()
{
	//lots of lots stuff;only implement the reverse of parallel mutex
	flag_parallel_mutex=!flag_parallel_mutex;
}
void clear_effective(int16_t p1,int16_t p2)
{
	redraw_everything();
}
uint16_t process_script_entry(uint16_t func,int16_t param[],uint16_t id,int16_t object)
{
	//printf("%s\n",scr_desc(func,param).c_str());
	EVENT_OBJECT &obj=game->rpg.evtobjs[object];
	const int16_t &param1=param[0],&param2=param[1],&param3=param[2];
	char addition[100];memset(addition,0,sizeof(addition));
	int npc_speed,role_speed;
	switch(func){
		case 0x10:
			npc_speed=3;
__walk_npc:
			{
				int16_t x_diff=obj.pos_x-(param1*32+param3*16),y_diff=obj.pos_y-(param2*16+param3*8);
				if(abs(x_diff)<npc_speed*2 && abs(y_diff)<npc_speed){
					obj.pos_x = param1*32+param3*16;
					obj.pos_y = param2*16+param3*8;
				}else{
					obj.pos_x += npc_speed*(x_diff<0 ? 2 : -2);
					obj.pos_y += npc_speed*(y_diff<0 ? 1 : -1);
				}

				//afterward check;MUST have,or will not match dospal exactly
				/*if(obj.pos_x==param1*32+param3*16 && obj.pos_y==param2*16+param3*8)
					//printf(addition,"完成");
				else{
					//printf(addition,"当前X:%x,Y:%x  目的X:%x,Y:%x",obj.pos_x,obj.pos_y,(param1*32+param3*16),(param2*16+param3*8));
					--id;
				}*/
			}
			break;
		case 0x11:
			if((object&1) != flag_parallel_mutex){
				//printf(addition,"被堵塞 当前X:%x,Y:%x  目的X:%x,Y:%x",obj.pos_x,obj.pos_y,(param1*32+param3*16),(param2*16+param3*8));
				--id;
				break;
			}
			npc_speed=2;
			goto __walk_npc;
		case 0x24:
			if(param1)
				(param1>0 ? game->rpg.evtobjs[param1] : obj).auto_script= param2;
			break;
		case 0x25:
			if(param1)
				(param1>0 ? game->rpg.evtobjs[param1] : obj).trigger_script= param2;
			break;
		case 0x40:
			break;
		case 0x43:
			game->rpg.music=param1;
			break;
		case 0x44:
			GameLoop_OneCycle(false);
			//redraw_everything();
			break;
		case 0x45:
			game->rpg.battle_music=param1;
			break;
		case 0x46:
			scene->team_pos.x=param1*32+param3*16;
			scene->team_pos.y=param2*16+param3*8;
			break;
		case 0x49:
			(game->rpg.evtobjs+(param1!=-1?param1:object))->status=param2;
			break;
		case 0x59:
			scene->toload=param1;
			flag_to_load|=0xC;
			break;
		case 0x6c:			
			if(param1){
				(param1>0 ? game->rpg.evtobjs[param1] : obj).pos_x += param2;
				(param1>0 ? game->rpg.evtobjs[param1] : obj).pos_y += param3;
			}
			break;
		case 0x6f:
			if(game->rpg.evtobjs[param1].status==param2){
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
				while((x_diff=scene->team_pos.x-(param1*32+param3*16)) && (y_diff=scene->team_pos.y-(param2*16+param3*8))){
					scene->team_pos.x += role_speed*(x_diff<0 ? 2 : -2);
					scene->team_pos.y += role_speed*(y_diff<0 ? 1 : -1);
					GameLoop_OneCycle(false);
					redraw_everything();
				}
			}
			break;
		case 0x73:
			//clear_effective(param1,param2);
			break;
		case 0x7a:
			role_speed=4;
			goto __walk_role;
		case 0x7b:
			role_speed=8;
			goto __walk_role;
		case 0x7c:
			if(!flag_parallel_mutex){
				--id;
				break;
			}
			npc_speed=4;
			goto __walk_npc;
		case 0x7f:
			//GameLoop_OneCycle(false);
			//redraw_everything();
			break;
		case 0x80:
			//GameLoop_OneCycle(false);
			//redraw_everything();
			break;
		case 0x82:
			npc_speed=8;
			goto __walk_npc;
		case 0x92:
			//clear_effective(1,0x41);
			break;
		case 0x93:
			//GameLoop_OneCycle(false);
			//redraw_everything();
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
	}
	return id;
}

uint16_t process_script(uint16_t id,int16_t object)
{
	EVENT_OBJECT &obj=game->rpg.evtobjs[object];
	uint16_t next_id=id;
	while(id)
	{
		SCRIPT &curr=game->scripts[id];
		const int16_t &param1=curr.param[0],&param2=curr.param[1],&param3=curr.param[2];
		//printf("独占脚本%04x:%04x %04x %04x %04x ;",id,curr.func,(uint16_t)param1,(uint16_t)curr.param[1],(uint16_t)curr.param[2]);
		switch(curr.func)
		{
			case 0:
				id = next_id;
				//printf("停止执行\n");
				return id;
			case 0xffff:
				//printf("显示对话 `%s`\n",cut_msg(game->rpg.msgs[param1],game->rpg.msgs[param1+1]));
				break;
			case 1:
				//printf("停止执行，将调用地址替换为下一条命令\n");
				return id+1;
			case 2:
				//printf("停止执行，将调用地址替换为脚本%x:",param1);
				if(curr.param[1]==0){
					//printf("成功\n");
					return param1;
				}else if(obj.scr_jmp_count++<curr.param[1]){
					//printf("第%x次成功\n",obj.scr_jmp_count);
					return param1;
				}else{
					//printf("过期失效\n");
					obj.scr_jmp_count = 0;
				}
				break;
			case 3:
				//printf("跳转到脚本%x:",param1);
				if(curr.param[1]==0){
					//printf("成功\n");
					id = param1;
					continue;
				}else if(obj.scr_jmp_count++<curr.param[1]){
					//printf("第%x次成功\n",obj.scr_jmp_count);
					id = param1;
					continue;
				}else{
					//printf("过期失效\n",param1);
					obj.scr_jmp_count = 0;
				}
				break;
			case 4:
				//printf("调用脚本%x %x\n",param1,curr.param[1]);
				process_script(param1,curr.param[1]?curr.param[1]:object);
				break;
			case 5:
				//printf("清屏 方式%x 延迟%x,更新角色信息:%s\n",param1,curr.param[1],curr.param[2]?"是":"否");
				//总算有一个不用实现的了……Oh no!
				redraw_everything();
				break;
			case 6:
				//时间关系，不再模拟QB7的随机函数				
				//printf("以%d%%几率跳转到脚本%x:",param1,curr.param[1]);
				if(rnd0()*100<param1){
					//printf("成功\n");
					id = curr.param[1];
					continue;
				}else
					//printf("失败\n");
				break;
			case 7:
				//printf("开战 第%x组敌人 胜利脚本%x 逃跑脚本%x\n",param1,curr.param[1],curr.param[2]);
				break;
			case 8:
				next_id = id+1;
				//printf("将调用地址替换为脚本%x\n",next_id);
				break;
			case 9:
				//printf("空闲%x循环\n",param1);
				for(int cycle=1;cycle<=(param1?param1:1);++cycle){
					//printf("第%x循环:\n",cycle);
					GameLoop_OneCycle(curr.param[1]!=0);
					redraw_everything();
				}
				break;
			case 0xA:
				//printf("选择:选否则继续(y/n)");
				char sele;
				scanf("%c",&sele);
				if(sele=='y'){
					id = param1;
					//printf("跳转\n");
					continue;
				}
				//printf("继续\n");
				break;
			default:
				id = process_script_entry(curr.func,curr.param,id,object);
		}
		++id;
	}
	return id;
}
uint16_t process_autoscript(uint16_t id,int16_t object)
{
	if(id==0)
		return 0;
	SCRIPT &curr=game->scripts[id];
	EVENT_OBJECT &obj=game->rpg.evtobjs[object];
	const int16_t &param1=curr.param[0],&param2=curr.param[1],&param3=curr.param[2];
	//printf("触发场景:%s,触发对象:(%04x,%s)\n",scr_desc.getdesc("SceneID",scene_curr).c_str(),object,scr_desc.getdesc("ObjectID",object).c_str());
	//printf("自动脚本%04x:%04x %04x %04x %04x ;",id,curr.func,(uint16_t)param1,(uint16_t)param2,(uint16_t)param3);
	switch(curr.func){
		case 0:
			//printf("停止执行\n");
			return id;
		case 2:
			//printf("停止执行，将调用地址替换为脚本%x:",param1);
			if(param2==0){
				//printf("成功\n");
				return id = param1;
			}else if(obj.scr_jmp_count_auto++<param2){
				//printf("第%x次成功\n",obj.scr_jmp_count_auto);
				return id = param1;
			}else{
				//printf("失败\n");
				obj.scr_jmp_count_auto = 0;
			}
			break;
		case 3:
			//printf("跳转到脚本%x",param1);
			if(param2==0){
				//printf("成功\n");
				id = param1;
				process_autoscript(id,object);
			}else if(obj.scr_jmp_count_auto++<param2){
				//printf("第%x次成功\n",obj.scr_jmp_count_auto);
				id = param1;
				process_autoscript(id,object);
			}else{
				//printf("失败\n");
				obj.scr_jmp_count_auto = 0;
			}
			break;
		case 4:
			//printf("调用脚本%x %x\n",param1,param2);
			process_script(param1,param2?param2:object);
			break;
		case 6:
			//时间关系，不再模拟QB7的随机函数				
			//printf("以%d%%几率跳转到脚本%x:",param1,param2);
			if(rnd0()*100<param1){
				//printf("成功\n");
				id = param2;
				process_autoscript(id,object);
			}else
				//printf("失败\n");
			break;
		case 9:
			//printf("自动脚本空闲第%x循环:\n",++obj.scr_jmp_count_auto);
			if(obj.scr_jmp_count_auto<param1)
				return id;
			else
				obj.scr_jmp_count_auto = 0;
			break;
		default:
			id = process_script_entry(curr.func,curr.param,id,object);
	}
	return id+1;
}
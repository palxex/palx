#include "scene.h"
#include "internal.h"
#include "game.h"

inline sprite &map::getsprite(int x,int y,int h,int l,uint8_t *src,bool throu,int layer)
{
	tile &t=sprites[x][y][h][l];
	t.image=boost::shared_ptr<sprite>(new sprite(src));
	t.blocked=throu;
	t.layer=layer;
	return *t.image.get();
}
inline tile &map::gettile(int x,int y,int h,int l)
{
	return sprites[x][y][h][l];
}
map::map():scene_map(0,screen->w,screen->h),sprites(boost::extents[0x40][0x80][2][2])
{}
int t=-1;
void map::change(int p){
	t=p;
}
inline void map::make_tile(uint8_t *_buf,int x,int y,int h,int vx,int vy,BITMAP *dest)
{
	if(x<0||y<0||h<0||x>0x40-1||y>0x80-1||h>1) return;
	int index=(_buf[1]&0x10)<<4|_buf[0],index2=(_buf[3]&0x10)<<4|_buf[2];
	bool throu=(_buf[1]&0x20)?true:false,throu2=(_buf[3]&0x20)?true:false;
	int layer=_buf[1]&0xf,layer2=_buf[3]&0xf;
	getsprite(x,y,h,0,GOP.decode(t,index),throu,layer).blit_to(dest,x*32+h*16-16-vx,y*16+h*8-8-vy+gettile(x,y,h,0).layer*8,gettile(x,y,h,0).layer*8);
	if(index2)
		getsprite(x,y,h,1,GOP.decode(t,index2-1),throu,layer).blit_to(dest,x*32+h*16-16-vx,y*16+h*8-8-vy+gettile(x,y,h,1).layer*8+1,gettile(x,y,h,1).layer*8+1);
}
void map::make_onescreen(BITMAP *dest,int source_x,int source_y,int dest_x,int dest_y)
{
	uint8_t *mapbuf=MAP.decode(t,0);
	for(int y=source_y/16;y<dest_y/16+1;y++)
		for(int x=source_x/32;x<dest_x/32+1;x++)
			for(int h=0;h<2;h++)
				make_tile(mapbuf+y*0x200+x*8+h*4,x,y,h,source_x,source_y,bmp);
	blit(bmp,dest,source_x-game->rpg.viewport_x,source_y-game->rpg.viewport_y,source_x-game->rpg.viewport_x,source_y-game->rpg.viewport_y,dest_x-source_x,dest_y-source_y);
}
void map::blit_to(BITMAP *dest,int sx,int sy,int dx,int dy)
{
	bitmap::blit_to(dest,sx,sy,dx,dy);
	blit(bmp,bmp,sx,sy,dx,dy,bmp->w,bmp->h);
}

Scene::Scene():scene_buf(create_bitmap(screen->w,screen->h)),team_pos(game->rpg.viewport_x+x_scrn_offset,game->rpg.viewport_y+y_scrn_offset)
{}
Scene::~Scene()
{}
void Scene::clear_scanlines()
{
}
void Scene::clear_active()
{
	active_list.swap(s_list());
}
void Scene::calc_team_walking(int key)
{
	int16_t &direction=game->rpg.team_direction;
	abstract_x_bak=team_pos.toXY().x;
	abstract_y_bak=team_pos.toXY().y;
	viewport_x_bak=game->rpg.viewport_x;
	viewport_y_bak=game->rpg.viewport_y;
	if(key>=VK_DOWN && key<=VK_RIGHT){
		direction=key-3;
		position target=team_pos+position(direction_offs[direction][0],direction_offs[direction][1]);
		if(!scenemap.gettile(target.toXYH().x,target.toXYH().y,target.toXYH().h,0).blocked&&
			target.toXY().x>=0 && target.toXY().x<=coordinate_x_max+x_scrn_offset &&
			target.toXY().y>=0 && target.toXY().y<=coordinate_y_max+y_scrn_offset)
		{
			game->rpg.viewport_x+=direction_offs[direction][0];
			game->rpg.viewport_y+=direction_offs[direction][1];
			team_pos.toXY()=target;
			team_walk_one_step();
			return;
		}
	}
	stop_and_update_frame();
}
void Scene::our_team_setdraw()
{
	//for(std::vector<sprite_prim *>::iterator it=team_prims.begin();it!=team_prims.end();it++)
	//	active_list.push_back((*it)->getsprite(game->rpg.team[it-team_prims.begin()].frame));
	for(int i=0;i<=game->rpg.team_roles+game->rpg.team_followers;i++){
		sprite *it=mgos[team_mgos[i]].getsprite(game->rpg.team[i].frame);
		it->x=game->rpg.team[i].x-it->width/2;
		it->y=game->rpg.team[i].y+game->rpg.layer+10;
		it->l=game->rpg.layer+6;
		active_list.push_back(it);
	}
}
void Scene::visible_NPC_movment_setdraw()
{
	int t=0;
	for(std::vector<EVENT_OBJECT>::iterator i=sprites_begin;i!=sprites_end;i++,t++)
		if(i->pos_x>game->rpg.viewport_x && i->pos_x<game->rpg.viewport_x+screen->w &&
			i->pos_y>game->rpg.viewport_y && i->pos_y<game->rpg.viewport_y+screen->h &&
			i->image && i->status && i->vanish_time==0)
		{
			sprite *it=mgos[npc_mgos[t]].getsprite(i->curr_frame);
			it->x=i->pos_x-game->rpg.viewport_x-it->width/2;
			it->y=i->pos_y-game->rpg.viewport_y-i->layer*8+9;
			it->l=i->layer*8+2;
			active_list.push_back(it);
		}
}
void Scene::Redraw_Tiles_or_Fade_to_pic()
{
	redraw_flag=1;
}
void Scene::move_usable_screen()
{
	if(redraw_flag)
	{
		produce_one_screen();
		/*if(abstract_x_bak!=team_pos.toXY().x)
		{
			int x1,x2,y1,y2,vx1,vy1,vx2,vy2,tx=abs(team_pos.toXY().x-abstract_x_bak),ty=abs(team_pos.toXY().y-abstract_y_bak);
			if(direction_offs[game->rpg.team_direction][1]>0)
				y1=screen->h-8,y2=screen->h,vy1=ty,vy2=0;
			else
				y1=0,y2=8,					vy1=0,vy2=ty;		
			if(direction_offs[game->rpg.team_direction][0]>0)
				x1=screen->w-16,x2=screen->w,vx1=tx,vx2=0;
			else
				x1=0,x2=16,					vx1=0,vx2=tx;

  			scenemap.blit_to(scene_buf,vx1,vy1,vx2,vy2);
			
			short &vx=game->rpg.viewport_x,&vy=game->rpg.viewport_y;
			scenemap.make_onescreen(scene_buf,vx,vy+y1,vx+screen->w,vy+y2);
			scenemap.make_onescreen(scene_buf,vx+x1,vy,vx+x2,vy+screen->h);
		}*/
	}
}
void Scene::get_sprites()
{
	load_NPC_mgo();
}
void Scene::produce_one_screen()
{
	scenemap.make_onescreen(scene_buf,game->rpg.viewport_x,game->rpg.viewport_y,game->rpg.viewport_x+screen->w,game->rpg.viewport_y+screen->h);
}
void Scene::draw_normal_scene(int)
{
	static BITMAP *scanline=create_bitmap(screen->w,screen->h);
	blit(scene_buf,scanline,0,0,0,0,screen->w,screen->h);
	for(s_list::iterator i=active_list.begin();i!=active_list.end();i++)
		(*i)->blit_to(scanline);
	blit(scanline,screen,0,0,0,0,screen->w,screen->h);
	pal_fade_in(1);
}
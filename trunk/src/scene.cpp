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
map::map():scene_map(0,320,200),sprites(boost::extents[0x40][0x80][2][2])
{}
int t;
void map::change(int p){
	t=p;
}
inline void map::make_tile(uint8_t *_buf,int x,int y,int h,int vx,int vy,BITMAP *dest)
{
	int index=(_buf[1]&0x10)<<4|_buf[0],index2=(_buf[3]&0x10)<<4|_buf[2];
	bool throu=(_buf[1]&0x20)?true:false,throu2=(_buf[3]&0x20)?true:false;
	int layer=_buf[1]&0xf,layer2=_buf[3]&0xf;
	getsprite(x,y,h,0,GOP.decode(t,index),throu,layer).blit_to(dest,x*32+h*16-16-vx,y*16+h*8-8-vy);
	if(index2)
		getsprite(x,y,h,1,GOP.decode(t,index2-1),throu,layer).blit_to(dest,x*32+h*16-16-vx,y*16+h*8-8-vy);
}
void map::make_onescreen(BITMAP *dest,int source_x,int source_y)
{
	uint8_t *mapbuf=MAP.decode(t,0);
	for(int y=source_y/16;y<source_y/16+14;y++)
		for(int x=source_x/32;x<source_x/32+11;x++)
			for(int h=0;h<2;h++)
				make_tile(mapbuf+y*0x200+x*8+h*4,x,y,h,source_x,source_y,bmp);
	blit(bmp,dest,0,0,0,0,320,200);
}

Scene::Scene():scene_buf(create_bitmap(320,200)),current(0),toload(1),team_pos(game->rpg.viewport_x+x_scrn_offset,game->rpg.viewport_y+y_scrn_offset)
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
	if(key>=VK_DOWN && key<=VK_RIGHT){
		direction=key-3;
		position target=team_pos+position(direction_offs[direction][0],direction_offs[direction][1]);
		if(!scenemap.gettile(target.toXYH().x,target.toXYH().y,target.toXYH().h,0).blocked)
		{
			game->rpg.viewport_x+=direction_offs[direction][0];
			game->rpg.viewport_y+=direction_offs[direction][1];
			curframe=(curframe+1)%4;
			team_pos.toXY()=target;
		}
	}
	if(team_pos.toXY().x==abstract_x_bak && team_pos.toXY().y==abstract_y_bak)
		curframe&=2,curframe^=2;
	for(std::list<sprite_prim *>::iterator i=team_prims.begin();i!=team_prims.end();i++)
		(*i)->curr=direction*3+frames3[curframe];
}
void Scene::our_team_setdraw()
{
	//std::copy(team_prims.begin(),team_prims.end(),std::back_inserter(active_list));
	for(std::list<sprite_prim *>::iterator it=team_prims.begin();it!=team_prims.end();it++)
		active_list.push_back((*it)->getcurrent());
}
void Scene::visible_NPC_movment_setdraw()
{}
void Scene::Redraw_Tiles_or_Fade_to_pic()
{}
void Scene::move_usable_screen()
{
	scenemap.blit_to(scene_buf,game->rpg.viewport_x-viewport_x_bak,game->rpg.viewport_y-viewport_y_bak,0,0);
//	produce_one_screen();
}
void Scene::get_sprites()
{
}
void Scene::produce_one_screen()
{
	scenemap.make_onescreen(scene_buf,game->rpg.viewport_x,game->rpg.viewport_y);
}
void Scene::draw_normal_scene(int)
{
	for(s_list::iterator i=active_list.begin();i!=active_list.end();i++)
		(*i)->blit_to(scene_buf);
	blit(scene->scene_buf,screen,0,0,0,0,320,200);
}
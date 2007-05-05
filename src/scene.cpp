#include "scene.h"
#include "internal.h"
#include "game.h"

sprite &map::getsprite(int x,int y,int h,int l,uint8_t *src,bool throu,int layer)
{
	sprites[x][y][h][l].image=boost::shared_ptr<sprite>(new sprite(src));
	sprites[x][y][h][l].throughable=throu;
	sprites[x][y][h][l].layer=layer;
	return *sprites[x][y][h][l].image.get();
}
map::map():scene_map(0,2032,2040),sprites(boost::extents[0x40][0x80][2][2])
{}
void map::change(int p){
	uint8_t *mapbuf=MAP.decode(p,0);
	boost::multi_array_ref<uint8_t[4],3> mapdef((uint8_t*[4])mapbuf,boost::extents[0x80][0x40][2]);
	for(int y=0;y<0x80;y++)
		for(int x=0;x<0x40;x++)
			for(int h=0;h<2;h++)
			{
				uint8_t *_buf=mapbuf+y*0x200+x*8+h*4;
				int index=(_buf[1]&0x10)<<4|_buf[0],index2=(_buf[3]&0x10)<<4|_buf[2];
				bool throu=(_buf[1]&0x20)>>5?true:false,throu2=(_buf[3]&0x20)>>5?true:false;
				int layer=_buf[1]&0xf,layer2=_buf[3]&0xf;
				getsprite(x,y,h,0,GOP.decode(p,index),throu,layer).blit_to(bmp,x*32+h*16-16,y*16+h*8-8);
				if(index2)
					getsprite(x,y,h,1,GOP.decode(p,index2-1),throu,layer).blit_to(bmp,x*32+h*16-16,y*16+h*8-8);
			}
}

Scene::Scene():scene_buf(create_bitmap(320,200)),current(0),toload(1)
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
	abstract_x_bak=game->rpg.viewport_x+x_scrn_offset;
	abstract_y_bak=game->rpg.viewport_y+y_scrn_offset;
	if(key>=VK_DOWN && key<=VK_RIGHT){
		direction=key-3;
		//if through
			game->rpg.viewport_x+=direction_offs[direction][0];
			game->rpg.viewport_y+=direction_offs[direction][1];
			curframe=(curframe+1)%4;
			role_abstract_x=game->rpg.viewport_x+x_scrn_offset;
			role_abstract_y=game->rpg.viewport_y+y_scrn_offset;
	}
	if(role_abstract_x==abstract_x_bak && role_abstract_y==abstract_y_bak)
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
	produce_one_screen();
}
void Scene::get_sprites()
{
}
void Scene::produce_one_screen()
{
	scenemap.blit_to(scene_buf,game->rpg.viewport_x,game->rpg.viewport_y,0,0);
}
void Scene::process_scrn_drawing(int)
{
	for(s_list::iterator i=active_list.begin();i!=active_list.end();i++)
		(*i)->blit_to(scene_buf);
	blit(scene->scene_buf,screen,0,0,0,0,320,200);
}
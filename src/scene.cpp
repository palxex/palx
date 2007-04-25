#include "scene.h"
#include "internal.h"

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
{}
void Scene::our_team_setdraw()
{}
void Scene::visible_NPC_movment_setdraw()
{}
void Scene::Redraw_Tiles_or_Fade_to_pic()
{}
void Scene::move_usable_screen()
{
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
		(*i)->blit(0,scene_buf,team_pos.x,team_pos.y);
	blit(scene->scene_buf,screen,0,0,0,0,320,200);
}
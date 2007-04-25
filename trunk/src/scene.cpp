#include "scene.h"
#include "internal.h"

Scene::Scene(scene_map *_map):now(_map),scene_buf(create_bitmap(320,200)),current(0),toload(1)
{}
Scene::~Scene()
{}
void Scene::clear_scene()
{
	clear_bitmap(scene_buf);
}
void Scene::clear_active()
{
	active_list.swap(s_list());
}
void Scene::calc_team_walking()
{}
void Scene::our_team_setdraw()
{}
void Scene::visible_NPC_movment_setdraw()
{}
void Scene::Redraw_Tiles_or_Fade_to_pic()
{}
void Scene::adjust_viewport()
{
	camera_pos.x=team_pos.x-x_scrn_offset;
	camera_pos.y=team_pos.x-y_scrn_offset;
}
void Scene::get_sprites()
{}
void Scene::produce_one_screen()
{
	now->blit_to(scene_buf,camera_pos.x,camera_pos.y,0,0);
}
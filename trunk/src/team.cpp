#include "internal.h"

void load_team_mgo(int role1,int role2,int role3)
{
	for(int i=0;i<=game->rpg.team_roles;i++){
		sprite_action *t=new sprite_action;
		t->getsource(MGO.decode(game->rpg.roles_properties.icon[i]));
		scene->active_list.push_back(boost::shared_ptr<sprite_action>(t));
	}
}
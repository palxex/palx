#include "internal.h"

void load_team_mgo()
{
	for(int i=0;i<=game->rpg.team_roles;i++){
		bool changed;
		uint8_t *buf=MGO.decode(game->rpg.roles_properties.avator[game->rpg.team[i].role],changed);
		if(!changed){
			sprite_action *t=new sprite_action;
			t->getsource(buf);
			scene->active_list.push_back(boost::shared_ptr<sprite_action>(t));
		}
	}
}
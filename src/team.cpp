#include "internal.h"
#include "resource.h"
#include "game.h"
#include <algorithm>
#include <boost/lambda/lambda.hpp>
//using namespace boost::lambda;

std::list<sprite_prim> mgos;
typedef std::list<sprite_prim>::iterator mgo_iter;
std::list<sprite_prim *> team_prims;

bool load_mgo(int id,int layer,int y_off,int layer_off)
{
	bool decoded;
	uint8_t *buf=MGO.decode(id,decoded);
	if(!decoded)
		mgos.push_back(sprite_prim(id,buf,layer,y_off,layer_off));
	return decoded;
}
void load_team_mgo()
{
	team_prims.swap(std::list<sprite_prim *>());
	for(int i=0;i<=game->rpg.team_roles;i++)
	{
		int id=game->rpg.roles_properties.avator[game->rpg.team[i].role];
		load_mgo(id,game->rpg.layer,10,6);
		sprite_prim *tmp=&*std::find(mgos.begin(),mgos.end(),sprite_prim(id));
		team_prims.push_back(tmp);
	};
}
#include "Game.h"

#include <alfont.h>

#include "internal.h"
#include "scene.h"

using namespace std;
namespace{
	template<typename T>
	inline void reunion(vector<T> &vec,uint8_t *src,long &len)
	{
		T *usrc=(T *)src;
		copy(usrc,usrc+len/sizeof(T),back_inserter(vec));
	}
	inline void reunion(sprite_prim &vec,uint8_t *src,long &len)
	{
		vec.getsource(src);
	}
	template<typename T>
	inline void reunion(T *vec,uint8_t *src,long &len)
	{
		T *usrc=(T *)src;
		memcpy(vec,usrc,len);
	}
	template<typename T>
	inline void reunion(T &vec,uint8_t *src,long &len)
	{
		memcpy(&vec,src,len);
	}
}

Game::Game(int save=0):
	rpg((memset(&rpg,0,sizeof(RPG)),rpg))
{
	rpg.team_roles=-1;
	//allegro init
	allegro_init();
	install_timer();
	install_keyboard();
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,320,200,0,0);
	set_color_depth(8);

	pat.set(0);

	alfont_init();
	ttfont::glb_font=alfont_load_font("simsun.ttc");
	alfont_set_language(ttfont::glb_font, Encode_Code);	
	alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
	//alfont_text_mode(-1);
	alfont_set_font_background(ttfont::glb_font, FALSE);
	alfont_set_font_size(ttfont::glb_font,15);
	//global setting

	//load sss&data
	long len;
	EVENT_OBJECT teo;memset(&teo,0,sizeof(teo));evtobjs.push_back(teo);
	SCENE   tsn;memset(&tsn,0,sizeof(tsn));scenes.push_back(tsn);
	reunion(evtobjs,	SSS.decode(0,len), len);
	reunion(scenes,		SSS.decode(1,len), len);
	reunion(rpg.objects,SSS.decode(2,len), len);
	reunion(msg_idxes,	SSS.decode(3,len), len);
	reunion(scripts,	SSS.decode(4,len), len);

	reunion(shops,					DATA.decode(0,len),len);
	reunion(monsters,				DATA.decode(1,len),len);
	reunion(enemyteams,				DATA.decode(2,len),len);
	reunion(rpg.roles_properties,	DATA.decode(3,len),len);
	reunion(magics,					DATA.decode(4,len),len);
	reunion(battlefields,			DATA.decode(5,len),len);
	reunion(learns,					DATA.decode(6,len),len);
	//7:not used
	//8:not used
	reunion(UIpics,					DATA.decode(9,len),len);
	reunion(discharge_effects,		DATA.decode(10,len),len);
	//11:not known!!!
	reunion(message_handles,		DATA.decode(12,len),len);
	reunion(enemyposes,				DATA.decode(13,len),len);
	reunion(upgradexp,				DATA.decode(14,len),len);

	flag_to_load=0x1D;
	//load save
	if(save!=0)
		load(save);
}
Game::~Game(){
}

void Game::load(int id){
	flag_to_load=0x12;
	rpg_to_load=id;
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"rb");
	if(!fprpg){
		allegro_message("ºÜ±§Ç¸£¬%d.rpg²»´æÔÚ¡«",id);
		exit(-1);
	}
	fread((RPG*)this,sizeof(RPG),1,fprpg);
	scene->toload=rpg.scene_id;
	fclose(fprpg);
}
void Game::save(int id){
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"wb");
	fclose(fprpg);
}
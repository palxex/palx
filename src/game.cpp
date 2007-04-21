#include "game.h"

#include <alfont.h>
#include <boost/shared_array.hpp>

using namespace std;
using namespace boost;
namespace{
	template<typename T>
	inline void reunion(vector<T> &vec,shared_array<uint8_t> &src,long &len)
	{
		T *usrc=(T *)src.get();
		copy(usrc,usrc+len/sizeof(T),back_inserter(vec));
	}
	template<typename T>
	inline void reunion(T *vec,shared_array<uint8_t> &src,long &len)
	{
		T *usrc=(T *)src.get();
		memcpy(vec,usrc,len);
	}
}

game::game(int save=0):
	rpg((memset(&rpg,0,sizeof(RPG)),rpg))
{
	//allegro init
	allegro_init();
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,320,200,0,0);
	set_color_depth(8);

	alfont_init();
	ttfont::glb_font=alfont_load_font("simsun.ttc");
	alfont_set_language(ttfont::glb_font, Encode_Code);	
	alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
	//alfont_text_mode(-1);
	alfont_set_font_background(ttfont::glb_font, FALSE);
	alfont_set_font_size(ttfont::glb_font,15);
	//global setting

	//load sss&data
	long t;
	EVENT_OBJECT teo;memset(&teo,0,sizeof(teo));evtobjs.push_back(teo);
	SCENE   tsn;memset(&tsn,0,sizeof(tsn));scenes.push_back(tsn);
	reunion(evtobjs,	SSS.decode(0,0,t),t);
	reunion(scenes,		SSS.decode(1,0,t),t);
	reunion(objects,	SSS.decode(2,0,t),t);
	reunion(msg_idxes,	SSS.decode(3,0,t),t);
	reunion(scripts,	SSS.decode(4,0,t),t);
	reunion(shops,		DATA.decode(0,0,t),t);

	pat.set(0);
	//load save
	if(save!=0)
		load(save);
}
game::~game(){
}

void game::load(int id){
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"rb");
	if(!fprpg){
		allegro_message("ºÜ±§Ç¸£¬%d.rpg²»´æÔÚ¡«",id);
		exit(-1);
	}
	fclose(fprpg);
}
void game::save(int id){
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"wb");
	fclose(fprpg);
}
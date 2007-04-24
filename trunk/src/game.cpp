#include "Game.h"

#include <alfont.h>

#include "internal.h"

using namespace std;
namespace{
	template<typename T>
	inline void reunion(vector<T> &vec,uint8_t *src,long &len)
	{
		T *usrc=(T *)src;
		copy(usrc,usrc+len/sizeof(T),back_inserter(vec));
	}
	template<typename T>
	inline void reunion(T *vec,uint8_t *src,long &len)
	{
		T *usrc=(T *)src;
		memcpy(vec,usrc,len);
	}
	inline int determain_smkfs(uint8_t *src)
	{
		uint16_t *usrc=(uint16_t*)src;
		return usrc[0]-(usrc[usrc[0]-1]==0?1:0);
	}
}

Game::Game(int save=0):
	rpg((memset(&rpg,0,sizeof(RPG)),rpg))
{
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
	long t;
	EVENT_OBJECT teo;memset(&teo,0,sizeof(teo));evtobjs.push_back(teo);
	SCENE   tsn;memset(&tsn,0,sizeof(tsn));scenes.push_back(tsn);
	reunion(evtobjs,	SSS.decode(0,0,t), t);
	reunion(scenes,		SSS.decode(1,0,t), t);
	reunion(rpg.objects,SSS.decode(2,0,t), t);
	reunion(msg_idxes,	SSS.decode(3,0,t), t);
	reunion(scripts,	SSS.decode(4,0,t), t);

	reunion(shops,								DATA.decode(0,0,t),t);
	reunion(monsters,							DATA.decode(1,0,t),t);
	reunion(enemyteams,							DATA.decode(2,0,t),t);
	reunion(rpg.roles_properties,				DATA.decode(3,0,t),t);
	reunion(magics,								DATA.decode(4,0,t),t);
	reunion(battlefields,						DATA.decode(5,0,t),t);
	reunion(learns,								DATA.decode(6,0,t),t);
	//7:not used
	//8:not used
	//11:not known!!!
	reunion(enemyposes,							DATA.decode(13,0,t),t);
	reunion(upgradexp,							DATA.decode(14,0,t),t);

	int subfiles_9 =determain_smkfs(			DATA.decode( 9,0,t)),
		subfiles_10=determain_smkfs(			DATA.decode(10,0,t)),
		subfiles_12=determain_smkfs(			DATA.decode(12,0,t));
	decoder_func olddecoder=DATA.setdecoder(de_mkf_smkf);
	for(int i=0;i<subfiles_9;i++)
		UIpics.push_back(new sprite(			DATA.decode( 9,i)));
	for(int i=0;i<subfiles_10;i++)
		discharge_effects.push_back(new sprite(	DATA.decode(10,i)));
	for(int i=0;i<subfiles_12;i++)
		message_handles.push_back(new sprite(	DATA.decode(12,i)));

	flag_to_load=0x1D;
	//load save
	if(save!=0)
		load(save);
}
Game::~Game(){
	typedef vector<sprite *>::iterator iter;
	for(iter i=UIpics.begin();i!=UIpics.end();i++)
		delete *i;
	for(iter i=discharge_effects.begin();i!=discharge_effects.end();i++)
		delete *i;
	for(iter i=message_handles.begin();i!=message_handles.end();i++)
		delete *i;
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
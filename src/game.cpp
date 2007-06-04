#include "Game.h"

#include <alfont.h>

#include "internal.h"
#include "scene.h"

#include "begin.h"

using namespace std;
volatile uint8_t time_interrupt_occurs; 
namespace{
	template<typename T>
	inline void reunion(vector<T> &vec,uint8_t *src,const long &len)
	{
		T *usrc=(T *)src;
		copy(usrc,usrc+len/sizeof(T),back_inserter(vec));
	}
	inline void reunion(sprite_prim &vec,uint8_t *src,const long &len)
	{
		vec.getsource(src);
	}
	template<typename T>
	inline void reunion(T *vec,uint8_t *src,const long &len)
	{
		T *usrc=(T *)src;
		memcpy(vec,usrc,len);
	}
	template<typename T>
	inline void reunion(T &vec,uint8_t *src,const long &len)
	{
		memcpy(&vec,src,len);
	}
	void timer_proc()
	{
		static int pal_lock=0;
		static PALETTE pal;
		mutux_setpalette=false;
		if(pal_lock++==10){
			get_palette(pal);
			RGB temp=pal[0xF6];
			memcpy(pal+0xF6,pal+0xF7,6);
			pal[0xF8]=temp;
			temp=pal[0xF9];
			memcpy(pal+0xF9,pal+0xFA,0x12);
			pal[0xFE]=temp;
			set_palette(pal);
			pal_lock=0;
		};
		time_interrupt_occurs++;
		mutux_setpalette=true;
	}
	END_OF_FUNCTION(timer_proc);
}

int scale=1;
RPG rpg;
Game::Game(int save=0):rpg(::rpg)
{
	memset(&rpg,0,sizeof(RPG));
		scale=SCREEN_W/320;
		x_scrn_offset*=scale;
		y_scrn_offset*=scale;


	alfont_init();
	ttfont::glb_font=alfont_load_font(strcat(getenv("WINDIR"),"\\fonts\\mingliu.ttc"));
	alfont_set_language(ttfont::glb_font, "cht");	
	alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
	//alfont_text_mode(-1);
	alfont_set_font_background(ttfont::glb_font, FALSE);
	alfont_set_font_size(ttfont::glb_font,16);
	//global setting
	
	LOCK_VARIABLE(time_interrupt_occurs);
	install_int(timer_proc,10);

	//load sss&data
	long len=0;
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
	if(save!=0 || (save=begin_scene()(this)))
		rpg_to_load=save;
	else
		map_toload=1;
}
Game::~Game(){
	remove_int(timer_proc);
}

void Game::load(int id){
	if(!id)
		return;
	flag_to_load=0x12;
	rpg_to_load=id;
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"rb");
	if(!fprpg){
		allegro_message("ºÜ±§Ç¸£¬%d.rpg²»´æÔÚ¡«",id);
		exit(-1);
	}
	fread(&rpg,sizeof(RPG),1,fprpg);
	map_toload=rpg.scene_id;
	evtobjs.clear();evtobjs.push_back(EVENT_OBJECT());
	reunion(evtobjs,(uint8_t*)&rpg.evtobjs,(const long&)sizeof(rpg.evtobjs));
	scenes.clear();scenes.push_back(SCENE());
	reunion(scenes,(uint8_t*)&rpg.scenes,(const long&)sizeof(rpg.scenes));
	fclose(fprpg);
	flag_to_load=0x17;
	pat.read(0);
	pal_fade_out(1);
}
void Game::save(int id){
	FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"wb");
	copy(evtobjs.begin()+1,evtobjs.end(),rpg.evtobjs);
	copy(scenes.begin()+1,scenes.end(),rpg.scenes);
	fwrite(&rpg,sizeof(RPG),1,fprpg);
	fclose(fprpg);
}
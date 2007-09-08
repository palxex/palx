/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL                  *
 *   Copyleft (C) 2006 by Pal Lockheart                                    *
 *   palxex@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, If not, see                          *
 *   <http://www.gnu.org/licenses/>.                                       *
 ***************************************************************************/
#include "game.h"

#include <alfont.h>

#include "internal.h"
#include "scene.h"

using namespace std;
volatile uint8_t time_interrupt_occurs;
int mutex_paletting=0,mutex_blitting=0,mutex_int=0;
extern int rix_fade_flag;
int scale=1;
namespace res{
	palette pat;
	RPG rpg;
	//data
	std::vector<SHOP> shops;
	std::vector<MONSTER> monsters;
	std::vector<ENEMYTEAM> enemyteams;
	std::vector<MAGIC> magics;
	std::vector<BATTLE_FIELD> battlefields;
	std::vector<UPGRADE_LEARN> learns;
	sprite_prim UIpics;
	sprite_prim discharge_effects;
	sprite_prim message_handles;
	ENEMY_POSES enemyposes;
	std::vector<UPGRADE_EXP> upgradexp;

	//sss
	std::vector<EVENT_OBJECT> evtobjs;
	std::vector<SCENE>   scenes;
	std::vector<int32_t> msg_idxes;
	std::vector<SCRIPT>  scripts;

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
		if(rix_fade_flag && rix->getvolume()==0){
			rix_fade_flag=0;
			rix->playing=false;
		}
		if(!mutex_int){
			static int pal_lock=0;
			static PALETTE pal;
			mutux_setpalette=false;
			if(pal_lock++==10){
				get_palette_range(pal,0xF6,0xFF);
				RGB temp=pal[0xF6];
				memcpy(pal+0xF6,pal+0xF7,6);
				pal[0xF8]=temp;
				temp=pal[0xF9];
				memcpy(pal+0xF9,pal+0xFA,0x12);
				pal[0xFE]=temp;
				set_palette_range(pal,0xF6,0xFF,1);
				pal_lock=0;
			};
			time_interrupt_occurs++;
			mutux_setpalette=true;
		}
		rest(0);
	}
	END_OF_FUNCTION(timer_proc);
	void prtscrn_proc()
	{
		if(key[KEY_PRTSCR] || key[KEY_P]){
			static PALETTE pal;
			static char filename[30];
			static int i=0;
			get_palette(pal);
			sprintf(filename,"ScrnShot\\%d.bmp",i++);
			save_bitmap(filename,screen,pal);
		}
		rest(0);
	}
	END_OF_FUNCTION(prtscrn_proc);

    void init_resource()
    {
        memset(&rpg,0,sizeof(RPG));
            scale=SCREEN_W/320;
            x_scrn_offset*=scale;
            y_scrn_offset*=scale;


        alfont_init();
        char fontpath[100];
        sprintf(fontpath,"%s%s",getenv("WINDIR"),"\\fonts\\mingliu.ttc");
        ttfont::glb_font=alfont_load_font(fontpath);
        alfont_set_language(ttfont::glb_font, "cht");
        alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
        //alfont_text_mode(-1);
        alfont_set_font_background(ttfont::glb_font, FALSE);
        alfont_set_char_extra_spacing(ttfont::glb_font,1);
        alfont_set_font_size(ttfont::glb_font,16);
        //global setting

        LOCK_VARIABLE(time_interrupt_occurs);
        install_int(timer_proc,10);

        install_int(prtscrn_proc,100);

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

    }
    void destroy_resource(){
        remove_int(timer_proc);
        remove_int(prtscrn_proc);
    }

    /*/
    extern "C" __declspec(dllimport) uint32_t __stdcall GetLastError();/*/
    int GetLastError(){return 0;}//*/
    void load(int id){
        if(!id)
            return;
        flag_to_load=0x12;
        rpg_to_load=id;
        FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"rb");
        if(!fprpg){
            allegro_message("sorry,%d.rpg is not exist~The ERROR CODE is %lx",id,GetLastError());
            return;
        }
        scene->scenemap.change(-1);
        fread(&rpg,sizeof(RPG),1,fprpg);
        rpg.viewport_x-=0xA0*(scale-1);
        rpg.viewport_y-=0x70*(scale-1);
        for(int i=0;i<=rpg.team_roles+rpg.team_followers;i++){
            rpg.team[i].x+=0xA0*(scale-1);
            rpg.team[i].y+=0x70*(scale-1);
        }
        map_toload=rpg.scene_id;
        evtobjs.clear();evtobjs.push_back(EVENT_OBJECT());
        reunion(evtobjs,(uint8_t*)&rpg.evtobjs,(const long&)sizeof(rpg.evtobjs));
        scenes.clear();scenes.push_back(SCENE());
        reunion(scenes,(uint8_t*)&rpg.scenes,(const long&)sizeof(rpg.scenes));
        fclose(fprpg);
        flag_to_load=0x17;
        MAP.clear();
        GOP.clear();
        RNG.clear();
        SSS.clear();
        //DATA.clear();
        //ABC.clear();
        //VOC.clear();
        //BALL.clear();
        //RGM.clear();
        //FBP.clear();
        //F.clear();
        //FIRE.clear();
        //MGO.clear();
        pat.read(0);
        pal_fade_out(1);
    }
    void save(int id){
        FILE *fprpg=fopen(static_cast<ostringstream&>(ostringstream()<<id<<".rpg").str().c_str(),"wb");
        copy(evtobjs.begin()+1,evtobjs.end(),rpg.evtobjs);
        copy(scenes.begin()+1,scenes.end(),rpg.scenes);
        fwrite(&rpg,sizeof(RPG),1,fprpg);
        fclose(fprpg);
    }

}

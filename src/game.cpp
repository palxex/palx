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
#include "alfont.h"
#include "internal.h"
#include "scene.h"

using namespace std;
volatile uint8_t time_interrupt_occurs;
int mutex_paletting=0,mutex_blitting=0,mutex_int=0;
int scale=1;
namespace res{
	palette pat;
	RPG rpg;
	SETUP_DEF setup;
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
	UPGRADE_EXP upgradexp;

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
	inline void reunion(sprite_prim &vec,cached_res &ar,int f)
	{
		vec.getsource(ar,f);
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
		if(!mutex_int){
			static int pal_lock=0;
			static PALETTE pal;
			mutex_paletting=true;
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
			mutex_paletting=false;
		}
		rest(1);
	}
	END_OF_FUNCTION(timer_proc);

    void init_resource()
    {
        memset(&rpg,0,sizeof(RPG));
            scale=SCREEN_W/320;
            x_scrn_offset*=scale;
            y_scrn_offset*=scale;


        LOCK_VARIABLE(time_interrupt_occurs);
        install_int(timer_proc,10);

        //load sss&data
        long len=0;
        EVENT_OBJECT teo;memset(&teo,0,sizeof(teo));evtobjs.push_back(teo);
        SCENE   tsn;memset(&tsn,0,sizeof(tsn));scenes.push_back(tsn);
        reunion(setup,      SETUP.decode(len), len);

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
        //11:not known!!!
        reunion(enemyposes,				DATA.decode(13,len),len);
        reunion(upgradexp,				DATA.decode(14,len),len);

		DATA.setdecoder(de_mkf_smkf);
        reunion(UIpics,					DATA,9);
        reunion(discharge_effects,		DATA,10);
        reunion(message_handles,		DATA,12);

        flag_to_load=0x1D;

    }
    void destroy_resource(){
        remove_int(timer_proc);
    }

    /*/
    extern "C" __declspec(dllimport) uint32_t __stdcall GetLastError();/*/
    int GetLastError(){return 0;}//*/
    void load(int id){
        if(!id)
            return;
        pat.read(0);
        char name[80];
        sprintf(name,"%s/%d.RPG",path_root.c_str(),id);
        FILE *fprpg=fopen(name,"rb");
        if(!fprpg){
			pat.set(rpg.palette_offset);
			map_toload=(rpg.scene_id?rpg.scene_id:1);
            return;
        }
        flag_to_load=0x12;
        rpg_to_load=id;
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
        pal_fade_out(1);
    }
    void save(int id){
        char name[80];
        sprintf(name,"%s/%d.RPG",path_root.c_str(),id);
        FILE *fprpg=fopen(name,"wb");
		rpg.save_times=1;
        copy(evtobjs.begin()+1,evtobjs.end(),rpg.evtobjs);
        copy(scenes.begin()+1,scenes.end(),rpg.scenes);

		int viewport_x=rpg.viewport_x,viewport_y=rpg.viewport_y;
		rpg.viewport_x+=0xA0*(scale-1),rpg.viewport_y+=0x70*(scale-1);

		RPG::position real_team[TEAMROLES];
		copy(rpg.team,rpg.team+TEAMROLES,real_team);
		for(int i=0;i<5;i++)
			rpg.team[i].x-=0xA0*(scale-1),
			rpg.team[i].y-=0x70*(scale-1);

		RPG::track real_track[TEAMROLES];
		copy(rpg.team_track,rpg.team_track+TEAMROLES,real_track);
		for(int i=0;i<5;i++)
			rpg.team_track[i].x-=0xA0*(scale-1),
			rpg.team_track[i].y-=0x70*(scale-1);

        fwrite(&rpg,sizeof(RPG),1,fprpg);
        fclose(fprpg);

		rpg.viewport_x=viewport_x,rpg.viewport_y=viewport_y;
		copy(real_team,real_team+TEAMROLES,rpg.team);
		copy(real_track,real_track+TEAMROLES,rpg.team_track);
    }

}

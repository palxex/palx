/***************************************************************************
 *   PALx: A platform independent port of classic RPG PAL              *
 *   Copyleft (C) 2006 by Pal Lockheart                        *
 *   palxex@gmail.com                                      *
 *                                                 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                           *
 *                                                 *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                  *
 *                                                 *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, If not, see                  *
 *   <http://www.gnu.org/licenses/>.                           *
 ***************************************************************************/
#include <boost/shared_array.hpp>

#include <cstdio>

#include "resource.h"
#include "UI.h"
#include "pallib.h"
#include "config.h"

#if defined (WIN32)
#   define FONT_PATH getenv("WINDIR")
#   define FONT_FILE "/fonts/mingliu.ttc"
#   define LOCALE "chinese"
#else
#   define FONT_PATH ""
#   define FONT_FILE "/usr/share/fonts/truetype/arphic/uming.ttf" //ubuntu gutsy gibbon;other distribution has other position but I don't know the unified method to determine it.
#   define LOCALE "BIG5"
#endif

using namespace std;
using namespace boost;
using namespace Pal::Tools;

ini_parser::ini_parser(char *conf):name(conf),needwrite(false)
{
	ini_parser::section::configmap configprop;
	configprop["path"].value=".";
	configprop["path"].comment="资源路径";
	configprop["setup"].value="true";
	configprop["setup"].comment="Bool;是否用setup.dat里的设置覆盖这里的对应设置";
	configprop["allow_memory"].value="";
	configprop["allow_memory"].comment="Bool;是否允许跨进程记忆最后存档";
	configprop["last"].value="";
	configprop["last"].comment="Int;最后载入的存档";
	configprop["resource"].value="dos";
	configprop["resource"].comment="dos/win95/ss(?)";
	configprop["encode"].value=LOCALE;
	configprop["encode"].comment="win32:chs/cht;linux/mac/dos/...(iconv):GBK/BIG5";
	section config("config",configprop);
	sections["config"]=config;

	ini_parser::section::configmap debugprop;
	debugprop["resource"].value="mkf";
	debugprop["resource"].comment="资源使用方式;mkf/filesystem";
	debugprop["allow_frozen"].value="true";
	debugprop["allow_frozen"].comment="允许冻结启动/停止;true/false";
	section debug("debug",debugprop);
	sections["debug"]=debug;

	ini_parser::section::configmap displayprop;
	displayprop["height"].value="200";
	displayprop["height"].comment="粒度;320x200正整数倍.";
	displayprop["width"].value="320";
	displayprop["scale"].value="none";
	displayprop["scale"].comment="none, 2x; etc.";
	displayprop["fullscreen"].value="false";
	displayprop["fullscreen"].comment="Bool ;全屏";
	section display("display",displayprop);
	sections["display"]=display;

	ini_parser::section::configmap fontprop;
	fontprop["type"].value="truetype";
	fontprop["type"].comment="truetype: ttf/ttc; fon: wor16.fon";
	char fontpath[100];
	sprintf(fontpath,"%s%s",FONT_PATH,FONT_FILE);
	fontprop["path"].value=fontpath;
	section font("font",fontprop);
	sections["font"]=font;

	ini_parser::section::configmap musicprop;
	musicprop["type"].value="rix";
	musicprop["type"].comment="rix/mid/foreverCD,+gameCD,+gameCDmp3，或任意混合。foreverCD指永恒回忆录之FM曲集";
	musicprop["volume"].value="255";
	musicprop["volume"].comment="0-255;音量";
	musicprop["volume_sfx"].value="255";
	musicprop["volume_sfx"].comment="0-255;音效音量";
	section music("music",musicprop,"与setup正交");
	sections["music"]=music;

	ini_parser::section::configmap keyprop;
	keyprop["west"].value="0x4b";
	keyprop["north"].value="0x48";
	keyprop["east"].value="0x4d";
	keyprop["south"].value="0x50";
	section keymap("keymap",keyprop,"行走键盘定义；与setup正交");
	sections["keymap"]=keymap;

	ifstream ifs(name.c_str());
	if(ifs.is_open() )
		while(!ifs.eof()){
			section s;
			ifs>>s;
			sections[s.name()]=s;
		}
}

void ini_parser::write(){
	ofstream ofs(name.c_str(),ios_base::out|ios_base::trunc);
	for(std::map<string,section>::const_iterator i=sections.begin();i!=sections.end();i++)
		ofs<<(*i).second;
	ofs.flush();
}

ini_parser::~ini_parser()
{
    if(getSection("config").get("allow_memory",bool()) && getSection("config").get("last",int())!=rpg_to_load)
        getSection("config").keymap["last"].value=lexical_cast<string>(rpg_to_load),needwrite=true;
	if(needwrite)
		write();
}

namespace{
	uint8_t *denone_file(const char *file,long &len)
	{
		int32_t length;
		FILE *fp=fopen(file,"rb");
		fseek(fp,0,SEEK_END);
		length=ftell(fp);
		rewind(fp);
		uint8_t *buf=new uint8_t[length];
		fread(buf,length,1,fp);
		fclose(fp);
		len=length;
		return buf;
	}
	uint8_t *demkf_ptr(const char *file,int n,long &len)
	{
		int32_t offset=n*4,length;
		FILE *fp=fopen(file,"rb");
		fseek(fp,offset,SEEK_SET);
		fread(&offset,4,1,fp);
		fread(&length,4,1,fp);length-=offset;
		fseek(fp,offset,SEEK_SET);
		uint8_t *buf=new uint8_t[length];
		fread(buf,length,1,fp);
		fclose(fp);
		len=length;
		return buf;
	}
	shared_array<uint8_t> demkf_sp(shared_array<uint8_t> src,int n,long &len,int &files)
	{
		uint32_t *usrc=(uint32_t *)src.get();
		files=usrc[0]/4-2;
		int32_t length=usrc[n+1]-usrc[n];
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n],length);
		len=length;
		return shared_array<uint8_t>(buf);
	}
	shared_array<uint8_t> demkf_file(const char *file,int n,long &len,shared_array<uint8_t> &buf,long &size,bool cached)
	{
		if(cached){
			len=size;
			return buf;
		}else{
			buf = shared_array<uint8_t>(demkf_ptr(file,n,len));
			size=len;
			return buf;
		}
	}
	uint8_t *desmkf_ptr(shared_array<uint8_t> src,int n,long &len,int &files)
	{
		uint16_t *usrc=(uint16_t *)src.get();
		files=(usrc[0]-((usrc[usrc[0]-1]<=0 || usrc[usrc[0]-1]*2>=len || usrc[usrc[0]-1]<usrc[usrc[0]-2])?1:0));
		int16_t length;
		if(n == files - 1)
			length=len-usrc[n]*2;
		else
			length=(usrc[n+1]-usrc[n])*2;
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n]*2,length);
		len=length;
		return buf;
	}
	uint8_t *deyj1_ptr(shared_array<uint8_t> src,long &len)
	{
		void *dst;
		uint32_t length;
		DecodeYJ1(src.get(),dst,(uint32&)length);
		len=length;
		return (uint8_t*)dst;
	}
	shared_array<uint8_t> deyj1_sp(shared_array<uint8_t> src,long &len)
	{
		return shared_array<uint8_t>(deyj1_ptr(src,len));
	}
	uint8_t *deyj2_ptr(shared_array<uint8_t> src,long &len)
	{
		void *dst;
		uint32_t length;
		DecodeYJ2(src.get(),dst,(uint32&)length);
		len=length;
		return (uint8_t*)dst;
	}
	shared_array<uint8_t> deyj2_sp(shared_array<uint8_t> src,long &len)
	{
		return shared_array<uint8_t>(deyj2_ptr(src,len));
	}
	uint8_t *defile_dir(const char *dir,int file,long &len,int &files)
	{
		char *buf=new char[80];
		for(int i=0;i<999999 && file == 0;i++)
		{
			char buf2[80];
			sprintf(buf2,"%s/%d",dir,i);
			FILE *fp=fopen(buf2,"rb");
			if(!fp){
				files=i;
				break;
			}
			fclose(fp);
		}
		sprintf(buf,"%s/%d",dir,file);
		return denone_file(shared_array<char>(buf).get(),len);
	}
	uint8_t *defile_sp(shared_array<char> dir,int file,long &len,int &files)
	{
		return defile_dir(dir.get(),file,len,files);
	}
	shared_array<char> dedir_dir(const char *dir,int dir2)
	{
		char *buf=new char[80];
		sprintf(buf,"%s/%d",dir,dir2);
		return shared_array<char>(buf);
	}
}

int CARD=0;
namespace{
    void close_button_handler(void)
    {
        //if(!yesno_dialog()) return;
        running=false;
        remove_timer();
    }
    END_OF_FUNCTION(close_button_handler)
    int volume;
    void switchin_proc()
    {
        global->set<int>("music","volume",volume);
    }
    void switchout_proc()
    {
        volume=global->get<int>("music","volume");
        rix->stop();
    }
}

global_init::global_init(char *name):conf(name)
{
	//version dispatch
	boost::function<uint8_t *(shared_array<uint8_t> ,long &)> extract_ptr;
	boost::function<shared_array<uint8_t>(shared_array<uint8_t> ,long &)> extract_sp;
	if(get<string>("config","resource")=="dos")
		extract_ptr=deyj1_ptr,extract_sp=deyj1_sp,sfx_file="VOC.MKF";
	else if(get<string>("config","resource")=="win95")
		extract_ptr=deyj2_ptr,extract_sp=deyj2_sp,sfx_file="SOUNDS.MKF";

	if(get<string>("config","encode")=="")
	  if(get<string>("config","resource")=="dos")
	    set<string>("config","encode",LOCALE);
	  else if(get<string>("config","resource")=="win95")
	    set<string>("config","encode","GBK");

	if(get<string>("debug","resource")=="mkf")
	{
		de_none			=bind(denone_file,_1,_4);
		de_mkf			=bind(demkf_ptr,_1,_2,_4);
		de_mkf_yj1		=bind(extract_ptr,	bind(demkf_file,		_1,_2,_4,_6,_7,_8),_4);
		de_mkf_mkf_yj1	=bind(extract_ptr,	bind(demkf_sp,bind(demkf_file,_1,_2,_4,_6,_7,_8),_3,_4,_5),_4);
		de_mkf_smkf	=bind(desmkf_ptr,	bind(demkf_file,		_1,_2,_4,_6,_7,_8),_3,_4,_5);
		de_mkf_yj1_smkf=bind(desmkf_ptr,	bind(extract_sp,		bind(demkf_file,_1,_2,_4,_6,_7,_8),_4),_3,_4,_5);
	}else if(get<string>("debug","resource")=="filesystem"){
		de_none			=bind(denone_file,_1,_4);
		de_mkf			=bind(defile_dir,_1,_2,_4,_5);
		de_mkf_yj1		=de_mkf;
		de_mkf_mkf_yj1	=bind(defile_sp,	bind(dedir_dir,_1,_2),_3,_4,_5);
		de_mkf_smkf	=de_mkf_mkf_yj1;
		de_mkf_yj1_smkf=de_mkf_mkf_yj1;
	}
}
int global_init::operator ()()
{
	string path_root=get<string>("config","path");
	SETUP.set(path_root+"/SETUP.DAT",de_none);
	PAT.set	(path_root+"/PAT.MKF"	,de_mkf);
	//MIDI.set(path_root+"/MIDI.MKF"	,de_mkf);
	SFX.set	(path_root+"/"+sfx_file	,de_mkf);
	DATA.set(path_root+"/DATA.MKF"	,de_mkf);
	SSS.set	(path_root+"/SSS.MKF"	,de_mkf);
	FBP.set	(path_root+"/FBP.MKF"	,de_mkf_yj1);
	MAP.set	(path_root+"/MAP.MKF"	,de_mkf_yj1);
	GOP.set	(path_root+"/GOP.MKF"	,de_mkf_smkf);
	BALL.set(path_root+"/BALL.MKF"	,de_mkf_smkf);
	RGM.set	(path_root+"/RGM.MKF"	,de_mkf_smkf);
	ABC.set	(path_root+"/ABC.MKF"	,de_mkf_yj1_smkf);
	F.set	(path_root+"/F.MKF"	,de_mkf_yj1_smkf);
	FIRE.set(path_root+"/FIRE.MKF"	,de_mkf_yj1_smkf);
	MGO.set	(path_root+"/MGO.MKF"	,de_mkf_yj1_smkf);
	RNG.set	(path_root+"/RNG.MKF"	,de_mkf_mkf_yj1);

	msges.set(path_root+"/M.MSG");
	objs.set(path_root+"/WORD.DAT");
	playrix::set((path_root+"/MUS.MKF").c_str());

	CARD=(get<bool>("display","fullscreen")?GFX_AUTODETECT:GFX_SAFE);

	//allegro init
	allegro_init();
	set_gfx_mode(CARD,get<int>("display","width"),get<int>("display","height"),0,0);
	//set_display_switch_mode(SWITCH_BACKGROUND);
	//set_display_switch_callback(SWITCH_IN,switchin_proc);
	//set_display_switch_callback(SWITCH_OUT,switchout_proc);
	set_color_depth(8);
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	install_timer();
	install_keyboard();keyboard_lowlevel_callback = key_watcher;
	//install_mouse();enable_hardware_cursor();select_mouse_cursor(MOUSE_CURSOR_ARROW);show_mouse(screen);

	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	if(get<string>("font","type")=="truetype")
	{
        alfont_init();
        ttfont::glb_font=alfont_load_font(get<string>("font","path").c_str());
		alfont_set_language(ttfont::glb_font, get<string>("config","encode").c_str());
        alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
        alfont_text_mode(-1);
        alfont_set_font_background(ttfont::glb_font, FALSE);
        alfont_set_char_extra_spacing(ttfont::glb_font,1);
        alfont_set_font_size(ttfont::glb_font,16);
	}else if(get<string>("font","type")=="fon")
	{}

	randomize();

	int save=0;
	if(get<bool>("config","allow_memory"))
		save=get<int>("config","last");

	return save;
}
global_init *global;

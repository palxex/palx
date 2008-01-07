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
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include <cstdio>

#include "resource.h"
#include "UI.H"
#include "pallib.h"

using namespace std;
using namespace boost;
using namespace Pal::Tools;
class ini_parser
{
    struct correspond{
		string value,comment;
	};
	typedef std::map<string,correspond> configmap;
	class section{
		string section_name;
		string section_desc;
		configmap keymap;
	public:
        section(){}
        section(string name,std::map<string,correspond> map,string description=""):section_name(name),section_desc(description),keymap(map)
        {}
		friend istream &operator>>(istream &is,section &rhs)
		{
			while(!is.eof() && is.get()!='[');
			is>>rhs.section_name;
			rhs.section_name.erase(find(rhs.section_name.begin(),rhs.section_name.end(),']'),rhs.section_name.end());

			string line,sp;
			getline(is,line);
			istringstream ss(line);
			ss>>sp>>rhs.section_desc;
			while(getline(is,line) && *line.c_str()!='[' && !is.eof())
			{
				string name,equ,value,comment;
				if(line=="")
					continue;
				line.erase(find(line.begin(),line.end(),'\r'),line.end());//remove 0xA!DOS/Win text signature
				if(find(line.begin(),line.end(),';')!=line.end())
					copy(find(line.begin(),line.end(),';')+1,line.end(),back_inserter(comment));
				line.erase(find(line.begin(),line.end(),';'),line.end());//comments
				if(line=="")
					continue;
				line.insert(find(line.begin(),line.end(),'='),' ');
				line.insert(find(line.begin(),line.end(),'=')+1,' ');

				istringstream ss(line);
				ss>>name>>equ>>value;
				rhs.keymap[name].value=value;
				rhs.keymap[name].comment=comment;
				//pos=is.tellg(); BS.g++ implement.Such a simple pointer return can cause a move of file pointer.
			}
			if(!is.eof())
				is.seekg(-80,ios_base::cur);//BS. again. Not follow the standardization behavior of moving back *charactors*.
			return is;
		}
		friend ostream &operator<<(ostream &os,const section &rhs)
		{
			os<<"["<<rhs.section_name<<"]\t;"<<rhs.section_desc<<endl;
			for(configmap::const_iterator i=rhs.keymap.begin();i!=rhs.keymap.end();i++)
				os<<(*i).first<<" = "<<(*i).second.value<<" ; "<<(*i).second.comment<<endl;
			os<<endl;
			return os;
		}
		string name() const{
			return section_name;
		}
		string getString(char *name){
			return keymap[name].value;
		}
		bool getBool(char *name){
			return keymap[name].value=="true";
		}
		int getInt(char *name){
		    const char *t=keymap[name].value.c_str();
			return boost::lexical_cast<int>("0"+keymap[name].value);
		}
	};
	string name;
	std::map<string,section> sections;
	bool needwrite;
public:
	ini_parser(char *conf):name(conf),needwrite(false)
	{
        configmap configprop;
        configprop["path"].value=".";
		configprop["path"].comment="资源路径";
        configprop["setup"].value="true";
		configprop["setup"].comment="Bool;是否用setup.dat里的设置覆盖这里的对应设置";
        configprop["allow_memory"].value="";
		configprop["allow_memory"].comment="Bool;是否允许跨进程记忆最后存档";
        configprop["last"].value="";
		configprop["last"].comment="Int;最后载入的存档";
        section config("config",configprop);
        sections["config"]=config;
        configmap debugprop;
        debugprop["resource"].value="mkf";
        debugprop["resource"].comment="资源使用方式;mkf/filesystem";
        debugprop["allow_frozen"].value="true";
        debugprop["allow_frozen"].comment="允许冻结启动/停止;true/false";
        section debug("debug",debugprop);
        sections["debug"]=debug;
        configmap displayprop;
        displayprop["height"].value="320";
		displayprop["height"].comment="粒度;320x200正整数倍.";
        displayprop["width"].value="200";
        displayprop["scale"].value="none";
		displayprop["scale"].comment="none, 2x; etc.";
        displayprop["fullscreen"].value="false";
        displayprop["fullscreen"].value="Bool ;全屏";
        section display("display",displayprop);
        sections["display"]=display;
		configmap fontprop;
		fontprop["type"].value="truetype";
		fontprop["type"].comment="truetype: ttf/ttc; fon: wor16.fon";
		fontprop["path"].value="%WINDIR%/fonts/mingliu.ttc";
		section font("font",fontprop);
		sections["font"]=font;
		configmap musicprop;
		musicprop["type"].value="rix";
		musicprop["type"].comment="rix/mid/foreverCD,+gameCD,+gameCDmp3，或任意混合。foreverCD指永恒回忆录之FM曲集";
		musicprop["volume"].value="100";
		musicprop["volume"].comment="0-100;音量";
		section music("music",musicprop,"与setup正交");
		sections["music"]=music;
		configmap keyprop;
		keyprop["west"].value="";
		keyprop["north"].value="";
		keyprop["east"].value="";
		keyprop["south"].value="";
		section keymap("keymap",keyprop,"行走键盘定义；与setup正交");
		sections["keymap"]=keymap;

		ifstream ifs(name.c_str());
		if(ifs.is_open() )
			while(!ifs.eof()){
				section s;
				ifs>>s;
				sections[s.name()]=s;
			}
		else
			needwrite=true;
	}
	void write(){
		ofstream ofs(name.c_str(),ios_base::out|ios_base::trunc);
		for(std::map<string,section>::const_iterator i=sections.begin();i!=sections.end();i++)
			ofs<<(*i).second;
		ofs.flush();
	}
	~ini_parser()
	{
		if(needwrite)
			write();
	}
	section &getSection(char *sec){
		return sections[string(sec)];
	}
};

ini_parser conf("palx.conf");

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
void close_button_handler(void)
{
	//if(!yesno_dialog()) return;
	running=false;
	remove_timer();
}
END_OF_FUNCTION(close_button_handler)

#if defined (WIN32)
#define FONT_PATH getenv("WINDIR")
#define FONT_FILE "\\fonts\\mingliu.ttc"
#define LOCALE "chinese"
#define PATH_SEP '\\'
#else
#define FONT_PATH ""
#define FONT_FILE "/usr/share/fonts/truetype/arphic/uming.ttf" //ubuntu gutsy gibbon;other distribution has other position but I don't know the unified method to determine it.
#define LOCALE "BIG5"
#define PATH_SEP '/'
#endif

string path_root;

int global_init(char *)
{
	if(conf.getSection("debug").getString("resource")=="mkf")
	{
		de_none			=bind(denone_file,_1,_4);
		de_mkf			=bind(demkf_ptr,_1,_2,_4);
		de_mkf_yj1		=bind(deyj1_ptr,	bind(demkf_file,		_1,_2,_4,_6,_7,_8),_4);
		de_mkf_mkf_yj1	=bind(deyj1_ptr,	bind(demkf_sp,bind(demkf_file,_1,_2,_4,_6,_7,_8),_3,_4,_5),_4);
		de_mkf_smkf	=bind(desmkf_ptr,	bind(demkf_file,		_1,_2,_4,_6,_7,_8),_3,_4,_5);
		de_mkf_yj1_smkf=bind(desmkf_ptr,	bind(deyj1_sp,		bind(demkf_file,_1,_2,_4,_6,_7,_8),_4),_3,_4,_5);
	}else if(conf.getSection("debug").getString("resource")=="filesystem"){
		de_none			=bind(denone_file,_1,_4);
		de_mkf			=bind(defile_dir,_1,_2,_4,_5);
		de_mkf_yj1		=de_mkf;
		de_mkf_mkf_yj1	=bind(defile_sp,	bind(dedir_dir,_1,_2),_3,_4,_5);
		de_mkf_smkf	=de_mkf_mkf_yj1;
		de_mkf_yj1_smkf=de_mkf_mkf_yj1;
	}

	path_root=conf.getSection("config").getString("path");
	SETUP.set(path_root+"/SETUP.DAT",de_none);
	PAT.set	(path_root+"/PAT.MKF"	,de_mkf);
	//MIDI.set(path_root+"/MIDI.MKF"	,de_mkf);
	VOC.set	(path_root+"/VOC.MKF"	,de_mkf);
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

	CARD=(conf.getSection("display").getBool("fullscreen")?GFX_AUTODETECT_FULLSCREEN:GFX_AUTODETECT_WINDOWED);

	//allegro init
	allegro_init();
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
	install_timer();
	install_keyboard();
	keyboard_lowlevel_callback = key_watcher;
	install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
	set_gfx_mode(CARD,conf.getSection("display").getInt("height"),conf.getSection("display").getInt("width"),0,0);
	set_color_depth(8);

	if(conf.getSection("font").getString("type")=="truetype")
	{
        alfont_init();
        char fontpath[100];
        sprintf(fontpath,"%s%s",FONT_PATH,FONT_FILE);
        ttfont::glb_font=alfont_load_font(fontpath);
        alfont_set_language(ttfont::glb_font, LOCALE);
        alfont_set_convert(ttfont::glb_font, TYPE_WIDECHAR);
        alfont_text_mode(-1);
        alfont_set_font_background(ttfont::glb_font, FALSE);
        alfont_set_char_extra_spacing(ttfont::glb_font,1);
        alfont_set_font_size(ttfont::glb_font,16);
	}else if(conf.getSection("font").getString("type")=="fon")
	{}

	randomize();
	playrix::set((path_root+"/MUS.MKF").c_str());

	int save=0;
	if(conf.getSection("config").getBool("allow_memory"))
		save=conf.getSection("config").getInt("last");
	return save;
}

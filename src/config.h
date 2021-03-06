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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#if defined(__MINGW32__) || defined(__DJGPP__) //mingw hack. unknown bug on the ifstream tellg that makes it actually moved the pointer.
#   define getpos()
#   define retpos() is.seekg(-80,std::ios_base::cur)
#else
#   define getpos() pos=is.tellg()
#   define retpos() is.seekg(pos,std::ios_base::beg)
#endif

std::string env_expand(std::string);
class ini_parser
{
	friend class global_settings;
	class section{
		std::string section_name;
		std::string section_desc;
	public:
		struct correspond{
			std::string value,comment;
		};
		typedef std::map<std::string,correspond> configmap;
		configmap keymap;
        section(){}
        section(std::string name,std::map<std::string,correspond> map,std::string description=""):section_name(name),section_desc(description),keymap(map)
        {}
		friend std::istream &operator>>(std::istream &is,section &rhs)
		{
			std::streampos pos;
			while(!is.eof() && is.get()!='[');
			is>>rhs.section_name;
			rhs.section_name.erase(find(rhs.section_name.begin(),rhs.section_name.end(),']'),rhs.section_name.end());

			std::string line,sp;
			getline(is,line);
			std::istringstream ss(line);
			ss>>sp>>rhs.section_desc;
			while(getline(is,line) && *line.c_str()!='[' && !is.eof())
			{
				std::string name,equ,value,comment;
				if(line=="")
					continue;
				line.erase(find(line.begin(),line.end(),'\r'),line.end());//remove 0xA!DOS/Win text signature
				if(find(line.begin(),line.end(),';')!=line.end()){
					std::string _comment;
					copy(find(line.begin(),line.end(),';')+1,line.end(),back_inserter(_comment));
					std::istringstream ss(_comment);
					ss>>comment;
				}
				line.erase(find(line.begin(),line.end(),';'),line.end());//comments
				if(find(line.begin(),line.end(),'=')==line.end())
					continue;
				line.insert(find(line.begin(),line.end(),'='),' ');
				line.insert(find(line.begin(),line.end(),'=')+1,' ');

				std::istringstream ss(line);
				ss>>name>>equ>>value;
				rhs.keymap[name].value=value;
				rhs.keymap[name].comment=comment;
				getpos();
			}
			if(!is.eof())
				retpos();
			return is;
		}
		friend std::ostream &operator<<(std::ostream &os,const section &rhs)
		{
			os<<"["<<rhs.section_name<<"]\t;"<<rhs.section_desc<<std::endl;
			for(configmap::const_iterator i=rhs.keymap.begin();i!=rhs.keymap.end();i++)
				os<<(*i).first<<" = "<<(*i).second.value<<" ; "<<(*i).second.comment<<std::endl;
			os<<std::endl;
			return os;
		}
		std::string name() const{
			return section_name;
		}
		std::string get(const char *name,const std::string&){
			return env_expand(keymap[name].value);
		}
		bool get(const char *name,bool){
			return keymap[name].value=="true";
		}
		int hexgetint(std::string i)
		{
			std::istringstream in(i);
			int x;
			in>>std::hex>>x;
			return x;
		}
		int get(const char *name,int){
			try
			{
				return boost::lexical_cast<int>(keymap[name].value);
			}
			catch(boost::bad_lexical_cast &)
			{
				try
				{
					return hexgetint(keymap[name].value);
				}
				catch(...)
				{
					return 0;
				}
			}
			catch(...)
			{
				return 0;
			}
		}
		double get(const char *name,double){
			try
			{
				return boost::lexical_cast<double>(keymap[name].value);
			}
			catch(boost::bad_lexical_cast &)
			{
				try
				{
					return hexgetint(keymap[name].value);
				}
				catch(...)
				{
					return 0;
				}
			}
			catch(...)
			{
				return 0;
			}
		}
		void set(const char *name,const std::string &val){
			keymap[name].value=val;
		}
		void set(const char *name,bool val){
			keymap[name].value=(val?"true":"false");
		}
		void set(const char *name,int val){
			keymap[name].value=boost::lexical_cast<std::string>(val);
		}
		void set(const char *name,double val){
			keymap[name].value=boost::lexical_cast<std::string>(val);
		}
	};
	std::string name;
	std::map<std::string,section> sections;
	bool needwrite;
public:
	ini_parser(const char *conf,bool);
	void write();
	~ini_parser();
	section &getSection(const char *sec){
		return sections[std::string(sec)];
	}
	template<typename T>
	void set(const char *sec,const char *name,T val)
	{
	    needwrite=true;
        getSection(sec).set(name,val);
	}
};

class luabinding;
class global_settings{
	luabinding *m_lua;
	ini_parser m_conf;
	int m_save;
	static int argc;
	static char **argv;
	static global_settings *instance_ptr;
public:
	std::string sfx_file;
	global_settings(int,char *[]);
	~global_settings();
	void display_setup(bool =true);
	template<typename T>
	T get(const char *sec,const char *name)
	{
		return m_conf.getSection(sec).get(name,T());
	}
	template<typename T>
	void set(const char *sec,const char *name,T val)
	{
		m_conf.set(sec,name,val);
	}
	void init();
	int get_save(){
	    return m_save;
	}
	static void set_param(int _argc,char *_argv[]){
	    argc=_argc;
	    argv=_argv;
	}
	static global_settings *instance(){
	    if(instance_ptr==NULL)
            instance_ptr=new global_settings(argc,argv);
        return instance_ptr;
	}
	static void destroy(){
	    delete instance_ptr;
	}
};

class cut_msg_impl
{
	char *glb_buf;
	char buf[100];
public:
	cut_msg_impl(){}
	void set(const std::string &fname);
	~cut_msg_impl();
	char *operator()(int start,int end);
	char *operator()(int start);
};
extern cut_msg_impl objs,msges;

#endif //_CONFIG_H

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
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#ifdef __MINGW32__  //mingw hack. unknown bug on the ifstream tellg that makes it actually moved the pointer.
#   define getpos()
#   define retpos() is.seekg(-80,std::ios_base::cur)
#else
#   define getpos() pos=is.tellg()
#   define retpos() is.seekg(pos,std::ios_base::beg)
#endif

class ini_parser
{
	friend class global_init;
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
				if(find(line.begin(),line.end(),';')!=line.end())
					copy(find(line.begin(),line.end(),';')+1,line.end(),back_inserter(comment));
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
		std::string get(char *name,std::string){
			return keymap[name].value;
		}
		bool get(char *name,bool){
			return keymap[name].value=="true";
		}
		int get(char *name,int){
		    const char *t=keymap[name].value.c_str();
			return boost::lexical_cast<int>("0"+keymap[name].value);
		}
		void set(char *name,const std::string &val){
			keymap[name].value=val;
		}
		void set(char *name,bool val){
			keymap[name].value=(val?"true":"false");
		}
		void set(char *name,int val){
			keymap[name].value=boost::lexical_cast<std::string>(val);
		}
	};
	std::string name;
	std::map<std::string,section> sections;
	bool needwrite;
public:
	ini_parser(char *conf);
	void write();
	~ini_parser();
	section &getSection(char *sec){
		return sections[std::string(sec)];
	}
	template<typename T>
	void set(char *sec,char *name,T val)
	{
	    needwrite=true;
        getSection(sec).set(name,val);
	}
};

class global_init{
	ini_parser conf;
public:
	std::string sfx_file;
	global_init(char *name);
	template<typename T>
	T get(char *sec,char *name)
	{
		return conf.getSection(sec).get(name,T());
	}
	template<typename T>
	void set(char *sec,char *name,T val)
	{
		conf.set(sec,name,val);
	}
	int operator()();
};

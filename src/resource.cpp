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
#include "resource.h"

decoder_func de_none,de_mkf,de_mkf_yj1,de_mkf_mkf_yj1,de_mkf_smkf,de_mkf_yj1_smkf;

long cached_res::_len;
bool cached_res::_decoded;

void cached_res::set(const std::string &filename,decoder_func &func)
{
	file=filename;
	setdecoder(func);
}
cached_res::~cached_res(){
	clear();
}
decoder_func cached_res::setdecoder(decoder_func &func)
{
	clear();
	decoder_func old_decoder=decoder;
	decoder=func;
	return old_decoder;
}
uint8_t *cached_res::decode(int n,int n2,bool &decoded,long &length)
{
	decoded=false;
	std::pair<int,int> pos(n,n2);
	cache_type::iterator i=cache.find(pos);
	int slices;
	if(i==cache.end()){
		bool buf_decoded=false;
		boost::shared_array<uint8_t> buf;
		long size=0;
		if(buf_cache.find(n)!=buf_cache.end())
			buf_decoded=true,
			buf=buf_cache[n],
			size=buf_sizes[n];
		cache[pos]=decoder(file.c_str(),n,n2,length,slices,buf,size,buf_decoded);
		splits[n]=slices;
		buf_cache[n]=buf;
		buf_sizes[n]=size;
	}else
		decoded=true;
	return cache[pos];
}
int cached_res::slices(int n)
{
	decode(n);
	return splits[n];
}
uint8_t *cached_res::decode(int n,bool &decoded,long &length)
{
	return decode(n,0,decoded,length);
}
uint8_t *cached_res::decode(int n,long &length)
{
	bool c;
	return decode(n,0,c,length);
}
uint8_t *cached_res::decode(long &length)
{
    bool c;
    return decode(0,0,c,length);
}
void cached_res::clear(){
	for(cache_type::iterator i=cache.begin();i!=cache.end();i++)	delete i->second;
	cache.clear();
}
void cached_res::clear(int n, int n2){
	cache_type::iterator iter=cache.find(std::pair<int,int>(n,n2));
	delete iter->second;
	cache.erase(iter);
}

namespace Pal{
	cached_res MIDI,SFX,MAP,GOP,RNG,DATA,SSS,BALL,RGM,FBP,F,FIRE,ABC,MGO,PAT,SETUP;
}

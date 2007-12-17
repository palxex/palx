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
#include "pallib.h"

#include <boost/shared_array.hpp>

using namespace boost;
using namespace Pal::Tools;
namespace{
	uint8_t *denone(FILE *fp,long &len)
	{
		int32_t length;
		fseek(fp,0,SEEK_END);
		length=ftell(fp);
		rewind(fp);
		uint8_t *buf=new uint8_t[length];
		fread(buf,length,1,fp);
		len=length;
		return buf;
	}
	uint8_t *demkf_t(FILE *fp,int n,long &len)
	{
		int32_t offset=n*4,length;
		fseek(fp,offset,SEEK_SET);
		fread(&offset,4,1,fp);
		fread(&length,4,1,fp);length-=offset;
		fseek(fp,offset,SEEK_SET);
		uint8_t *buf=new uint8_t[length];
		fread(buf,length,1,fp);
		len=length;
		return buf;
	}
	shared_array<uint8_t> demkf(FILE *fp,int n,long &len)
	{
		return shared_array<uint8_t>(demkf_t(fp,n,len));
	}
	shared_array<uint8_t> demkf_impl(shared_array<uint8_t> src,int n,long &len)
	{
		uint32_t *usrc=(uint32_t *)src.get();
		int32_t length=usrc[n+1]-usrc[n];
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n],length);
		len=length;
		return shared_array<uint8_t>(buf);
	}
	uint8_t *desmkf(shared_array<uint8_t> src,int n,long &len)//todo:算法不完整
	{
		uint16_t *usrc=(uint16_t *)src.get();
		int files=usrc[0];
		int16_t length;
		if(n == files - 1 || (n == files - 2 && usrc[files-1] == 0) )
			length=(int16_t)len-usrc[n]*2;
		else
			length=(usrc[n+1]-usrc[n])*2;
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n]*2,length);
		len=length;
		return buf;
	}
	uint8_t *deyj1(shared_array<uint8_t> src,long &len)
	{
		void *dst;
		uint32_t length;
		DecodeYJ1(src.get(),dst,(uint32&)length);
		len=length;
		return (uint8_t*)dst;
	}
	shared_array<uint8_t> deyj1_t(shared_array<uint8_t> src,long &len)
	{
		return shared_array<uint8_t>(deyj1(src,len));
	}
}
decoder_func de_none		=bind(denone,_1,_4);
decoder_func de_mkf			=bind(demkf_t,_1,_2,_4);
decoder_func de_mkf_yj1		=bind(deyj1,	bind(demkf,		_1,_2,_4),_4);
decoder_func de_mkf_mkf_yj1	=bind(deyj1,	bind(demkf_impl,bind(demkf,_1,_2,_4),_3,_4),_4);
decoder_func de_mkf_smkf	=bind(desmkf,	bind(demkf,		_1,_2,_4),_3,_4);
decoder_func de_mkf_yj1_smkf=bind(desmkf,	bind(deyj1_t,		bind(demkf,_1,_2,_4),_4),_3,_4);

long cached_res::_len;
bool cached_res::_decoded;

cached_res::cached_res(const char *filename,decoder_func &func):
	file(filename),
	fp(fopen(filename,"rb"))
{
	if(!fp)
		exit(-1);
	setdecoder(func);
}
cached_res::~cached_res(){
	fclose(fp);
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
	if(i==cache.end())
		cache[pos]=decoder(fp,n,n2,length);
	else
		decoded=true;
	return cache[pos];
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

cached_res ABC("ABC.MKF" ,de_mkf_yj1);
cached_res MIDI("MIDI.MKF",de_mkf);
cached_res VOC("VOC.MKF" ,de_mkf);
cached_res MAP("MAP.MKF" ,de_mkf_yj1);
cached_res GOP("GOP.MKF" ,de_mkf_smkf);
cached_res RNG("RNG.MKF" ,de_mkf_mkf_yj1);
cached_res DATA("DATA.MKF",de_mkf);
cached_res SSS("SSS.MKF" ,de_mkf);
cached_res BALL("BALL.MKF",de_mkf_smkf);
cached_res RGM("RGM.MKF" ,de_mkf);
cached_res FBP("FBP.MKF" ,de_mkf_yj1);
cached_res F("F.MKF"   ,de_mkf_yj1);
cached_res FIRE("FIRE.MKF",de_mkf_yj1);
cached_res MGO("MGO.MKF" ,de_mkf_yj1);
cached_res PAT("PAT.MKF" ,de_mkf);
cached_res SETUP("SETUP.MKF",de_none);

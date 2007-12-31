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
#pragma warning(disable: 4819)
#ifndef RESOURCE_H
#define RESOURCE_H

#include "structs.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <cstdio>
#include <string>
#include <map>

class cached_res;
typedef boost::function<uint8_t *(FILE *,int,int,long&)> decoder_func;
extern decoder_func de_mkf,de_mkf_yj1,de_mkf_mkf_yj1,de_mkf_smkf,de_mkf_yj1_smkf;

class cached_res{
	std::string file;
	FILE *fp;
	decoder_func decoder;
	typedef std::map<std::pair<int,int>,uint8_t *> cache_type;
	cache_type cache;
public:
	static long _len;static bool _decoded;
	cached_res(const char *filename,decoder_func &d);
	~cached_res();
	decoder_func setdecoder(decoder_func &);
	uint8_t *decode(int,int,bool& =_decoded,long& =_len);
	uint8_t *decode(int,bool& ,long& =_len);
	uint8_t *decode(int,long& =_len);
	uint8_t *decode(long& =_len);
	void clear();
	void clear(int n,int n2);
};
extern cached_res ABC,VOC,MAP,GOP,RNG,DATA,SSS,BALL,RGM,FBP,F,FIRE,ABC,MGO,PAT,SETUP;

#endif

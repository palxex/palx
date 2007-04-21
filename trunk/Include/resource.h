#pragma warning(disable: 4819)
#ifndef RESOURCE_H
#define RESOURCE_H

#include "structs.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_array.hpp>

#include <cstdio>
#include <string>
#include <map>

class cached_res;
typedef boost::function<boost::shared_array<uint8_t> (FILE *,int,int,long&)> decoder_func;
extern decoder_func de_mkf,de_mkf_yj1,de_mkf_mkf_yj1,de_mkf_smkf,de_mkf_yj1_smkf;
extern long _len;

class cached_res{
	FILE *fp;
	decoder_func decoder;
	typedef std::map<std::pair<int,int>,boost::shared_array<uint8_t> > cache_type;
	cache_type cache;
public:
	cached_res(const char *filename,decoder_func &d);
	~cached_res();
	decoder_func setdecoder(decoder_func &);
	boost::shared_array<uint8_t> decode(int,int,long& =_len);
};
extern cached_res ABC,VOC,MAP,GOP,RNG,DATA,SSS,BALL,RGM,FBP,F,FIRE,ABC,MGO,PAT;

#endif
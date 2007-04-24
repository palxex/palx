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
extern long _len;

class cached_res{
	std::string file;
	FILE *fp;
	decoder_func decoder;
	typedef std::map<std::pair<int,int>,uint8_t *> cache_type;
	cache_type cache;
	bool changed;
public:
	cached_res(const char *filename,decoder_func &d);
	~cached_res();
	decoder_func setdecoder(decoder_func &);
	uint8_t *decode(int,int,long& =_len);
	uint8_t *decode(int,long& =_len);
	void clear();
	void clear(int n,int n2);
};
extern cached_res ABC,VOC,MAP,GOP,RNG,DATA,SSS,BALL,RGM,FBP,F,FIRE,ABC,MGO,PAT;

#endif
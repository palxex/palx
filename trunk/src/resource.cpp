#include "resource.h"
#include "pallib.h"

using namespace boost;
using namespace Pal::Tools;
namespace{
	shared_array<uint8_t> demkf(FILE *fp,int n,long &len)
	{
		int32_t offset=n*4,length;
		fseek(fp,offset,SEEK_SET);
		fread(&offset,4,1,fp);
		fread(&length,4,1,fp);length-=offset;
		fseek(fp,offset,SEEK_SET);
		uint8_t *buf=new uint8_t[length];
		fread(buf,length,1,fp);
		len=length;
		return shared_array<uint8_t>(buf);
	}
	shared_array<uint8_t> demkf_impl(shared_array<uint8_t> &src,int n,long &len)
	{
		uint32_t *usrc=(uint32_t *)src.get();
		int32_t length=usrc[n+1]-usrc[n];
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n],length);
		len=length;
		return shared_array<uint8_t>(buf);
	}
	shared_array<uint8_t> desmkf(shared_array<uint8_t> &src,int n,long &len)//todo:算法不完整
	{
		uint16_t *usrc=(uint16_t *)src.get();
		int16_t length=(usrc[n+1]-usrc[n])*2;
		uint8_t *buf=new uint8_t[length];
		memcpy(buf,src.get()+usrc[n]*2,length);
		len=length;
		return shared_array<uint8_t>(buf);
	}
	shared_array<uint8_t> deyj1(shared_array<uint8_t> &src,long &len)
	{
		void *dst;
		uint32_t length;
		DecodeYJ1(src.get(),dst,(uint32&)length);
		len=length;
		return shared_array<uint8_t>((uint8_t*)dst);
	}
}
decoder_func de_mkf			=bind(demkf,_1,_2,_4);
decoder_func de_mkf_yj1		=bind(deyj1,	bind(demkf,		_1,_2,_4),_4);
decoder_func de_mkf_mkf_yj1	=bind(deyj1,	bind(demkf_impl,bind(demkf,_1,_2,_4),_3,_4),_4);
decoder_func de_mkf_smkf	=bind(desmkf,	bind(demkf,		_1,_2,_4),_3,_4);
decoder_func de_mkf_yj1_smkf=bind(desmkf,	bind(deyj1,		bind(demkf,_1,_2,_4),_4),_3,_4);

long _len;

cached_res::cached_res(const char *filename,decoder_func &func):
	fp(fopen(filename,"rb"))
{
	if(!fp)
		exit(-1);
	setdecoder(func);
}
cached_res::~cached_res(){
	fclose(fp);
}
decoder_func cached_res::setdecoder(decoder_func &func)
{
	decoder_func old_decoder=decoder;
	decoder=func;
	return old_decoder;
}
boost::shared_array<uint8_t> cached_res::decode(int n,int n2,long &length)
{
	std::pair<int,int> pos(n,n2);
	cache_type::iterator i=cache.find(pos);
	if(i==cache.end())
		cache[pos]=decoder(fp,n,n2,length);
	return cache[pos];
}

cached_res ABC("abc.mkf" ,de_mkf_yj1_smkf);
cached_res MIDI("midi.mkf",de_mkf);
cached_res VOC("voc.mkf" ,de_mkf);
cached_res MAP("map.mkf" ,de_mkf_yj1);
cached_res GOP("gop.mkf" ,de_mkf_smkf);
cached_res RNG("rng.mkf" ,de_mkf_mkf_yj1);
cached_res DATA("data.mkf",de_mkf);
cached_res SSS("sss.mkf" ,de_mkf);
cached_res BALL("ball.mkf",de_mkf_smkf);
cached_res RGM("rgm.mkf" ,de_mkf_smkf);
cached_res FBP("fbp.mkf" ,de_mkf_yj1);
cached_res F("f.mkf"   ,de_mkf_yj1_smkf);
cached_res FIRE("fire.mkf",de_mkf_yj1_smkf);
cached_res MGO("mgo.mkf" ,de_mkf_yj1_smkf);
cached_res PAT("pat.mkf" ,de_mkf);
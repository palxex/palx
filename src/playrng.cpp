#include "resource.h"
#include "pallib.h"
#include "timing.h"
#include "internal.h"

int RNG_num;
void play_RNG(int begin,int end,int gap)
{
	decoder_func olddecoder=RNG.setdecoder(de_mkf);
	int total_clips=((uint32_t *)RNG.decode(RNG_num))[0]/4-2;
	RNG.setdecoder(olddecoder);
	BITMAP *cache=create_bitmap(320,200);
	blit(screen,cache,0,0,0,0,cache->w,cache->h);
	for(int i=begin;i<std::min(total_clips,end);i++){
		Pal::Tools::DecodeRNG(RNG.decode(RNG_num,i),cache->dat);
		blit(cache,screen,0,0,0,0,cache->w,cache->h);
		pal_fade_in(1);
		wait(100/gap);
	}
}



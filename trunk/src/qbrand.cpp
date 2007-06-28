#include "config.h"

#include <time.h>

uint64_t seed;
uint32_t a=214013,c=2531011,d=16777216;

void randomize()
{
	time_t t;
	time(&t);
	double f=t;
	uint64_t u=*reinterpret_cast<uint64_t*>(&f);
	seed=((u>>48)^((u>>32)&0xffff))<<8;
}
float rnd0()
{
	seed=(seed*a+c)%d;
	return (float)seed/d;
}

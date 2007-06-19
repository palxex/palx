#include "config.h"

#if defined _MSC_VER
	#include <time.h>
#elif defined __GNUC__
	#include <sys/time.h>
#endif

uint64_t seed;
uint32_t a=214013,c=2531011,d=16777216;

void randomize()
{
	time_t t;
	time(&t);
	double f=*reinterpret_cast<double*>(&t);
	uint64_t u=*reinterpret_cast<uint64_t*>(&f);
	seed=t;((u>>48)^((u>>32)&0xffff))<<8;
}
float rnd0()
{
	seed=(seed*a+c)%d;
	return (float)seed/d;
}

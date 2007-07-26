#include "timing.h"
#include "allegdef.h"

void wait(uint8_t gap)
{
	time_interrupt_occurs=0;
	rest(gap*10);
}

void wait_key(uint8_t gap)
{
	clear_keybuf();
	time_interrupt_occurs=0;
	while(!keypressed())
		rest(10);
}

void wait_for_key()
{
	while(!keypressed())
		rest(10);
	clear_keybuf();
}

void delay(uint8_t gap)
{
	rest(gap);
}

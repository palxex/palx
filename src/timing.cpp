#include "timing.h"
#include "allegdef.h"

void wait(uint8_t gap)
{
	time_interrupt_occurs=0;
	while(time_interrupt_occurs<gap)
		rest(0);
}

void wait_key(uint8_t gap)
{
	clear_keybuf();
	time_interrupt_occurs=0;
	while(!keypressed() && time_interrupt_occurs<gap)
		rest(0);
}

void wait_for_key()
{
	while(!keypressed())
		rest(0);
	clear_keybuf();
}

void delay(uint8_t gap)
{
	rest(gap);
}

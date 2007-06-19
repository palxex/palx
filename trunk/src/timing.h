#ifndef TIMING_H
#define TIMING_H

#include "allegdef.h"

extern volatile uint8_t time_interrupt_occurs; 
extern void wait(uint8_t cycles);
extern void wait_key(uint8_t gap);
extern void wait_for_key();
extern void delay(uint8_t cycles);

#endif

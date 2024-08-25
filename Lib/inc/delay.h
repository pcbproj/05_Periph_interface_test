#ifndef DELAY_H
#define DELAY_H

#include "stm32f407xx.h"

extern uint32_t delay_us;
extern uint32_t delay_ms;


void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);




#endif

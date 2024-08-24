#ifndef DELAY_H
#define DELAY_H

#include "stm32f407xx.h"

uint32_t delay_us = 0;
uint32_t delay_ms = 0;


void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);




#endif

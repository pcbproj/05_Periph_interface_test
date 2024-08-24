#ifndef DELAY_H
#define DELAY_H

#include "stm32f407xx.h"

#define AHB1_FREQ_HZ				84000000
#define SYSTICK_TIMER_PERIOD_US		1	
#define SYSTICK_TIMER_CONST			AHB1_FREQ_HZ / 1000000 / (SYSTICK_TIMER_PERIOD_US)

#define COUNTER_1000_MS		1000	// 1000 ms for 1 second 


uint32_t delay_us = 0; // global variable for US timer counter for delay functions
uint32_t delay_ms = 0; // global variable for MS timer counter for delay functions


void Delay_us(uint32_t	us);
void Delay_ms(uint32_t	ms);



#endif 
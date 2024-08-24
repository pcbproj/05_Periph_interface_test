#include "delay.h"


void Delay_us(uint32_t	us){
	delay_us = 0;
	while(delay_us < us){
		__NOP();
	}; 
}



void Delay_ms(uint16_t ms){
	delay_ms = 0;
	while(delay_ms < ms){
		__NOP();
	}; 
}



#include "delay.h"



void Delay_us(uint32_t us){		// фунукция задержки на 1 мкс
	delay_us = 0;
	while(delay_us < us){
		__NOP();
	};
}



void Delay_ms(uint32_t ms){		// фунукция задержки на 1 мкс
	delay_us = 0;
	while(delay_ms < ms){
		__NOP();
	};
}




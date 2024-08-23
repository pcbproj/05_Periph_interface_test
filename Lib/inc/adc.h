#ifndef ADC_H
#define ADC_H

#include "stm32f4xx.h"


void ADC1_Init(void);
uint8_t	ADC1_StartConvert(void);
uint16_t ADC1_Read(void);



#endif
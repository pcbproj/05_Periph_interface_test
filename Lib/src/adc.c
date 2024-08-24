#include "adc.h"



void ADC1_Init(void){
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Включение тактирования АЦП
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Включение тактирования порта GPIOA
	
	GPIOA->MODER |= GPIO_MODER_MODE5; // GPIOA_PA5 в аналоговом режиме (ADC1_CH5)
	 
	ADC1->SMPR2 |= ADC_SMPR2_SMP5_0; // выбираем время конвертирования 15 тактов для канал 5
	
	ADC1->SQR1 = 0x00; // 1 conversion in cequence
	ADC1->SQR3 |= (5 << ADC_SQR3_SQ1_Pos); // channel 5 selected for first conversion in cequence
	ADC1->CR1 |= ADC_CR1_RES_1; // Selected 8-bit ADC sample
	ADC1->CR2 &= ADC_CR2_CONT;	// single conversion mode
	
	//ADC1->CR2 |= ADC_CR2_ADON; // ADC enabled
}



void ADC1_StartConversion(void){
	ADC1->CR2 |= ADC_CR2_ADON; // ADC enabled
}



uint16_t ADC1_Read(void){
	while(!(ADC1->SR & ADC_SR_EOC)){};	
	return ADC1->DR;
}

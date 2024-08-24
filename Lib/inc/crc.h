#ifndef CRC_H
#define CRC_H

#include "stm32f407xx.h"


//--------- параметры вычисления CRC для DS18B20 --------
#define CRC_POLYNOM		(uint8_t)0x31	// BIN = 1_0011_0001 берем младшие 8 бит
#define CRC_LEN_8_BITS		8



uint8_t CRC_Calc(uint8_t mass[], uint8_t mass_size, uint8_t POLY);


#endif




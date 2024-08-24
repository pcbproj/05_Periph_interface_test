#include "crc.h"




/*
	Алгоритм вычислени CRC:
	========================
	Циклически сдвигаем CRC и вычисляем бит shift_in_bit = CRC[7] XOR data_bit_in
	Если shift_in_bit == 1, то после сдвига выполняем еще (CRC xor POLY)
	Пока не кончатся биты в последовательности данных
	data_bit_in - это младший бит в байте. 
	В CRC в младший бит задвигаются байты входных данных начиная с младшего бита.
*/

uint8_t CRC_Calc(uint8_t mass[], uint8_t mass_size, uint8_t POLY){
	uint8_t crc = 0 , crc_out = 0;
	uint8_t in_data;
	uint8_t in_bits;
	for(uint8_t j = 0; j < mass_size; j++){
		in_data = mass[j];
		for(uint8_t i = 0; i < 8; i++){
			if(((crc & 0x80) >> 7) != (in_data & 0x01)){
				crc = crc << 1;
				crc = crc ^ POLY;
			}
			else{
				crc = crc << 1;
			}
			in_data = in_data >> 1;
		}
	}
	for(uint8_t i = 0; i < 8; i++){	// разворачиваем CRC биты в правильном порядке
		if(crc & (1 << i)) crc_out |= (1 << (7-i));
	}
	return crc_out;
}	
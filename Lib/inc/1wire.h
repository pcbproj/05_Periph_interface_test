#ifndef ONE_WIRE_H
#define ONE_WIRE_H

//#include "stdio.h"		// only for using printf()
#include "stm32f407xx.h"
#include "crc.h"
#include "delay.h"


// --------- 1 wire подстановки для лучшей читабельности кода -------
#define release_1wire()		(GPIOE -> BSRR |= GPIO_BSRR_BS2)	// для этого нужно настроить выход PE2 в режиме open-drain
#define pull_low_1wire()	(GPIOE -> BSRR |= GPIO_BSRR_BR2)
#define rx_mode_1wire()		(GPIOE -> MODER &= ~(GPIO_MODER_MODE2_0))
#define tx_mode_1wire()		(GPIOE -> MODER	|= GPIO_MODER_MODE2_0)
#define check_1wire()		(GPIOE -> IDR & GPIO_IDR_ID2)

//-------- 1 wire ROM commands -----------------
#define READ_ROM		0x33
#define MATCH_ROM		0x55
#define SKIP_ROM		0xCC
#define SEARCH_ROM		0xF0
#define ALARM_SEARCH	0xEC

//------- 1-wire memory command ---------------
#define READ_SCRATCH	0xBE
#define WRITE_SCRATCH	0x4E
#define COPY_SCRATCH	0x48
#define CONVERT_T		0x44
#define RECALL_E2		0xB8
#define READ_PWR		0xB4

//-------- 1-wire ERROR codes ---------------
#define OK_1WIRE		0
#define NO_DEVICE_1WIRE	1
#define CRC_ERR_1WIRE	2

//--------- параметры вычисления CRC для DS18B20 --------
#define CRC_POLYNOM		(uint8_t)0x31	// BIN = 1_0011_0001 берем младшие 8 бит
#define CRC_LEN_8_BITS		8

//-------- константы длин 1-Wire ------------
#define ROM64_BYTE_LEN				8
#define ROM64_BIT_LEN				64
#define BYTE_LEN					8
#define SCRATCH_BYTE_LEN			9


//------- константы для сканирования шины 1-Wire --------
#define ROM64_ZERO_BITS_CONDITION	0x01
#define ROM64_ONE_BITS_CONDITION	0x02
#define ROM64_DIFF_BITS_CONDITION	0x00
#define ROM64_NO_DEVICE_CONDITION	0x03
#define ROM64_BITS_CONDITION_MASK	0x03

#define MAX_1WIRE_DEVICES_NUMBER	128



void GPIO_1WireInit(void);
uint8_t Start_1wire(void);
void WriteByte_1wire(uint8_t byte_value);
uint8_t ReadByte_1wire(void);
uint8_t Read_ROM64(uint8_t *family_code, uint8_t ser_num[], uint8_t *crc);
uint8_t ReadScratchpad(uint8_t scratch_array[]);
uint8_t Convert_Temperature(void);

float Temperature_CalcFloat(uint16_t temper_in);	// calculate temperature value in float with sign
uint16_t Scratch_To_Temperature(uint8_t scratch_array[]);
float Temperature_CalcFloat(uint16_t temper_in);
uint8_t WriteScratch(uint8_t tx_array[]);

void WriteBit(uint8_t bit);
uint8_t ReadBit(void);

uint8_t ScanROM(uint8_t ROM64_array_prev[],	// массив uint8_t ROM_64[8] с предыдущим значением
				uint8_t ROM64_array[],		//  массив uint8_t ROM_64[8] с новым значением
				uint8_t branches[]			// массив с разночтениями: 1 - разночтение в позиции бита, 0 - нет разночтений.
				);

/*
	Функция поиска "правильной" единицы в массиве branches[] и выдает номер бита и байта ее позиции
*/
uint8_t FindOnesBranches( uint8_t branches[], 
						uint8_t ROM64[], 
						uint8_t *bit_num,
						uint8_t *byte_num );


/*
	функция формирования массива ROM64_Prev
*/
void PrevROM64_Assemble(uint8_t ROM64[],		// ROM64[] массив с текущими значениями ROM64
					uint8_t bit_num,		// номер бита разночтения
					uint8_t byte_num,		// номер байта разночтения
					uint8_t prevROM64[]		// массив с предыдущими битами до бита разночтения (младше), 
											// в бите разночтения стоит 1, а после него (старшие) все биты нулевые
					);



/***************** 
	Функция сканирования шины 1-wire для поиска всех устройств и их ROM64
	первым находит устройство с минимальным значением кода ROM64
	сортировка устройств по возрастанию кода ROM64 
	
	функция возвращает кол-во найденных устройств на шине 1-wire
	а в выходном параметре ROMs_array[][] сохраняются все идентификаторы устройств на шине 1-wire
****************/

uint8_t Scan_1Wire(uint8_t ROMs_array[MAX_1WIRE_DEVICES_NUMBER][ROM64_BYTE_LEN]);







#endif

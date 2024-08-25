#include "1wire.h"




void GPIO_1WireInit(void){
	//--------- GPIO settings for 1-WIRE PE2-pin ----------------
	GPIOE -> MODER	|= GPIO_MODER_MODE2_0;	// PE2 output mode
	GPIOE -> OTYPER |= GPIO_OTYPER_OT2;		// PE2 output open-drain

}



uint8_t Start_1wire(void){	// функция начала транзакции 1-wire 
	tx_mode_1wire();
	pull_low_1wire();	// Master reset pulse
	Delay_us(500);		
	release_1wire();
	rx_mode_1wire();	
	Delay_us(100);		// wait 100 us = 60 us pause + 40 us presence pulse
	if(check_1wire()){
		return 1;	// no presence pulse from 1-wire device
	}
	else{
	Delay_us(200);
		return 0;	// received presence pulse from 1-wire device
	}
}



void WriteByte_1wire(uint8_t byte_value){
	uint8_t write_bit_code = 0;
	uint8_t tmp = 0;
	tx_mode_1wire();
	for(uint8_t i = 0; i < 8; i++){
		write_bit_code = 0;
		tmp = (1 << i);
		write_bit_code = (byte_value & tmp);
		pull_low_1wire();
		Delay_us(5);
		if(write_bit_code != 0){
			release_1wire();
		}
		Delay_us(55);		// write bit slot time = 60 us
		release_1wire();
		Delay_us(2);	// пауза между битами 2 мкс
	}
	rx_mode_1wire();
}



uint8_t ReadByte_1wire(void){
	uint8_t rx_byte = 0;
	for(uint8_t i = 0; i < 8; i++){
		tx_mode_1wire();
		pull_low_1wire();
		Delay_us(2);
		release_1wire();
		rx_mode_1wire();
		Delay_us(12);
		if(check_1wire()){	// if received one
			rx_byte |= (1 << i);
		}
		Delay_us(60-14);	// read bit slot time = 60 us
		tx_mode_1wire();
		release_1wire();
		Delay_us(2);		// пауза между битами 2 мкс
	}
	return  rx_byte;
}


uint8_t Read_ROM64(uint8_t *family_code, uint8_t ser_num[], uint8_t *crc){
	uint8_t tmp_array[ROM64_BYTE_LEN];
	uint8_t crc_calculated = 0;
	uint8_t err_code = 0;
	if(!Start_1wire()){			// 1-wire device found
		WriteByte_1wire(READ_ROM);
		Delay_us(100);
		*family_code = ReadByte_1wire();
		tmp_array[0] = *family_code;
		for(uint8_t i = 0; i < 6; i++){
			ser_num[i] = ReadByte_1wire();
			tmp_array[i+1] = ser_num[i];
		}
		*crc = ReadByte_1wire();
		tmp_array[7] = *crc;
		//printf("================= \n");
		//printf("==== READ ROM 64 bits .... \n");
		//printf("==== SCRATCH = ");
		//for (uint8_t i = 0; i < ROM64_BYTE_LEN; i++){

		//	printf("0x%X ", tmp_array[i]);

		//}
		//printf("\n==== CRC Rx = 0x%X \n", tmp_array[7]);

		crc_calculated = CRC_Calc(tmp_array, 7, CRC_POLYNOM);
		//printf("==== CRC calculated = 0x%X \n" , crc_calculated);
		
		if(crc_calculated == tmp_array[7]) return OK_1WIRE;
		else return CRC_ERR_1WIRE;	// error ROM64 read  
	}
	else{
		return NO_DEVICE_1WIRE;			// error. 1-wire device are not found
	}

}





uint8_t ReadScratchpad(uint8_t scratch_array[]){
	uint8_t err_code = 0;
	uint16_t temp = 0;
	uint8_t scratch_tmp[SCRATCH_BYTE_LEN] = {};
	uint8_t crc_calculated = 0;

	if(!Start_1wire()){			// 1-wire device found
		WriteByte_1wire(SKIP_ROM);
		Delay_us(100);
		WriteByte_1wire(READ_SCRATCH);
		Delay_us(100);
		for(uint8_t i = 0; i < SCRATCH_BYTE_LEN; i++){	// read all 9 bytes from scratchpad
			scratch_tmp[i] = ReadByte_1wire();
			Delay_us(100);
		}		
		
		crc_calculated = CRC_Calc(scratch_tmp, (SCRATCH_BYTE_LEN - 1), CRC_POLYNOM);

		if(crc_calculated == scratch_tmp[SCRATCH_BYTE_LEN - 1]){
			for(uint8_t i = 0; i < SCRATCH_BYTE_LEN; i++) scratch_array[i] = scratch_tmp[i];
			return OK_1WIRE;
		}
		else{
			//printf("--- ERROR: Scratch Read CRC mismatch \n");
			return NO_DEVICE_1WIRE;
		}
						
		
	}
	else{
		return NO_DEVICE_1WIRE;
	}
}





uint8_t Convert_Temperature(void){
	uint8_t err_code = 0;
	uint16_t temp = 0;
	if(!Start_1wire()){			// 1-wire device found
		WriteByte_1wire(SKIP_ROM);
		Delay_us(100);
		WriteByte_1wire(CONVERT_T);
		Delay_us(100);
		return OK_1WIRE;
	}
	else{
		return NO_DEVICE_1WIRE;
	}
}


// TODO: Проверить эту функцияю!!!!
/*********
Функция выделяет из Scrathpad только температуру и выдает ее 
в 16-ти битном виде как она была считана
**********/
uint16_t Scratch_To_Temperature(uint8_t scratch_mem[]){
	uint16_t temper;
	temper = ((scratch_mem[1] << 8 )) + scratch_mem[0];
	return temper;

}

// TODO: Проверить эту функцияю!!!!
/*************
Функция вычисления температуры в дробном виде (float) со знаком.
Выдает значение со знаком
************/

float Temperature_CalcFloat(uint16_t temper_in){
	float temper_float;
	if(temper_in < 0x0800){	// если положительные температуры
		temper_float = (float)temper_in / 16;
	}
	else{	// если отрицательные температуры
		temper_float = ( 0 - (float)temper_in / 16 );
	}
	return temper_float;
}



uint8_t WriteScratch(uint8_t tx_array[]){	// write only 3 byties from array [0 1 2 ] will be writed
	if(!Start_1wire()){			// 1-wire device found
		WriteByte_1wire(SKIP_ROM);
		Delay_us(100);
		WriteByte_1wire(WRITE_SCRATCH);
		Delay_us(100);
		for(uint8_t i = 0; i < 3; i++){		// write only 3 byties from tx_array 
			WriteByte_1wire(tx_array[i]);
			Delay_us(100);
		}	
		Start_1wire();		// final reset pulse					
		return OK_1WIRE;
	}
	else{
		return NO_DEVICE_1WIRE;
	}
}



// функция сканирования шины 1wire для выделения ROM64 из нескольких устройств
uint8_t ScanROM(uint8_t ROM64_array_prev[],	// массив uint8_t ROM_64[8] с предыдущим значением
				uint8_t ROM64_array[],		//  массив uint8_t ROM_64[8] с новым значением
				uint8_t branches[]			// массив с разночтениями: 1 - разночтение в позиции бита, 0 - нет разночтений.
				){
	 
	uint8_t rx_bits = 0;
	uint8_t rx_byte_val = 0; 
	uint8_t byte_number = 0; 
	uint8_t bit_num = 0;
	uint8_t byte_branches = 0;
	uint8_t bit_mode_select = 0;

	if(!Start_1wire()){	
		WriteByte_1wire(SEARCH_ROM);
		Delay_us(100);
		
		for(uint8_t i = 0; i < ROM64_BIT_LEN; i++){
			byte_number = i / BYTE_LEN;
			bit_num = i % BYTE_LEN;
			rx_bits = 0x00;		// clear rx_bits

			if(bit_num == 0) {
				byte_branches = 0;	// clear byte_branches for next byte
				rx_byte_val = 0;	// clear rx byte value
			}

			if(ReadBit()) rx_bits |= (1 << 1);	// read bit value
			
			if(ReadBit()) rx_bits |= (1 << 0);	// read bit NOT_value
			
			bit_mode_select = rx_bits & ROM64_BITS_CONDITION_MASK;

			switch(bit_mode_select){
			case ROM64_ZERO_BITS_CONDITION:
				WriteBit(0);
				break;

			case ROM64_ONE_BITS_CONDITION:
				rx_byte_val |= (1 << bit_num);
				ROM64_array[byte_number] = rx_byte_val;
				WriteBit(1);
				break;

			case ROM64_DIFF_BITS_CONDITION:
				byte_branches |= (1 << bit_num);
				branches[byte_number] = byte_branches;
				if(ROM64_array_prev[byte_number] & (1 << bit_num)){	// send bit value from ROM64_array_prev in diffenent readed bit
					rx_byte_val |= (1 << bit_num);
					ROM64_array[byte_number] = rx_byte_val;
					WriteBit(1);	
				}
				else{
					WriteBit(0);	

				}
				break;

			case ROM64_NO_DEVICE_CONDITION:
				return NO_DEVICE_1WIRE;
				break;
			
			}	// switch (ROM64_BITS_CONDITION_MASK)
		
		}	// for ROM64_BITS_NUMBER

		return OK_1WIRE;
	}
	return NO_DEVICE_1WIRE;
}


/*
	Функция поиска "правильной" единицы в массиве branches[] и выдает номер бита и байта ее позиции
*/
uint8_t FindOnesBranches( uint8_t branches[], 
						uint8_t ROM64[], 
						uint8_t *bit_num,
						uint8_t *byte_num ){
	uint8_t branches_byte;

	for(uint8_t byte = 0; byte < ROM64_BYTE_LEN; byte++){
		
		branches_byte = branches[ROM64_BYTE_LEN - byte - 1];

		if(branches_byte != 0){	// если есть 1 в битах, т.е. ксли есть биты с разночтением в ROM64
			for(uint8_t bit = 0; bit < BYTE_LEN; bit++ ){
			
				if(branches_byte & (1 << (BYTE_LEN - bit - 1))){
					if(! (ROM64[ROM64_BYTE_LEN - byte - 1] & (1 << (BYTE_LEN - bit - 1)))){	// это новая ветка. 
						*bit_num = (BYTE_LEN - bit - 1);
						*byte_num = (ROM64_BYTE_LEN - byte - 1);
						
						return 0;	// branch found
					}
				}
			}	
		}
	}
	return 1;	// no branches found
}


/*
	функция формирования массива ROM64_Prev
*/
void PrevROM64_Assemble(uint8_t ROM64[],		// ROM64[] массив с текущими значениями ROM64
					uint8_t bit_num,		// номер бита разночтения
					uint8_t byte_num,		// номер байта разночтения
					uint8_t prevROM64[]		// массив с предыдущими битами до бита разночтения (младше), 
											// в бите разночтения стоит 1, а после него (старшие) все биты нулевые
					){
	uint8_t prevROM_byte = 0;

	for(uint8_t byte = 0; byte < ROM64_BYTE_LEN; byte++){
		if(byte < byte_num){
			prevROM64[byte] = ROM64[byte];	// копируем все байты младше byte_num в prevROM64[]
		}
		else {
			if (byte == byte_num){	// в байте byte_num копируем иты младше bit_num в байт byte_num
				
				prevROM64[byte] =  (ROM64[byte] | (1 << bit_num));
			}
			else{ 
				prevROM64[byte] = 0;	// старшие байты в prevROM64[] оставляем равными 0
			}
		}
	}
}




/***************** 
	Функция сканирования шины 1-wire для поиска всех устройств и их ROM64
	первым находит устройство с минимальным значением кода ROM64
	сортировка устройств по возрастанию кода ROM64 
	
	функция возвращает кол-во найденных устройств на шине 1-wire
	а в выходном параметре ROMs_array[][] сохраняются все идентификаторы устройств на шине 1-wire
****************/

uint8_t Scan_1Wire(uint8_t ROMs_array[MAX_1WIRE_DEVICES_NUMBER][ROM64_BYTE_LEN]){	// двумерный массив с найденными ROM64 устройств
	
	uint8_t dev_num = 0;
	
	uint8_t ROM64_Rx[ROM64_BYTE_LEN] = {};	// нужно хранить всю последовательность ROM-кодов
	
	uint8_t branch_bits[ROM64_BYTE_LEN] = {};	
	
	uint8_t ROM64_Prev[ROM64_BYTE_LEN] = {};
	
	uint8_t err = 0;

	uint8_t bit_number = 0 , byte_number = 0;
	uint8_t branch_found = 0;
	
	for (uint8_t i = 0; i < ROM64_BYTE_LEN; i++) ROM64_Prev[i] = 0;	// clear ROM64_Prev

	/***** Алгоритм сканирования шины 1-Wire ***********
	1. ScanROM() - получаем массивы ROM_rx[] и branch_bits[]
	2. поиск единицы в branch_bits[] начиная со старшего байта и старшего бита
		2.1 если нет единиц, то на шине было всего одно устройство и его ROM64 мы теперь знаем 
		2.2.если найдена единица в branch_bits[], то проверяем значение бита в этой же позиции в массиве ROM64_rx[].
			2.2.1. если в этой позиции 0, то это точка ветвления в 1 при следующем запуске ScanROM().
					Копируем с массив ROM64_Prev[] все биты до этой позиции (младше этой позиции), 
					а в эту позицию ставим 1. И в следующие позиции пишем нули.
			2.2.2. если в этой позиции 1. То это отработанный узел. 
				Ищем следующую единицу в branch_bits[] в меньших значащих битах. Возврат к п. 2.2.
	3. В массиве ROM_rx 

	*****************************************************/
	while(! branch_found ){

		err = ScanROM(ROM64_Prev, ROM64_Rx, branch_bits);

		if(! err) {
			
			for(uint8_t i = 0; i < ROM64_BYTE_LEN; i++){
				ROMs_array[dev_num][i] = ROM64_Rx[i];	// copy ROM64_RX into ROMs_array
			}
		
			dev_num++;
		
			// ищем бит разночтения ROM64. 
			branch_found = FindOnesBranches(branch_bits,			
								ROM64_Rx, 
								&bit_number,
								&byte_number
								);
			
			// если найден бит разночтения. 
			// То копируем в ROM_prev[] биты младше бита разночтения, 
			// бит разночтения ставим 1, старшие биты зануляем
			if (! branch_found) {
				PrevROM64_Assemble(ROM64_Rx,			// ROM64[] массив с текущими значениями ROM64
								bit_number,		// номер бита разночтения
								byte_number,		// номер байта разночтения
								ROM64_Prev		// массив ROM_prev, используется при сканировании на следующей итерации
								);
			}
			else{
				return dev_num;
			}
		}
		else{
			//printf("---- ERROR: 1-Wire devices not found \n");
			return 0;
		}	// if(! err)
	}  // while(! branch_found) 
	
	return dev_num;	
}



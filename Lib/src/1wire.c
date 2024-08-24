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
		printf("================= \n");
		printf("==== READ ROM 64 bits .... \n");
		printf("==== SCRATCH = ");
		for (uint8_t i = 0; i < ROM64_BYTE_LEN; i++){

			printf("0x%X ", tmp_array[i]);

		}
		printf("\n==== CRC Rx = 0x%X \n", tmp_array[7]);

		crc_calculated = CRC_Calc(tmp_array, 7, CRC_POLYNOM);
		printf("==== CRC calculated = 0x%X \n" , crc_calculated);
		
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
			printf("--- ERROR: Scratch Read CRC mismatch \n");
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



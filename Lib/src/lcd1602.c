/*
 * LCD module ver 1.1
 * for STM32F407 work with
 * LCD1602 LCD-indicator
 * author: Bortnikov A.Y.
 * 
 * 4-bit LCD bus data 
 * 
 * */

#include "lcd1602.h"





void LCD_GPIOInit(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	
	LCD_RS_PORT -> MODER |= ( 1 << RS_PIN_NUM * 2);	// pin for LCD_RS configured as output 
	LCD_RW_PORT -> MODER |= ( 1 << RW_PIN_NUM * 2);
	
	LCD_E_PORT -> MODER |= ( 1 << E_PIN_NUM * 2);	
	
	LCD_DB7_PORT -> MODER |= ( 1 << DB7_PIN_NUM * 2);	// pin for LCD_DB4 - DB7 configured as output 
	LCD_DB6_PORT -> MODER |= ( 1 << DB6_PIN_NUM * 2);
	LCD_DB5_PORT -> MODER |= ( 1 << DB5_PIN_NUM * 2);
	LCD_DB4_PORT -> MODER |= ( 1 << DB4_PIN_NUM * 2);
	
}


void LCD_DataPinsInput(void){
	LCD_DB7_PORT -> MODER &= ~( 1 << DB7_PIN_NUM * 2);	// pin for LCD_DB4 - DB7 configured as output 
	LCD_DB6_PORT -> MODER &= ~( 1 << DB6_PIN_NUM * 2);
	LCD_DB5_PORT -> MODER &= ~( 1 << DB5_PIN_NUM * 2);
	LCD_DB4_PORT -> MODER &= ~( 1 << DB4_PIN_NUM * 2);
}


void LCD_DataPinsOutput(void){
	LCD_DB7_PORT -> MODER |= ( 1 << DB7_PIN_NUM * 2);	// pin for LCD_DB4 - DB7 configured as output 
	LCD_DB6_PORT -> MODER |= ( 1 << DB6_PIN_NUM * 2);
	LCD_DB5_PORT -> MODER |= ( 1 << DB5_PIN_NUM * 2);
	LCD_DB4_PORT -> MODER |= ( 1 << DB4_PIN_NUM * 2);
}


/******
Функция собирает данные с выводов DB7-DB4 в младшую половину байта
*****/
uint8_t GPIO_ReadInputData(void){
	uint8_t rx_temp;
	if(CHECK_DB7()) rx_temp = 0x08;
	else rx_temp = 0x00;

	if(CHECK_DB6()) rx_temp |= 0x04;
	if(CHECK_DB5()) rx_temp |= 0x02;
	if(CHECK_DB4()) rx_temp |= 0x01;

	return rx_temp;
}


  
void LCD_Write4b( uint8_t data_com, uint8_t symbol, int half_bytes_number ){
	uint8_t tx_high_half_byte;
	
	E_LOW();
	RW_LOW();
	
	if(data_com == OP_DATA) RS_HIGH();
	else RS_LOW();
	tx_high_half_byte = symbol;

	for(uint8_t i = 0; i < half_bytes_number; i++){
		E_HIGH();
		//----- output half high byte data onto data-pins -----
		if(tx_high_half_byte & 0x80) DB7_HIGH();
		else DB7_LOW();
		
		if(tx_high_half_byte & 0x40) DB6_HIGH();
		else DB6_LOW();
		
		if(tx_high_half_byte & 0x20) DB5_HIGH();
		else DB5_LOW();
		
		if(tx_high_half_byte & 0x10) DB4_HIGH();
		else DB4_LOW();
		
		Delay_us(1);
		
		E_LOW();
		tx_high_half_byte = (symbol << 4);	 // shift low half byte into high half byte data

		Delay_us(1);
	}

	RS_LOW();

	Delay_us(1);
}




/*
return(7) - BusyFlag value
return(6:0) - AddressCounter value	
*/
uint8_t Read_BF_Addr( void ){
	uint8_t RxDataTemp;
	uint8_t RxData = 0x00;
	
	LCD_DataPinsInput();
	
	RW_HIGH();
	for(int i=0; i < 2; i++){
		E_HIGH();
		Delay_us(1);
		//----------------------------------
		RxDataTemp = GPIO_ReadInputData(); // read data
		//----- shift received half word into MSb position -----	
		RxData = ( RxDataTemp << ( 4*(1-i) ) );
		E_LOW();
		Delay_us(1);
	}
	
	E_LOW();
	LCD_DataPinsOutput();

	//GPIO_Write( LCD_PORT, 0x0000 );	 // clear bits A0, E and RW
	RS_LOW();
	RW_LOW();	
	return RxData;
}


void BF_Wait( void ){
	uint8_t bf_addr = 0xFF;
	while( ( bf_addr & BF_MASK ) != 0x00 ){ // check BF flag
		bf_addr = Read_BF_Addr();
	}
}

void ClearDisplay( void ){
	LCD_Write4b( (uint8_t)OP_COM, 0x01, WHOLE_BYTE );
	BF_Wait();	
}


void ReturnHome( void ){
	LCD_Write4b( (uint8_t)OP_COM, 0x02, WHOLE_BYTE );
	BF_Wait();	
	
}

 
/* EntryModeSet(); 
	ID_SH - data value
ID_SH(1) select cursor movement direction:
	ID_SH(1) = 1
	ID_SH(1) = 0
ID_SH(0) = 1 enables cursor movement when DDRAM is writed	
ID_SH(2) always must be = 1
other bits of ID_SH must be zeroes
*/
void EntryModeSet( uint8_t ID_SH ){
	LCD_Write4b( (uint8_t)OP_COM, ( ID_SH | 0x40 ), WHOLE_BYTE );
	BF_Wait();	
} 


/*Display_ON_OFF();
DCB(1:0) - cursor type selection:
	DCB(1:0) = 00 - no cursor at all
	DCB(1:0) = 01 - symbol blinking cursor
	DCB(1:0) = 10 - underline cursor, not blinking
	DCB(1:0) = 11 - underline cursor  and it blinking 
DCB(2) = 1 - display ON
DCB(2) = 0 - display OFF 	
*/
void Display_ON_OFF( uint8_t DCB ){
	LCD_Write4b( (uint8_t)OP_COM, (DCB | 0x08), WHOLE_BYTE );
	BF_Wait();	

}

/*
SC_RL(1:0) = 00 - not used
SC_RL(2) - select shift direction
	SC_RL(2) = 1 - right shift
	SC_RL(2) = 0 - left shift
SC_RL(3) - select what to be shifted
	SC_RL(3) = 1 - shift cursor
	SC_RL(3) = 0 - shift display
SC_RL(4) = 1 - always
*/
void CursorDisplayShift( uint8_t SC_RL ){
	LCD_Write4b( (uint8_t)OP_COM, (SC_RL | 0x10), WHOLE_BYTE );
	BF_Wait();	
}


/*
DL_P(0) = 0 - constant
DL_P(1) select coed page
	DL_P(1) = 1 - page 1
	DL_P(1) = 0 - page 0
DL_P(3:2) = 10 - always
DL_P(4) - select datawidth of interface
	DL_P(4) = 1 - 8 bit data width interface
	DL_P(4) = 0 - 4 bit data width interface
*/
void FunctionSet( uint8_t DL_P ){
	LCD_Write4b( (uint8_t)OP_COM, ( DL_P | 0x28 ), WHOLE_BYTE );
	BF_Wait();	
}

// used ONLY FOR LCD Initialization
void FunctionSet_HALF( uint8_t DL_P ){
	LCD_Write4b( (uint8_t)OP_COM, ( DL_P | 0x28 ), HIGH_HALF_BYTE );
}


/*
ADDR(6:0) - set DDRAM Address
ADDR(7) = 1 always	
*/
void Set_DDRAM_Addr( uint8_t ADDR ){
	LCD_Write4b( (uint8_t)OP_COM, ( ADDR | 0x80 ), WHOLE_BYTE );
	BF_Wait();	
}


//---- 4bit interface display init algorithm ------
void DisplayInit_4b( void ){
	const uint16_t Time_80us = 500;  
	
	/*
	wait for at least 20 ms
	*/
	for(int i=0; i<3; i++){
		FunctionSet_HALF( (uint8_t)( BUS_8_BITS | CODEPAGE_0 ) );
		delay_800ns( Time_80us );
	}
	
	FunctionSet_HALF( (uint8_t)( BUS_4_BITS | CODEPAGE_0 ) );
	delay_800ns( Time_80us );
	
	
	FunctionSet( (uint8_t)( BUS_4_BITS | CODEPAGE_1 ) );
	
	Display_ON_OFF( (uint8_t)( DISPLAY_OFF | CURSOR_TYPE_0 ) );
	
	ClearDisplay();
	
	EntryModeSet( (uint8_t)( EM_DISPSHIFT_OFF | EM_CURSHIFTRIGHT ) );
	
	Display_ON_OFF( (uint8_t)( DISPLAY_ON | CURSOR_TYPE_1 ) );
	
	
}


void LCD_WriteChar( uint8_t data ){
	LCD_Write4b( (uint8_t)OP_DATA, data, WHOLE_BYTE );
	BF_Wait();
}


uint8_t CharToLCD( unsigned char charact ){
	if( ( charact >= ASCII_RUS_A ) && ( charact <= ASCII_RUS_YA ) )
		return ( charact + RUS_LCD_ADD );
	else
		return charact;
}



void ConvertCharToLCD_Code( unsigned char *in_str, uint8_t *out_array, uint16_t len ){
	for( uint8_t i=0; i<len; i++ ){
		out_array[i] = CharToLCD( in_str[i] );
	}
}


void ShowArray( uint8_t *in_array ){
	for( uint8_t i=0; i<LCD_CHAR_NUM; i++ ){
		LCD_WriteChar( in_array[i] );
	}
}



void ShowScreen( uint8_t show_mass[LCD_LINE_NUM][LCD_CHAR_NUM] ){
	
	Set_DDRAM_Addr( 0x00 );
	ShowArray( show_mass[0] );
	Set_DDRAM_Addr( 0x40 );
	ShowArray( show_mass[1] );
	
}


unsigned char DigitToLCD( uint8_t digit ){
	return (unsigned char)( digit + SYMB_ZERO );
}


void ValueToLCDArray( uint8_t value, uint8_t *lcd_array, uint8_t len ){
	uint16_t devider = 1;
	
	for( uint8_t i=1; i<len; i++ ){
		devider *= 10;
	}
	
	for(uint8_t i=0; i<len; i++){
		lcd_array[i] = ( ( value / devider ) % 10 ) + SYMB_ZERO;
		devider /= 10;
	}
}

void IntToByteArray( uint32_t in_int, uint8_t *bytes_array ){
	for( uint8_t i = 0; i<4; i++ ){
		bytes_array[i] = ( uint8_t )( ( in_int >> i*8 ) & 0x000000FF );
	}
}

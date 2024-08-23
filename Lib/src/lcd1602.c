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




void delay_400ns( uint16_t n ){
	for(uint16_t i=0; i < n; i++){
		__NOP();
	}
}


void delay_800ns( uint16_t n ){
	for(uint16_t i=0; i < n; i++){
		delay_400ns(2);
	}
}


uint8_t RotateBits_8( uint8_t ByteIn ){
	uint8_t ByteOut = 0x00;
	
	for (int i = 0; i < 8; ++i){
		ByteOut = ByteOut | ( ( ByteIn >> i) & 1 ? 1 << (7 - i) : 0);
	}
	
	return ByteOut;
}


uint8_t RotateBits_4( uint8_t ByteIn ){
	uint8_t ByteOut = 0x00;
	
	for (int i = 0; i < 4; ++i){
		ByteOut = ByteOut | ( ( ByteIn >> i) & 1 ? 1 << (3 - i) : 0);
	}
	
	return ByteOut;
}



void LCD_GPIOInit(void){
	
	
}


void LCD_DataPinsInput(void){
	/*
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(LCD_PORT == GPIOA){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	}
	else if(LCD_PORT == GPIOB){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	}
	
	GPIO_InitStructure.GPIO_Pin = DB4 | DB5 | DB6 | DB7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(LCD_PORT, &GPIO_InitStructure);
	*/
}

  
void LCD_Write4b( uint8_t data_com, uint8_t symbol, int cycles ){
	uint16_t PortVal;
	uint8_t TempSymbolVal;
	uint16_t TempPortVal;
	
	GPIO_Write( LCD_PORT, 0x0000 );	 // clear bits A0, E and RW
	
	//- delay about 70 ns and rotate bits due wrong LCD connection ---------
	TempSymbolVal = RotateBits_8( symbol );
	//------------------------------------
	TempPortVal = ( (uint16_t)TempSymbolVal & 0x00FF );
	
	for(uint8_t i=0; i<cycles; i++){
		PortVal = ( ( ( ( TempPortVal ) & 0x000F ) << 8 ) );
		
		if( data_com == OP_DATA ){
			GPIO_SetBits( LCD_PORT, A0 );
			PortVal = ( PortVal | A0 );
		}
		GPIO_Write( LCD_PORT, PortVal );	
		GPIO_SetBits( LCD_PORT, E );
		//----- wait about 200 ns-----------
		delay_400ns(1);
		//----------------------------------
		GPIO_ResetBits( LCD_PORT, E );
		TempPortVal = ( ( (uint16_t)TempSymbolVal & 0x00FF ) >> 4 );
		
	}
	
	GPIO_ResetBits( LCD_PORT, A0 );
	delay_400ns(1);
}




/*
return(7) - BusyFlag value
return(6:0) - AddressCounter value	
*/
uint8_t Read_BF_Addr( void ){
	uint16_t RxPortVal = 0x0000;
	uint8_t RxDataTemp = 0x00;
	uint8_t RxData = 0x00;
	
	LCD_DataPinsInput();
	
	GPIO_SetBits( LCD_PORT, RW );	 // set bit RW for reading
	for(int i=0; i<2; i++){
		GPIO_SetBits( LCD_PORT, E );  // set bit E
		//-------- delay abount 800 ns --------
		delay_400ns(2); //28
		//----------------------------------
		RxPortVal = GPIO_ReadInputData( LCD_PORT ); // read data
		RxDataTemp = (uint8_t)( ( RxPortVal >> 8 ) & 0x000F );
		RxData = ( RxData | RotateBits_4( RxDataTemp ) );
		//----- shift received half word into MSb position -----	
		RxData = ( RxData << ( 4*(1-i) ) );
		GPIO_ResetBits( LCD_PORT, E );
		delay_400ns(1);
	}
	
	GPIO_ResetBits( LCD_PORT, RW );	 // set bit RW for reading
	
	LCD_PortInit();

	GPIO_Write( LCD_PORT, 0x0000 );	 // clear bits A0, E and RW
		
	return RxData;
}


void BF_Wait( void ){
	uint8_t bf_addr = 0xFF;
	while( ( bf_addr & BF_MASK ) != 0x00 ){ // check BF flag
		bf_addr = Read_BF_Addr();
	}
}

void ClearDisplay( void ){
	LCD_Write4b( (uint8_t)OP_COM, 0x01, 2 );
	BF_Wait();	
}


void ReturnHome( void ){
	LCD_Write4b( (uint8_t)OP_COM, 0x02, 2 );
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
	LCD_Write4b( (uint8_t)OP_COM, ( ID_SH | 0x40 ), 2 );
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
	LCD_Write4b( (uint8_t)OP_COM, (DCB | 0x08), 2 );
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
	LCD_Write4b( (uint8_t)OP_COM, (SC_RL | 0x10), 2 );
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
	LCD_Write4b( (uint8_t)OP_COM, ( DL_P | 0x28 ), 2 );
	BF_Wait();	
}

// used ONLY FOR LCD Initialization
void FunctionSet_HALF( uint8_t DL_P ){
	LCD_Write4b( (uint8_t)OP_COM, ( DL_P | 0x28 ), 1 );
}


/*
ADDR(6:0) - set DDRAM Address
ADDR(7) = 1 always	
*/
void Set_DDRAM_Addr( uint8_t ADDR ){
	LCD_Write4b( (uint8_t)OP_COM, ( ADDR | 0x80 ), 2 );
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
	LCD_Write4b( (uint8_t)OP_DATA, data, 2 );
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

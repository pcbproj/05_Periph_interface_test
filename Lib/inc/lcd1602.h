/*
 * LCD module ver 1.1
 * for STM32F407 work with
 * LCD1602 LCD-indicator
 * author: Bortnikov A.Y.
 * 
 * 4-bit LCD bus data 
 * 
 * */


#ifndef LCD1602_H
#define LCD1602_H

#include "stm32f4xx.h"
#include "delay.h"

/*--------- LCD1602 PINOUTS --------*/
// WARNING!!! При смене распиновки требуется поправить функцию LCD_GPIOInit(void)

#define LCD_RS_PORT         GPIOE	// RS  = A0 для некоторых LCD-индикаторов
#define LCD_RW_PORT			GPIOE
#define LCD_E_PORT          GPIOE 

#define LCD_DB7_PORT		GPIOE
#define LCD_DB6_PORT		GPIOE
#define LCD_DB5_PORT		GPIOE
#define LCD_DB4_PORT		GPIOE
				           
#define RS_PIN_NUM 			9  // data = 1 or command = 0 selection. RS  = A0 для некоторых LCD-индикаторов
#define RW_PIN_NUM 			0  // read = 1 or write = 0 selection
#define E_PIN_NUM  			1  // enable operation = 1 or disable operation = 0

#define DB7_PIN_NUM			7
#define DB6_PIN_NUM			6
#define DB5_PIN_NUM			5
#define DB4_PIN_NUM			4


#define RS_LOW()	(LCD_RS_PORT->BSRR |= 1 << (RS_PIN_NUM + 16))
#define RS_HIGH()	(LCD_RS_PORT->BSRR |= 1 << RS_PIN_NUM)

#define E_LOW()		(LCD_E_PORT->BSRR |= 1 << (E_PIN_NUM + 16))
#define E_HIGH()	(LCD_E_PORT->BSRR |= 1 << E_PIN_NUM)

#define RW_LOW()	(LCD_RW_PORT->BSRR |= 1 << (RW_PIN_NUM + 16))
#define RW_HIGH()	(LCD_RW_PORT->BSRR |= 1 << RW_PIN_NUM)

#define DB7_HIGH()	(LCD_DB7_PORT->BSRR |= 1 << (DB7_PIN_NUM + 16))
#define DB7_LOW()	(LCD_DB7_PORT->BSRR |= 1 << DB7_PIN_NUM)

#define DB6_HIGH()	(LCD_DB6_PORT->BSRR |= 1 << (DB6_PIN_NUM + 16))
#define DB6_LOW()	(LCD_DB6_PORT->BSRR |= 1 << DB6_PIN_NUM)


#define DB5_HIGH()	(LCD_DB5_PORT->BSRR |= 1 << (DB5_PIN_NUM + 16))
#define DB5_LOW()	(LCD_DB5_PORT->BSRR |= 1 << DB5_PIN_NUM)

#define DB4_HIGH()	(LCD_DB4_PORT->BSRR |= 1 << (DB4_PIN_NUM + 16))
#define DB4_LOW()	(LCD_DB4_PORT->BSRR |= 1 << DB4_PIN_NUM)



#define LCD_DATAPINS_MASK  0x0F00

#define LCD_CHAR_NUM       16
#define LCD_LINE_NUM       2

//------ data_com constant value ----
#define OP_COM             0
#define OP_DATA            1


//-------- LCD bits mask ----------------
#define BF_MASK            0x80
#define BF_BIT             6

//----Entry Mode Set MASK ---------
#define EM_DISPSHIFT_ON    0x01
#define EM_DISPSHIFT_OFF   0x00
#define EM_CURSHIFTRIGHT   0x02
#define EM_CURSHIFLEFT     0x00
//----- Display ON OFF control MASK ------
#define DISPLAY_ON         0x04
#define DISPLAY_OFF        0x00
#define CURSOR_TYPE_0      0x00 // no cursor 
#define CURSOR_TYPE_1      0x01 // symbol bliking
#define CURSOR_TYPE_2      0x02 // underline cursor not blinking
#define CURSOR_TYPE_3      0x03 // underline cursor blinking
//---- Cursor Display Shift MASK ----------
#define CURSOR_SHIFT       0x00
#define DISPLAY_SHIFT      0x08
#define SHIFT_LEFT         0x00
#define SHIFT_RIGHT        0x04
//------- Function Set MASK --------------
#define BUS_8_BITS         0x10
#define BUS_4_BITS         0x00
#define CODEPAGE_0         0x00
#define CODEPAGE_1         0x02

  
/**************
 * RUSSIAN symbols high register codes
 * get from char = 'Ю' for example
 * must be added 48(dec) = 0x30 for correct 
 * indication in LCD
 * 
 * 'А' = 0x90 but must be 0xC0 for LCD CODE_PAGE 1 
 * 'Я' = 0xAF but must be 0xDF for LCD CODE_PAGE 1
 * 
 * ************/

#define ASCII_RUS_A      0x90
#define ASCII_RUS_YA     0xAF
#define RUS_LCD_ADD      0x30

/*--- Special Symbols Codes ( CODE_PAGE = 1 ) -------*/
#define SYMB_OK          0x17
#define SYMB_CANCEL      0x78
#define SYMB_UPARROW     0x86    
#define SYMB_DWNARROW    0x87
#define SYMB_NDONE       0x2D       
#define SYMB_DONE        0x3E
#define SYMB_POINT       0x2E
#define SYMB_DBLPOINT    0x3A
#define SYMB_SLASH       0x2F
#define SYMB_VOSKL       0x21


#define SYMB_ZERO        0x30
#define SYMB_NINE        0x39
#define SYMB_RUS_A       0xC0
#define SYMB_RUS_YA      0xDF
#define SYMB_SPACE       0x20

/*
void RS_HIGH(void);
void RS_LOW(void);

void E_HIGH(void);
void E_LOW(void);

void RW_HIGH(void);
void RW_LOW(void);

void DB7_HIGH(void);
void DB7_LOW(void);

void DB6_HIGH(void);
void DB6_LOW(void);

void DB5_HIGH(void);
void DB5_LOW(void);

void DB4_HIGH(void);
void DB4_LOW(void);
*/

/*------- LOW LEVEL FUNCTIONS FOR LCD --------------*/
void LCD_GPIOInit(void);

/*
data_com - select write data or write command
	data_com = 1 - write data
	data_com = 0 - write command
*/
void LCD_Write4b( uint8_t data_com, uint8_t symbol, int cycles );


/*---------- LCD COMMANDS -----------------*/
void ClearDisplay( void );


void ReturnHome( void );


/* EntryModeSet(); 
	ID_SH - data value
ID_SH(1) select cursor movement direction:
	ID_SH(1) = 1
	ID_SH(1) = 0
ID_SH(0) = 1 enables cursor movement when DDRAM is writed	
ID_SH(2) always must be = 1
other bits of ID_SH must be zeroes
*/
void EntryModeSet( uint8_t ID_SH ); 


/*Display_ON_OFF();
DCB(1:0) - cursor type selection:
	DCB(1:0) = 00 - no cursor at all
	DCB(1:0) = 01 - symbol blinking cursor
	DCB(1:0) = 10 - underline cursor, not blinking
	DCB(1:0) = 11 - underline cursor  and it blinking 
DCB(2) = 1 - display ON
DCB(2) = 0 - display OFF 	
*/
void Display_ON_OFF( uint8_t DCB );



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
void CursorDisplayShift( uint8_t SC_RL );


/*
DL_P(0) = 0 - constant
DL_P(1) select code page
	DL_P(1) = 1 - page 1
	DL_P(1) = 0 - page 0
DL_P(3:2) = 10 - always
DL_P(4) - select datawidth of interface
	DL_P(4) = 1 - 8 bit data width interface
	DL_P(4) = 0 - 4 bit data width interface
*/
void FunctionSet( uint8_t DL_P );
void FunctionSet_HALF( uint8_t DL_P );

/*
ADDR(6:0) - set DDRAM Address
ADDR(7) = 1 always	
*/
void Set_DDRAM_Addr( uint8_t ADDR );


/*
return(7) - BusyFlag value
return(6:0) - AddressCounter value	
*/
uint8_t Read_BF_Addr( void );

void BF_Wait( void );

//---- 4bit interface display init algorithm ------
void DisplayInit_4b( void );


void LCD_WriteChar( uint8_t data );


uint8_t CharToLCD( unsigned char charact );


void ConvertCharToLCD_Code( unsigned char *in_str, uint8_t *out_array, uint16_t len );


void ShowScreen( uint8_t show_mass[LCD_LINE_NUM][LCD_CHAR_NUM] );

/*-------- Length = LCD_CHAR_NUM !!! -----------*/
void ShowArray( uint8_t *in_array );

unsigned char DigitToLCD( uint8_t digit );

void ValueToLCDArray( uint8_t value, uint8_t *lcd_array, uint8_t len );

void IntToByteArray( uint32_t in_int, uint8_t *bytes_array );


#endif



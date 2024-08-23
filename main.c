/*
Тест периферийных устройств для платы и МК:

GPIO: кнопки и светодиоды


USART6 - для RS485 подключения
	baudrate = 115200, 
	data bits = 8 
	stop bit = 1
	parity = NO


CAN2:
	bit rate = 500 kBit/s;
	TX_FRAME_ID = 0x567;
	data_len = 1;

DS18B20 - термодатчик


LCD16x02 - текстовый знакосинтезирующий экран с английским и китайским алфавитами

Потенциометр arduino

	
Алгоритм проверки следующий:
	0. Подключение:
		- подключить преобразователь RS485 -> TTL на разъем P5
			- С6 -> RS485_RXD
			- C7 -> RS485_TXD
			- GND -> RS485-GND
			- 3.3V -> RS485_VCC
		- подключить переходник CAN-USB
		- подключить термодатчик DS18B20 к разъему P7
		- подключить потенциометр к разъему Р4
			-- питание на +3.3V (вывод 1)
			-- сигнал на РА5 (вывод 3)
			-- GND на GND (вывод 2)
		- подключить LCD16x02 к разъемам (нужно схему подключения продумать)
		#####################
			- TODO: Продумать подключение LCD1602 к плате MCU! Изучить распиновку разъема LCD1602.
		#####################
		
		- Подключить питание через micro-USB разъем


	1. На светодиоды выводятся нажатые кнопки.
	
	2. В CAN2 отвечаем тем же пакетом данных на принятый фрейм:
		RX_FRAME_ID = 0x567 - принимает фремы только с таким ID
		TX_FRAME_ID = 0x567 - отвечает фреймами только с таким ID
		Для проверки CAN требуется отправить DATA_FRAME с ID = 0x567 на плату на CAN2 через приложение CANgaroo.
		И в приложении CANgaroo наблюдать ответный фрейм от платы.
	
	3. В USART6 MCU отвечает принятым байтом от ПК. Ответ приделает в терминал.
		Если в ПК придлетел такой же байт в ответ, то USART6 ОК. Следовательно RS485-TTL ок.
		
	4. На LCD показываем температуру с датчика DS18B20 и напряжение с ADC (можно отсчеты просто)

*/


#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "main.h"


#define COUNTER_1000_MS		1000	// 1000 ms for 1 second 

#define I2C_ARRAYS_LEN		8


char i2c_tx_array[I2C_ARRAYS_LEN] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };	// Массив записываетмый в EEPROM
char i2c_rx_array[I2C_ARRAYS_LEN] = {};	// Массив, куда будут читаться данные из EEPROM


uint16_t btn_count = 0;	// счетчик мс для опроса кнопок
uint16_t sec_count = 0;	// счетчик секунды для оотправки лога и сообщений
uint16_t delay1_cnt = 0; // счетчик мс для задержек
uint16_t delay2_cnt = 0; // счетчик мс для задержек


void RCC_Init(void);



void Delay_ms(uint16_t ms){
	delay1_cnt = 0;
	while(delay1_cnt < ms){}; 
}





void SysTick_Handler(void){		// прервание от Systick таймера, выполняющееся с периодом 1000 мкс
	btn_count++;
	sec_count++;
	delay1_cnt++;
	delay2_cnt++;
}


int main(void) {


	char btn_state_byte = 0;   
	/* в байте состояний кнопок btn_state_byte выставляются в 1 и сбрасываются в 0 соответствующие биты, при нажатии кнопок
	бит 0 - кнопка S1. Если кнопка нажата, то бит равен 1. Если кнопка отпущена, то бит равен 0.
	бит 1 - кнопка S2. Если кнопка нажата, то бит равен 1. Если кнопка отпущена, то бит равен 0.
	бит 2 - кнопка S3. Если кнопка нажата, то бит равен 1. Если кнопка отпущена, то бит равен 0.
	*/

	char I2C_ErrorCode = 0;
	char usart6_ErrorCode = USART6_OK;
	char test_start = 0;
  	
	uint16_t CAN2_RxFrameID, CAN2_RxDataLen;
	uint16_t CAN2_TxFrameID = TX_FRAME_ID, CAN2_TxDataLen;
	
	char CAN2_RxState = 0;
	char CAN2_TxState = 0;
	char CAN2_RxFlag = 0;
	char usart6_test_byte = 0;

	char CAN2_TxData[CAN_TX_DATA_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	char CAN2_RxData[CAN_RX_DATA_LEN] = {};

  	
  	
  	RCC_Init();
  	
  	GPIO_Init();
  	
	USART6_Init();

	CAN2_Init();
  	
  	SysTick_Config(84000);		// настройка SysTick таймера на время отрабатывания = 1 мс
								// 84000 = (AHB_freq / время_отрабатывания_таймера_в_мкс)
								// 84000 = 84_000_000 Гц / 1000 мкс; 
   
	//---- turn off leds ---------- 
	GPIOE -> BSRR |= GPIO_BSRR_BS13;
	GPIOE -> BSRR |= GPIO_BSRR_BS14;
	GPIOE -> BSRR |= GPIO_BSRR_BS15;

  
	while (1){
		
		//============= Buttons testing ================
		BTN_Check(&btn_count, &btn_state_byte);		// проверка нажатия кнопок
		
		GPIOE->ODR = (~(btn_state_byte << 13));		// выдаем состояние кнопок на LED1-LED3
		
	

		//========= CAN2 testing ========================
		CAN2_RxState = CAN2_ReceiveMSG(&CAN2_RxFrameID,			// идентификатор фрейма CAN
						&CAN2_RxDataLen,			// длина поля данных в байтах 0 - 8 байт
						CAN2_RxData						// массив байтов, принятый по CAN
					);
		
		Delay_ms(1);

		if(CAN2_RxState == CAN2_OK){		// CAN2 frame received
				CAN2_TxState = CAN2_SendMSG(CAN2_RxFrameID,			// идентификатор фрейма CAN 
											CAN2_RxDataLen,		// длина поля данных в байтах 0 - 8 байт
											CAN2_RxData				// массив байтов, для  отправки по CAN
											);
				if(!CAN2_TxState) CAN2_RxFlag = 1;	// flag for succsessfull CAN2 reception and answer

		}

		//========= USART6 testing: loop received data byte =============
		
		
		if(USART6->SR & USART_SR_RXNE) {
			usart6_test_byte = USART6->DR;

			usart6_send(&usart6_test_byte, 1);
			Delay_ms(1);

		} 


		//========== ADC measure =============


		//==== DS18B20 temper measure ============





	}	// while(1)
	  
}	// main()






/*************************** End of file ****************************/

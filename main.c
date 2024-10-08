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



uint16_t btn_count = 0;	// счетчик мс для опроса кнопок
uint16_t sec_count = 0;	// счетчик секунды для отправки лога и сообщений


void RCC_Init(void);




void SysTick_Handler(void){		// прервание от Systick таймера, выполняющееся с периодом 1 мкс
	uint16_t static us_counter = 0;
	uint16_t static ms_counter = 0;
	uint32_t static sec_counter = 0;
	if( us_counter < 1000 ){	// us timer
		us_counter++;
		delay_us++;
	}
	else{					// ms timer
		us_counter = 0;
		
		btn_count++;
		
		if(ms_counter < 1000){ 
			ms_counter++;	
			delay_ms++;
		}
		else{				// sec timer
			ms_counter = 0;

			sec_count++;
			
			if(sec_counter < 1000) 
				sec_counter++;	
			else 
				sec_counter = 0;
		}
	} 
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

  	uint16_t ADC1_sample = 0;
	uint8_t scratch_array[SCRATCH_BYTE_LEN] = {};
	uint16_t rx_temper;
	float temper_to_show;
  	

  	RCC_Init();
  	
  	GPIO_Init();
	GPIO_1WireInit();
  	
	USART6_Init();

	CAN2_Init();

	ADC1_Init();


  	
  	SysTick_Config(84);		// настройка SysTick таймера на время отрабатывания = 1 мкс
   
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
										CAN2_RxDataLen,			// длина поля данных в байтах 0 - 8 байт
										CAN2_RxData				// массив байтов, для  отправки по CAN
										);

		}

		//==== USART6 testing: return received data byte from USART6_RX  into USART6_TX
		if(USART6->SR & USART_SR_RXNE) {
			usart6_test_byte = USART6->DR;
			Delay_ms(1);
			usart6_send(&usart6_test_byte, 1);
		} 


		//========== ADC measure =============
		ADC1_StartConversion();
		ADC1_sample = ADC1_Read();
		

		//==== DS18B20 temper measure ============
		Convert_Temperature();
		ReadScratchpad(scratch_array);
		rx_temper = Scratch_To_Temperature(scratch_array);
		temper_to_show = Temperature_CalcFloat(rx_temper);


		//======= LCD1602 show data ===========
	

		
		
		Delay_ms(1000);	// wait for 1 sec for next iteration 

	}	// while(1)
	  
}	// main()






/*************************** End of file ****************************/

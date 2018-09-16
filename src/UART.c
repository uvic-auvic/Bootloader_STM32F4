/*
 * UART.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "LED.h"
#include "2DArray_Buffer.h"
#include "Command_Handler.h"

/***************************************************************
 * USER CONFIGURABLE DEFINES
 ***************************************************************/
#define UART_TO_USE			(USART1) //Example: USART1 or USART2. Has to match define from ST Library
#define GPIO_PORT_TO_USE 	(GPIOB) //Example: GPIOA or GPIOB. Has to match define from ST Library
#define TX_PIN_TO_USE		(6) // 0 to 15. Example 1 or 2.
#define RX_PIN_TO_USE		(7) // 0 to 15. Example 1 or 2.
#define BAUD_RATE	(9600)
/* END USER CONFIGURABLE DEFINES */

//Register bit for enabling TXEIE bit. This is used instead of the definitions in stm32f4xx_usart.h
#define USART_TXEIE		0b10000000
#define USART_RXNEIE	0b100000

#define UART 				(UART_TO_USE)
#define UART_GPIO_PORT		(GPIOB)
#define UART_TX_PIN			(0x1 << TX_PIN_TO_USE)
#define UART_RX_PIN			(0x1 << RX_PIN_TO_USE)
#define UART_TX_PINSOUCE	(TX_PIN_TO_USE)
#define UART_RX_PINSOUCE	(RX_PIN_TO_USE)

// Receive buffer for UART, no DMA
char inputString[BUFFER_DATA_LENGTH]; //string to store individual bytes as they are sent
uint8_t inputStringIndex = 0;
_2DArray_Buffer_t inputBuffer = {};

/***************************************************************
 * INITIALIZATION FUNCTIONS
 ***************************************************************/

static void init_UART_GPIO() {

	if (UART_GPIO_PORT == GPIOA) {
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	} else if (UART_GPIO_PORT == GPIOB) {
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	}

	GPIO_InitTypeDef GPIO_init;

	GPIO_init.GPIO_Pin = UART_TX_PIN | UART_RX_PIN;
	GPIO_init.GPIO_Mode = GPIO_Mode_AF;
	GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_init.GPIO_OType = GPIO_OType_OD;
	GPIO_init.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(UART_GPIO_PORT, &GPIO_init);

	if(UART == USART1) {
		GPIO_PinAFConfig(UART_GPIO_PORT, UART_TX_PINSOUCE, GPIO_AF_USART1);
		GPIO_PinAFConfig(UART_GPIO_PORT, UART_RX_PINSOUCE, GPIO_AF_USART1);
	}

}

static void init_UART_periph() {
	uint32_t interrupt_type = 0;
	if (UART == USART1) {

		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		interrupt_type = USART1_IRQn;
	} else if (UART == USART6) {

		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
		interrupt_type = USART6_IRQn;
	} else if (UART == USART2) {

		RCC->APB2ENR |= RCC_APB1ENR_USART2EN;
		interrupt_type = USART2_IRQn;
	}

	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initialization

	USART_InitStruct.USART_BaudRate = BAUD_RATE;	// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;	// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(UART, &USART_InitStruct);

	UART->CR1 |= USART_RXNEIE; //Enable the USART1 receive interrupt

//	NVIC_SetPriority(USART1_IRQn, 7);
//	NVIC_EnableIRQ(USART1_IRQn);

	NVIC_InitTypeDef NVIC_InitStruct;

//	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
//	NVIC_Init(&NVIC_InitStruct);

	// finally this enables the complete USART1 peripheral
	USART_Cmd(UART, ENABLE);
}


static void init_UART_DMA() {

}

extern void init_UART() {

	init_UART_GPIO();
	init_UART_periph();
	init_UART_DMA();


}

extern void deinit_UART() {

}

/***************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************/
extern int UART_push_out_len(char * message , uint8_t length) {

}

extern int UART_push_out(char * message) {

	UART_push_out_len(message, strlen(message));
}

extern int UART_push_out_len_debug(char * message, uint8_t length) {
	for(uint8_t i = 0; i < length; i++) {
		while(!(UART->SR & USART_SR_TXE));
		UART->DR = message[i];

	}
}

extern int UART_push_out_debug(char * message) {
	UART_push_out_len_debug(message, strlen(message));
}

/***************************************************************
 * INTERRUPT HANDLERS
 ***************************************************************/

static inline void UART_IRQHandler() {
	if((UART->SR & USART_FLAG_RXNE) == USART_FLAG_RXNE) { //If character is received

		char tempInput[1];
		tempInput[0] = UART->DR;

		//Check for new line character which indicates end of command
		if (tempInput[0] == '\n' || tempInput[0] == '\r') {

			if(strlen(inputString) > 0) {
				Buffer_add(&inputBuffer, inputString, BUFFER_DATA_LENGTH);
				memset(inputString, 0, BUFFER_DATA_LENGTH);
				inputStringIndex = 0;

				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				vTaskNotifyGiveFromISR(Command_Handler_Task_Handle, &xHigherPriorityTaskWoken);
			}

		} else {
			inputString[inputStringIndex] = tempInput[0];
			inputStringIndex = (inputStringIndex + 1) % BUFFER_DATA_LENGTH;
		}

	} else if ((UART->SR & USART_FLAG_TXE) == USART_FLAG_TXE) { // If Transmission is complete


	}
}

void USART1_IRQHandler() {
	UART_IRQHandler();
}

void USART2_IRQHandler() {
	UART_IRQHandler();
}

void USART6_IRQHandler() {
	UART_IRQHandler();
}

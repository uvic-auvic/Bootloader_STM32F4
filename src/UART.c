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

//Register bit for enabling TXEIE bit. This is used instead of the definitions in stm32f4xx_usart.h
#define USART_TXEIE	0b10000000
#define USART_RXEIE	0b100000

#define UART_TO_USE	(USART1)
#define UART (UART_TO_USE)
#define UART_GPIO_PORT	(GPIOB)
#define UART_TX_PIN		(GPIO_Pin_6)
#define UART_RX_PIN		(GPIO_Pin_7)

#define BAUD_RATE	(9600)

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
	GPIO_init.GPIO_OType = GPIO_OType_PP;
	GPIO_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(UART_GPIO_PORT, &GPIO_init);

	if(UART == USART1) {
		GPIO_PinAFConfig(UART_GPIO_PORT, UART_TX_PIN, GPIO_AF_USART1);
		GPIO_PinAFConfig(UART_GPIO_PORT, UART_RX_PIN, GPIO_AF_USART1);
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

	UART->CR1 |= USART_RXEIE; //Enable the USART1 receive interrupt

	NVIC_SetPriority(interrupt_type, 7);
	NVIC_EnableIRQ(interrupt_type);

	// finally this enables the complete USART1 peripheral
	USART_Cmd(UART, ENABLE);

}

extern void init_UART(TaskHandle_t xHandle) {

	init_UART_GPIO();
	init_UART_periph();
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
		while(!(UART-> SR & USART_SR_TXE)) {
			UART->DR = message[i];
		}
	}
}

extern int UART_push_out_debug(char * message) {
	UART_push_out_len_debug(message, strlen(message));
}



/***************************************************************
 * INTERRUPT HANDLERS
 ***************************************************************/

static inline void UART_IRQHandler() {
	GPIOD->ODR ^= GPIO_Pin_14;

	uint32_t temp = UART->DR;
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

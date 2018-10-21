/*
 * UART.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */
#include <string.h>

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "User_Defines.h"
#include "UART.h"
#include "LED.h"
#include "Command_Handler.h"

//Register bit for enabling TXEIE bit. This is used instead of the definitions in stm32f4xx_usart.h
#define USART_TXEIE		0b10000000
#define USART_RXNEIE	0b100000

#define UART 				(UART_TO_USE)
#define UART_GPIO_PORT		(GPIOB)
#define UART_TX_PIN			(0x1 << TX_PIN_TO_USE)
#define UART_RX_PIN			(0x1 << RX_PIN_TO_USE)
#define UART_TX_PINSOUCE	(TX_PIN_TO_USE)
#define UART_RX_PINSOUCE	(RX_PIN_TO_USE)

// Receive buffer for UART with DMA
static commBuffer_t receiveBuffer;
commBuffer_t * receiveBuffer_ptr = &receiveBuffer;

// Transmit buffer for UART, no DMA
char outputBuffer[OUTPUT_BUFFER_SIZE_BYTES];
uint8_t outputBufferIndexHead = 0, outputBufferIndexTail = 0;

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

	uint32_t interrupt_number = 0;
	if (UART == USART1) {

		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		interrupt_number = USART1_IRQn;
	} else if (UART == USART6) {

		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
		interrupt_number = USART6_IRQn;
	} else if (UART == USART2) {

		RCC->APB2ENR |= RCC_APB1ENR_USART2EN;
		interrupt_number = USART2_IRQn;
	}

	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initialization

	USART_InitStruct.USART_BaudRate = BAUD_RATE;	// the baudrate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1;	// we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No;// we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(UART, &USART_InitStruct);

	NVIC_SetPriority(interrupt_number, 7);
	NVIC_EnableIRQ(interrupt_number);

	// finally this enables the complete USART1 peripheral
	USART_Cmd(UART, ENABLE);
}

static void init_UART_DMA() {

	//USART1_RX is mapped on DM2 Stream2 Channel4
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	DMA_STREAM->CR = 0;

	while((DMA_STREAM->CR & 0x1) == 1);

	DMA_STREAM->PAR = (uint32_t) &(UART->DR);
	DMA_STREAM->M0AR = (uint32_t) &(receiveBuffer_ptr->payloadSize);
	DMA_STREAM->NDTR = PAYLOAD_INFO_SIZE_BYTES;
	DMA_STREAM->CR |= DMA_Channel_4;
	DMA_STREAM->CR |= DMA_DIR_PeripheralToMemory;
	DMA_STREAM->CR |= DMA_SxCR_MINC; // Enable Memory
	DMA_STREAM->CR |= DMA_SxCR_TCIE; // Enable transfer complete interrupt
	DMA->LIFCR |= 0xFFFFFFFF;
	DMA->HIFCR |= 0xFFFFFFFF;

	UART->CR3 |= USART_CR3_DMAR; // Enable DMA in UART Peripheral

	NVIC_SetPriority(DMA2_Stream2_IRQn, 7);
	NVIC_EnableIRQ(DMA2_Stream2_IRQn);

	DMA_STREAM->CR |= 0x1;

}

static void init_recieve_buffer() {

	receiveBuffer_ptr->isRecivingHeader = 1;
	receiveBuffer_ptr->isRecivingPayload = 0;
	receiveBuffer_ptr->payloadSize = 0;
	receiveBuffer_ptr->previousPacketID = 0;

}

extern void init_UART() {

	init_UART_GPIO();
	init_UART_periph();
	init_UART_DMA();
	init_recieve_buffer();

}

/***************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************/


/*
 * ERROR CODE:
 * -1 = String length is not 1 or greater
 * -2 = OutputBuffer will overflow. Wait some time and retry
 * 1  = Added to buffer successfully
 */
extern int UART_push_out_len(char* mesg, int len) {

	if(len < 1) {
		return -1;
	}

	int diff = outputBufferIndexTail - outputBufferIndexHead;

	if(diff <= 0) {
		diff += OUTPUT_BUFFER_SIZE_BYTES;
	}
	if(len > (diff - 1)) { //Has to be diff - 1. Cannot write to position pointed to by outputBufferIndexTail.
		return -2;
	}

	for (int i = 0; i < len; i++) {
		outputBuffer[outputBufferIndexHead] = mesg[i];
		outputBufferIndexHead = (outputBufferIndexHead + 1) & 63;
	}

	USART1->CR1 |= USART_TXEIE;
	return 1;

}

/*
 * ERROR CODE:
 * -1 = String length is not 1 or greater
 * -2 = OutputBuffer will overflow. Wait some time and retry
 * 1  = Added to buffer successfully
 */
extern int UART_push_out(char* mesg) {

	 return UART_push_out_len(mesg, strlen(mesg));
}

extern void UART_push_out_len_debug(char * message, uint8_t length) {
	for(uint8_t i = 0; i < length; i++) {
		while(!(UART->SR & USART_SR_TXE));
		UART->DR = message[i];

	}
}

extern void UART_push_out_debug(char * message) {
	UART_push_out_len_debug(message, strlen(message));
}

/***************************************************************
 * INTERRUPT HANDLERS
 ***************************************************************/

static inline void UART_IRQHandler() {

	if ((UART->SR & USART_FLAG_TXE) == USART_FLAG_TXE) { // If Transmission is complete

		if ((outputBufferIndexHead - outputBufferIndexTail) != 0) {
			UART->DR = outputBuffer[outputBufferIndexTail];
			outputBufferIndexTail = (outputBufferIndexTail + 1) & 63;
		} else {
			UART->CR1 &= ~USART_TXEIE;
		}

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

static inline void DMA_TC_Handler() { // DMA Transfer Complete Handler

	if (receiveBuffer_ptr->isRecivingHeader == 1) {

		DMA_STREAM->M0AR = (uint32_t) receiveBuffer_ptr->payload;
		DMA_STREAM->NDTR = receiveBuffer_ptr->payloadSize;
		DMA_STREAM->CR |= 0x1;

		receiveBuffer_ptr->isRecivingHeader = 0;
		receiveBuffer_ptr->isRecivingPayload = 1;

		UART_push_out("ACK\r\n");

	} else if (receiveBuffer_ptr->isRecivingPayload == 1) {

		DMA_STREAM->M0AR = (uint32_t) &(receiveBuffer_ptr->payloadSize);
		DMA_STREAM->NDTR = PAYLOAD_INFO_SIZE_BYTES;

		receiveBuffer_ptr->isRecivingHeader = 1;
		receiveBuffer_ptr->isRecivingPayload = 0;

		vTaskNotifyGiveFromISR(Command_Handler_Task_Handle, pdFALSE);
	}

}

void DMA2_Stream2_IRQHandler() {

	if((DMA->LISR & DMA_LISR_TCIF2 ) == DMA_LISR_TCIF2 ) {

		DMA->LIFCR |= DMA_LIFCR_CTCIF2;

		DMA_TC_Handler();
	}

}

/*
 * UART.h
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#ifndef UART_H_
#define UART_H_

#include "User_Defines.h"

#define INPUT_BUFFER_SIZE_BYTES		32
#define OUTPUT_BUFFER_SIZE_BYTES	64

typedef struct commBuffer {
	uint8_t isRecivingHeader;
	uint8_t isRecivingPayload;
	uint8_t previousPacketID;
	uint16_t payloadSize;
	uint8_t payload[MAX_PAYLOAD_SIZE_BYTES];
}commBuffer_t;

extern commBuffer_t * receiveBuffer_ptr;

extern void init_UART();
extern int UART_push_out(char * mesg);
extern int UART_push_out_len(char* mesg, int len);

#ifdef DEBUG

extern void UART_push_out_len_debug(char * message, uint8_t length);
extern void UART_push_out_debug(char * message);

#endif

#endif /* UART_H_ */

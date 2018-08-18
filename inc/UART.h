/*
 * UART.h
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#ifndef UART_H_
#define UART_H_

extern void init_UART(TaskHandle_t xHandle);
extern void deinit_UART();
extern int UART_push_out(char * message);
extern int UART_push_out_len(char * message , uint8_t length);

#ifdef DEBUG

extern int UART_push_out_len_debug(char * message, uint8_t length);
extern int UART_push_out_debug(char * message);

#endif

#endif /* UART_H_ */

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

#endif /* UART_H_ */

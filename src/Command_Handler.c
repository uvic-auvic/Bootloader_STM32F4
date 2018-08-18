/*
 * Command_Handler.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "UART.h"

static void Command_Handler() {

	while(1) {

		UART_push_out_debug("h");
		vTaskDelay(500);
	}

}

extern void init_Command_Handler() {

	TaskHandle_t xHandle = NULL;

	// Create the FSM task
    xTaskCreate(Command_Handler,
		(const char *)"Command_Handler",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		&xHandle              ); // pvCreatedTask */

    init_UART(xHandle);
}

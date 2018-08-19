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
#include "LED.h"
#include "Bootloader.h"
#include "2DArray_Buffer.h"
#include "User_Defines.h"

TaskHandle_t Command_Handler_Task_Handle = NULL;

/* COMMAND DEFINES */

// RID Command
#define CMD_RID_STR		"RID"
#define CMD_RID_LENGTH	3

// START command
#define CMD_START_STR		"START"
#define CMD_START_LENGTH	5

extern void Command_Handler() {

	char commandString[BUFFER_DATA_LENGTH] = {};

	while(1) {

		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

		Buffer_pop(&inputBuffer, commandString);

		// RID Command
		if(strncmp(CMD_RID_STR, commandString, CMD_RID_LENGTH) == 0 && strlen(commandString) == CMD_RID_LENGTH) {
			UART_push_out(DEVICEID);
			UART_push_out("\r\n");

		}

		else if(strncmp(CMD_START_STR, commandString, CMD_START_LENGTH) == 0 && strlen(commandString) == CMD_START_LENGTH) {
			xTaskNotifyGive(Bootloader_Task_Handle);
			UART_push_out("ACK\r\n");
		}

		//No Matches
		else {

		}

		led_toggle(LED3);

		vTaskDelay(pdMS_TO_TICKS(500));

	}

}


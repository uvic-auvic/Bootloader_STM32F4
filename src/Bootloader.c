/*
 * Bootloader.c
 *
 *  Created on: Aug 18, 2018
 *      Author: Poornachander
 */

#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "User_Defines.h"
#include "Bootloader.h"
#include "LED.h"
#include "UART.h"
#include "Command_Handler.h"
#include "Flash_Interface.h"

/* DEFINES */
#define BOOTLOADER_QUEUE_LENGTH	4
#define BOOTLOADER_QUEUE_SIZE	1

/* GLOBAL VARIABLES */
QueueHandle_t Bootloader_Queue_Handle = NULL;

struct flashBuffer * flashBuffer_ptr;
struct firmwareInfo * firmwareInfo_ptr;

static void init_flash_buffer() {
	flashBuffer_ptr->length = 0;
	flashBuffer_ptr->startingAddress = APP_START_ADDRESS;
	flashBuffer_ptr->mutex = xSemaphoreCreateMutex();
}

/**
 * Checks that the app sector data is intact
 * Returns:  1 - App data intact
 * 			-1 - App data corrupt
 */
static int8_t is_app_sector_valid() {
	//Do checksum

	return 1;
}

/**
 * Programs the data currently in the flash buffer into the flash and verifies that it was programmed correctly
 * Returns:  1 - Programming success
 * 			-1 - Attempted write to restricted flash sector
 * 			-2 - Verify failed
 * 			-3 - Mutex error
 */
static int8_t program_code_in_buffer() {

	if (xSemaphoreTake(flashBuffer_ptr->mutex, FLASHBUFFER_MUTEX_TIMEOUT)) {
		//Start writing to flash
	 	for(uint16_t i = 0; i < flashBuffer_ptr->length; i += 4) {
			if(write_word(flashBuffer_ptr->startingAddress + i, flashBuffer_ptr->data[i]) == -1) {
				//ERROR: Accessing restricted flash sector
				xSemaphoreGive(flashBuffer_ptr->mutex);
				return -1;
			}
		}

		//Verify that data has been written properly
		for(uint16_t i = 0; i < flashBuffer_ptr->length; i += 4) {

			if(read_word(flashBuffer_ptr->startingAddress + i) != flashBuffer_ptr->data[i]) {
				//ERROR: Flash data does not match expected data
				xSemaphoreGive(flashBuffer_ptr->mutex);
				return -2;
			}

		}

	} else {
		//Could not get mutex
		return -3;
	}

	xSemaphoreGive(flashBuffer_ptr->mutex);
	return 1;
}

// Starts the application
static void startApplication() {

	/* Deinit all Peripherals */
	deinit_LED();
	deinit_UART();
	deinit_flash();

	/* Branch to Application */
	typedef void (*pFunction)(void);
	pFunction appEntry;
	uint32_t appStack;

	/* Get the application stack pointer (First entry in the application vector table) */
	appStack = (uint32_t) *((__IO uint32_t*)APP_START_ADDRESS);

	/* Get the application entry point (Second entry in the application vector table) */
	appEntry = (pFunction) *(__IO uint32_t*) (APP_START_ADDRESS + 4);

	__disable_irq();

	// Disable IRQs
	for(uint8_t i = 0;i < 8;i++)
	{
	    NVIC->ICER[i] = 0xFFFFFFFF;
	}
	// Clear pending IRQs
	for(uint8_t i = 0;i < 8;i++)
	{
	    NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	__enable_irq();

	SysTick->CTRL =0;
	SysTick->LOAD=0;
	SysTick->VAL=0;

	/* Reconfigure vector table offset register to match the application location */
	SCB->VTOR = APP_START_ADDRESS;

	/* Set the application stack pointer */
	__set_MSP(appStack);

	/* Start the application */
	appEntry();
}

static void Bootloader_Main_Task() {

	Bootloader_Queue_Handle = xQueueCreate(BOOTLOADER_QUEUE_LENGTH, BOOTLOADER_QUEUE_SIZE);
	char queueCommand = 0;

	{
		uint8_t queueExitStatus = xQueueReceive(Bootloader_Queue_Handle, &queueCommand, pdMS_TO_TICKS(START_COMMAND_WAIT_MS));

		if(queueExitStatus == pdFALSE && FORCE_LOAD_BOOTLOADER == 0) {

			if(is_app_sector_valid() == 1 || FORCE_LOAD_APP == 1) {

				startApplication();
			} else {
				//ERROR: App sector corrupt. Cannot boot.
			}
		}
	}

	/* BEGIN BOOTLOADING */
	{
		//Send out device ID and max data size
		while(UART_push_out("DEVICE_ID:") == -2);
		while(UART_push_out(DEVICEID) == -2);
		while(UART_push_out("\r\n") == -2);
		while(UART_push_out("MAX_PACKET_SIZE:") == -2);
		char maxPacketSizeSTR[6] = {};
		itoa(MAX_PAYLOAD_SIZE_BYTES, maxPacketSizeSTR, 10);
		while(UART_push_out(maxPacketSizeSTR) == -2);
		while(UART_push_out("\r\n") == -2);
	}

	//Erase app sector before loading program
	//erase_app_sector();

	while(1) {

		xQueueReceive(Bootloader_Queue_Handle, &queueCommand, portMAX_DELAY);

		switch (queueCommand) {
		case 'F':

			//Check firmware is compatible with this device
			if(strncmp(DEVICEID, firmwareInfo_ptr->deviceID, DEVICE_ID_MAX_SIZE_BYTES)) {
				//ERROR: DEVICE ID DOES NOT MATCH
				UART_push_out("ERR:FIRMWARE_INCOMPATIBLE\r\n");
			} else if(firmwareInfo_ptr->programSize > APP_SECTION_SIZE) {
				char appSize[6];
				itoa(APP_SECTION_SIZE, appSize, 10);
				UART_push_out("ERR:FIRMWARE_TOO_LARGE: Available app section size is ");
				UART_push_out(appSize);
				UART_push_out(" bytes.\r\n");
			} else {
				//Program firmware data into config section
			}
			break;

		case 'M':
			program_code_in_buffer();
			break;

		case 'L':
			break;

		default:
			//ERROR: No Matches
			break;
		}

	}

}

static void init_Bootloader() {

	init_LED();
	init_flash_buffer();
	init_UART();
	init_flash();

	// Create the Bootloader Main task
	xTaskCreate(Bootloader_Main_Task,
		(const char *)"Bootloader_Main_Task",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		NULL              ); // pvCreatedTask */

	// Create the Command Handler task
    xTaskCreate(Command_Handler_Task,
		(const char *)"Command_Handle_Taskr",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		&Command_Handler_Task_Handle              ); // pvCreatedTask */

    // Create the blinky LED task
    xTaskCreate(Blinky_LED_Task,
		(const char *)"Blinky_LED_Task",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		NULL              ); // pvCreatedTask */

}

extern void Bootloader_Start() {

	init_Bootloader();
	vTaskStartScheduler();

	//Should never reach this
	while(1);

}

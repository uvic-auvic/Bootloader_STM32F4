/*
 * Bootloader.c
 *
 *  Created on: Aug 18, 2018
 *      Author: Poornachander
 */

#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "LED.h"
#include "UART.h"
#include "Command_Handler.h"
#include "User_Defines.h"
#include "Flash_Interface.h"

/* DEBUG DEFINES */
#define FORCE_LOAD_APP	0
#define FORCE_LOAD_BOOTLOADER	1 //Overrides FORCE_LOAD_APP

/* BOOTLOADER DEFINES */
#define START_COMMAND_WAIT_MS	(2000)

/* GLOBAL VARIABLES */
TaskHandle_t Bootloader_Task_Handle = NULL;

static int8_t is_app_sector_valid() {
	//Do checksum

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

static void Bootloader_Main_task() {

	led_on(LED2);

	//Waint unitl command handler task notifies of incommming command
	uint32_t ticks = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(START_COMMAND_WAIT_MS));

	if(ticks == 0 && FORCE_LOAD_BOOTLOADER == 0) {

		if(is_app_sector_valid() == 1 || FORCE_LOAD_APP == 1) {

			startApplication();
		} else {
			//ERROR: App sector corrupt. Cannot boot.
		}
	}

	//Begin bootloading
	while(1) {

		led_toggle(LED2);
		//UART_push_out_len_debug("h", 1);
		vTaskDelay(pdMS_TO_TICKS(500));

	}

}

static void init_Bootloader() {

	init_LED();
	init_UART();
	init_flash();

	// Create the Bootloader Main task
	xTaskCreate(Bootloader_Main_task,
		(const char *)"Bootloader_Main_task",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		&Bootloader_Task_Handle              ); // pvCreatedTask */

	// Create the Command Handler task
    xTaskCreate(Command_Handler,
		(const char *)"Command_Handler",
		configMINIMAL_STACK_SIZE,
		NULL,                 // pvParameters
		tskIDLE_PRIORITY + 1, // uxPriority
		&Command_Handler_Task_Handle              ); // pvCreatedTask */


}

extern void Bootloader_Start() {

	init_Bootloader();

	vTaskStartScheduler();

	//Should never reach this
	while(1);

}

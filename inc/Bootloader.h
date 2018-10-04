/*
 * Bootloader.h
 *
 *  Created on: Aug 18, 2018
 *      Author: Poornachander
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "User_Defines.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

struct flashBuffer {
	SemaphoreHandle_t mutex;
	uint32_t startingAddress;
	uint16_t length; //In Bytes
	uint8_t data[MAX_DATA_SIZE_BYTES];
};

struct firmwareInfo {
	uint16_t programSize;
	char firmwareVersion[FIRMWARE_VERSION_SIZE_BYTES];
	char deviceID[DEVICE_ID_MAX_SIZE_BYTES];
};

#define FLASHBUFFER_MUTEX_TIMEOUT	(portMAX_DELAY)

extern TaskHandle_t Bootloader_Task_Handle;
extern QueueHandle_t Bootloader_Queue_Handle;
extern struct flashBuffer * flashBuffer_ptr;
extern struct firmwareInfo * firmwareInfo_ptr;

extern void Bootloader_Start();

#endif /* BOOTLOADER_H_ */

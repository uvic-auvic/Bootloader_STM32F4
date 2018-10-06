/*
 * Command_Handler.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "User_Defines.h"
#include "UART.h"
#include "LED.h"
#include "Bootloader.h"
#include "CRC.h"

TaskHandle_t Command_Handler_Task_Handle = NULL;

/* COMMAND DEFINES */
// RID Command
#define CMD_RID_STR		"RID"
#define CMD_RID_LENGTH	3

// START command
#define CMD_START_STR		"START"
#define CMD_START_LENGTH	5

// Packet Type
#define DATA_TYPE_ASCII		'A'
#define DATA_TYPE_FIRST		'F'
#define DATA_TYPE_MID		'M'
#define DATA_TYPE_LAST		'L'


/*
 * Checks that the packet ID is in order
 * Checks that the CRC is correct
 * Returns: 1 = Actual CRC matches expected CRC
 * 			0 = Actual CRC does not match expected CRC
 */
static uint8_t check_payload_CRC() {
	uint32_t crc_calculated = calculate_CRC((uint32_t *)receiveBuffer_ptr->payload, receiveBuffer_ptr->payloadSize - CRC_SIZE_BYTES);
	uint32_t crc_received = *(uint32_t *)(receiveBuffer_ptr->payload + (receiveBuffer_ptr->payloadSize - CRC_SIZE_BYTES));

	if(crc_calculated == crc_received) {
		return 1;
	} else {
		return 0;
	}
}

extern void Command_Handler_Task() {

	while(1) {

		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

		//Check packet ID
		if( *(uint16_t *)(receiveBuffer_ptr->payload + PACKET_ID_OFFSET) != (receiveBuffer_ptr->previousPacketID + 1) && !OVERRIDE_PACKET_ID_CHECK)
		{
			//ERROR: Packet not in order

		} else
		{
			receiveBuffer_ptr->previousPacketID += 1;

			//Check if command is in ASCII
			if( *(char *)(receiveBuffer_ptr->payload + DATA_TYPE_OFFSET) == DATA_TYPE_ASCII)
			{

				// RID Command
				if(strncmp(CMD_RID_STR, (char *)(receiveBuffer_ptr->payload + DATA_OFFSET), CMD_RID_LENGTH) == 0)
				{
					UART_push_out(DEVICEID);
					UART_push_out("\r\n");

				}

				// START command
				else if(strncmp(CMD_START_STR, (char *)(receiveBuffer_ptr->payload + DATA_OFFSET), CMD_START_LENGTH) == 0)
				{
					char charToSend = 'S';
					UART_push_out("ACK\r\n");
					xQueueSend(Bootloader_Queue_Handle, &charToSend, portMAX_DELAY);
					//ACK response will be sent out from Bootloader task
				}

				//No matches
				else
				{
					//No matches for ASCII commands
				}

			}

			//Non-ASCII command
			else
			{

				if(check_payload_CRC() == 0 && !OVERRIDE_CRC_CHECK) {
					//ERROR: Error in payload
				} else
				{

					// First program packet
					if(*(char *)(receiveBuffer_ptr->payload + DATA_TYPE_OFFSET) == DATA_TYPE_FIRST)
					{
						uint8_t deviceIDSize = receiveBuffer_ptr->payloadSize - PACKET_ID_SIZE_BYTES - DATA_TYPE_SIZE_BYTES
								- PROGRAM_SIZE_SIZE_BYTES - FIRMWARE_VERSION_SIZE_BYTES - CRC_SIZE_BYTES;

						firmwareInfo_ptr->programSize = *(uint16_t *)(receiveBuffer_ptr->payload + PROGRAM_SIZE_OFFSET);
						memcpy(firmwareInfo_ptr->firmwareVersion, receiveBuffer_ptr->payload + FIRMWARE_VERSION_OFFSET, FIRMWARE_VERSION_SIZE_BYTES);
						memcpy(firmwareInfo_ptr->deviceID, receiveBuffer_ptr->payload + DEVICE_ID_OFFSET, deviceIDSize);

						//Notify Bootloader Task
						xQueueSend(Bootloader_Queue_Handle, "F", portMAX_DELAY);

						//Send success through UART
						UART_push_out("ACK\r\n");

					}

					// Middle programs packets
					else if(*(char *)(receiveBuffer_ptr->payload + DATA_TYPE_OFFSET) == DATA_TYPE_MID)
					{

						if(xSemaphoreTake(flashBuffer_ptr->mutex, portMAX_DELAY))
						{

							flashBuffer_ptr->startingAddress += flashBuffer_ptr->length; //Set starting address
							flashBuffer_ptr->length = receiveBuffer_ptr->payloadSize - NOT_DATA_SIZE_BYTES; //Set length of data
							memcpy(flashBuffer_ptr->data, receiveBuffer_ptr->payload + DATA_OFFSET, flashBuffer_ptr->length); //Copy data into buffer

							xSemaphoreGive(flashBuffer_ptr->mutex);
							xQueueSend(Bootloader_Queue_Handle, "M", portMAX_DELAY); //Notify bootloader task

							//Send success through UART
							UART_push_out("ACK\r\n");

						} else
						{
							//ERROR: Mutex error
						}


					}

					//Last program packet
					else if(*(char *)(receiveBuffer_ptr->payload + DATA_TYPE_OFFSET) == DATA_TYPE_LAST)
					{
						//Send success through UART
						UART_push_out("ACK\r\n");
					}

					//No Matches
					else
					{

					}

				}

			}

		}

		//Enable DMA stream
		DMA_STREAM->CR |= 0x1;

	}

}


/*
 * User_Defines.h
 *
 *  Created on: Aug 18, 2018
 *      Author: Poornachander
 */

#ifndef USER_DEFINES_H_
#define USER_DEFINES_H_

/* MEMORY DEFINES. Should match up with application linker script */
#define BOOTLOADER_START_ADDRESS	0x08000000
#define BOOTLOADER_SECTION_SIZE		0x7F00
#define CONFIG_START_ADDRESS 		0x08007F00
#define CONFIG_SECTION_SIZE			0x100
#define APP_START_ADDRESS 			0x08008000
#define APP_SECTION_SIZE			0x78000

/* CONFIG DEFINES */
#define DEVICEID	"BOOTLOADER_TEST"

/* UART DEFINES*/

/* LED DEFINES */




/* CHECKS TO SEE IF DEFINES ARE VALID */

#if (BOOTLOADER_SECTION_SIZE % 4) != 0
#error "BOOTLOADER_SECTION_SIZE must be a multiple of 4"
#endif

#if (CONFIG_SECTION_SIZE % 4) != 0
#error "CONFIG_SECTION_SIZE must be a multiple of 4"
#endif

/* OTHER DEFINE (Should not be modified by user */
#define DEVICEID_PTR (char *){DEVICEID}

#endif /* USER_DEFINES_H_ */

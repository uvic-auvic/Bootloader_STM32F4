/*
 * CRC.c
 *
 *  Created on: Sep 18, 2018
 *      Author: Poornachander
 */

#include "stm32f4xx.h"

extern uint32_t switch_endiness_uint32(uint32_t input, uint8_t numBytes) {

	uint32_t output = 0;

	for(uint8_t i = 0; i < numBytes; i++) {
		output = output << 8;
		output |= (input & 0xFF);
		input = input >> 8;
	}

	return output;
}

extern uint32_t calculate_CRC(uint32_t * input, uint16_t length) {

	RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;

	CRC->CR |= CRC_CR_RESET;

	for (uint16_t i = 0; i < length; i += 4) {
		CRC->DR = *(input);
		input += 1;
	}

	return (CRC->DR);

	RCC->AHB1ENR &= ~(RCC_AHB1ENR_CRCEN);

}

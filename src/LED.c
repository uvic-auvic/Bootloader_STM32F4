/*
 * LED.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#include "stm32f4xx.h"

#define LED_GPIO_PORT (GPIOD)
#define LED_GPIO_PIN_1	(GPIO_Pin_15)
#define LED_GPIO_PIN_2	(GPIO_Pin_14)
#define LED_GPIO_PIN_3	(GPIO_Pin_13)
#define LED_GPIO_PIN_4	(GPIO_Pin_12)

extern void led_on() {

	LED_GPIO_PORT->ODR |= LED_GPIO_PIN_1;
}

extern void led_off() {

	LED_GPIO_PORT->ODR &= ~(LED_GPIO_PIN_1);
}

extern void led_toggle() {

	LED_GPIO_PORT->ODR ^= LED_GPIO_PIN_1;
}
extern void init_LED() {

	if (LED_GPIO_PORT == GPIOD) {
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	}

	GPIO_InitTypeDef GPIO_init;

	GPIO_init.GPIO_Pin = LED_GPIO_PIN_1 | LED_GPIO_PIN_2 | LED_GPIO_PIN_3 | LED_GPIO_PIN_4;
	GPIO_init.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_init.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_init.GPIO_OType = GPIO_OType_PP;
	GPIO_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(LED_GPIO_PORT, &GPIO_init);
}

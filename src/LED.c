/*
 * LED.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "LED.h"

#define LED_TO_BLINK	(LED2)

static uint16_t led_period = 0;

extern void led_on(uint32_t LED) {

	LED_GPIO_PORT->ODR |= LED;
}

extern void led_off(uint32_t LED) {

	LED_GPIO_PORT->ODR &= ~(LED);
}

extern void led_toggle(uint32_t LED) {

	LED_GPIO_PORT->ODR ^= LED;
}

extern void led_behaviour(uint16_t _led_period) {
	if(_led_period > 2000) {
		_led_period = 2000;
	}

	led_period = _led_period;
}

extern void Blinky_LED_Task() {

	while(1) {

		if(led_period < 0) {
			led_off(LED_TO_BLINK);
		} else if (led_period == 0) {
			led_on(LED_TO_BLINK);
		} else {
			led_toggle(LED_TO_BLINK);
		}

		vTaskDelay(pdMS_TO_TICKS(led_period));
	}
}

extern void init_LED() {

	if (LED_GPIO_PORT == GPIOD) {
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	}

	GPIO_InitTypeDef GPIO_init;

	GPIO_init.GPIO_Pin = LED1 | LED2 | LED3 | LED4;
	GPIO_init.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_init.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_init.GPIO_OType = GPIO_OType_PP;
	GPIO_init.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(LED_GPIO_PORT, &GPIO_init);
}

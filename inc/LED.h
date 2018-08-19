/*
 * LED.h
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f4xx_gpio.h"

#define LED1	(GPIO_Pin_15)
#define LED2	(GPIO_Pin_14)
#define LED3	(GPIO_Pin_13)
#define LED4	(GPIO_Pin_12)

extern void led_on();
extern void led_off();
extern void led_toggle();
extern void init_LED();
extern void deinit_LED();

#endif /* LED_H_ */

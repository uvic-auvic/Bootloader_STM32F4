/*
 * LED.h
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#ifndef LED_H_
#define LED_H_

#include "stm32f4xx_gpio.h"



extern void led_on();
extern void led_off();
extern void led_toggle();
extern void led_behaviour(uint16_t _led_period);
extern void Blinky_LED_Task();
extern void init_LED();

#endif /* LED_H_ */

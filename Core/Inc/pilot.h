/*
 * pilot.h
 *
 *  Created on: Sep 2, 2025
 *      Author: micha
 */

#ifndef INC_PILOT_H_
#define INC_PILOT_H_

#include "stm32l4xx_hal.h"   // HAL dla STM32L4
#include "gpio.h"
extern const uint16_t pilot_min;
extern const uint16_t pilot_mid;
extern const uint16_t pilot_max;

// Funkcje zwracające różnicę czasu (sygnały PWM z pilota RC)

void set_speed(uint32_t signal);
void set_speed1(uint32_t signal);
void set_teering(uint32_t signal);
void pilot_set_steer_min_scale(float scale);
float pilot_get_steer_min_scale(void);


extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
#endif /* INC_PILOT_H_ */

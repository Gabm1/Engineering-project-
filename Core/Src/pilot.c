/*
 * pilot.c
 *

 *  Created on: Sep 2, 2025
 *      Author: micha
 */
#include "pilot.h"
#include "main.h"
#include <math.h>
uint16_t const pilot_min = 1100;
uint16_t const pilot_mid = 1500;
uint16_t const pilot_max = 1900;

uint16_t const steer_min = 1000;
uint16_t const steer_mid = 1500;
uint16_t const steer_max = 2000;


// USTAWIENIA DO ZMIANY
// =============================
// np. 0.60f => przy pełnym skręcie jedna strona ma 60%, druga 100%
// zmieniasz sobie tę wartość i wszystko działa dalej
static float STEER_MIN_SCALE = 0.60f;

// deadbandy (żeby nie pływało)
static const uint16_t SPEED_DB  = 50;
static const uint16_t STEER_DB  = 50;

// steerc: skręt w zakresie [-1..+1]
// -1 = pełny skręt w lewo (prawy silnik przycięty)
// +1 = pełny skręt w prawo (lewy silnik przycięty)
static float steerc = 0.0f;

void set_speed(uint32_t signal){

	if (signal > pilot_mid + 50){
		if(signal >= pilot_max){
			HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, SET);
			HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, RESET);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 10000);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 10000);
				}
		else if(signal<pilot_max){
			float ratio = (float)(signal - pilot_mid) / (float)(pilot_max - pilot_mid);
			float set_f = ratio * 8000.0f+2000;
			uint32_t set = (uint32_t)set_f;

			HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, SET);
			HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, RESET);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, set);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, set);}
		}
	else if (signal < pilot_mid - 50){
		if(signal < pilot_min){
			HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, RESET);
			HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, SET);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 10000);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 10000);

		}
		else if(signal<pilot_mid){
			float ratio = (float)(pilot_mid - signal) / (float)(pilot_mid - pilot_min);
			float set_f = ratio * 8000.0f+2000;
			uint32_t set = (uint32_t)set_f;
			HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, RESET);
			HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, SET);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, set);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, set);

		}
			}
	else{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
	}
}

static inline float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline uint32_t clampu32(uint32_t x, uint32_t lo, uint32_t hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void set_teering(uint32_t signal)
{
    // domyślnie na wprost
    steerc = 0.0f;

    // deadband wokół środka
    if (signal > (steer_mid + STEER_DB)) {
        if (signal >= steer_max) {
            steerc = +1.0f;
        } else {
            float t = (float)(signal - steer_mid) / (float)(steer_max - steer_mid); // 0..1
            steerc = clampf(t, 0.0f, 1.0f);
        }
    }
    else if (signal < (steer_mid - STEER_DB)) {
        if (signal <= steer_min) {
            steerc = -1.0f;
        } else {
            float t = (float)(steer_mid - signal) / (float)(steer_mid - steer_min); // 0..1
            steerc = -clampf(t, 0.0f, 1.0f);
        }
    }
}

void set_speed1(uint32_t signal)
{
    // 1) Wyznacz bazowy PWM (0..10000) oraz kierunek
    uint32_t base_pwm = 0;

    if (signal > (pilot_mid + SPEED_DB)) {
        // forward
        HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, SET);
        HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, RESET);

        if (signal >= pilot_max) {
            base_pwm = 10000;
        } else {
            float ratio = (float)(signal - pilot_mid) / (float)(pilot_max - pilot_mid); // 0..1
            float set_f = ratio * 8000.0f + 2000.0f; // 2000..10000
            base_pwm = (uint32_t)set_f;
        }
    }
    else if (signal < (pilot_mid - SPEED_DB)) {
        // backward
        HAL_GPIO_WritePin(DIR1A_GPIO_Port, DIR1A_Pin, RESET);
        HAL_GPIO_WritePin(DIR1B_GPIO_Port, DIR1B_Pin, SET);

        if (signal <= pilot_min) {
            base_pwm = 10000;
        } else {
            float ratio = (float)(pilot_mid - signal) / (float)(pilot_mid - pilot_min); // 0..1
            float set_f = ratio * 8000.0f + 2000.0f; // 2000..10000
            base_pwm = (uint32_t)set_f;
        }
    }
    else {
        // stop
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
        return;
    }

    // 2) Nałóż steering: jedna strona przycinana do STEER_MIN_SCALE przy pełnym skręcie
    float minS = clampf(STEER_MIN_SCALE, 0.0f, 1.0f);     // np. 0.6
    float k = 1.0f - minS;                                // ile "ucięcia" przy pełnym skręcie

    float scaleL = 1.0f;
    float scaleR = 1.0f;

    if (steerc > 0.0f) {
        // skręt w prawo -> lewy silnik przycinamy
        scaleL = 1.0f - (steerc * k); // od 1 do minS
        scaleR = 1.0f;
    } else if (steerc < 0.0f) {
        // skręt w lewo -> prawy silnik przycinamy
        scaleR = 1.0f - ((-steerc) * k); // od 1 do minS
        scaleL = 1.0f;
    }

    uint32_t pwmL = (uint32_t)((float)base_pwm * scaleL);
    uint32_t pwmR = (uint32_t)((float)base_pwm * scaleR);

    pwmL = clampu32(pwmL, 0, 10000);
    pwmR = clampu32(pwmR, 0, 10000);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwmL);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwmR);
}
void pilot_set_steer_min_scale(float scale)
{
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;
    STEER_MIN_SCALE = scale;
}

float pilot_get_steer_min_scale(void)
{
    return STEER_MIN_SCALE;
}




#ifndef ENGINE_H
#define ENGINE_H

#include "data.h"
#include <stdint.h>
#include <driver/ledc.h>
#include "esp_err.h"

#define RPWM_GPIO 18
#define LPWM_GPIO 19

#define R_EN_GPIO 25
#define L_EN_GPIO 26


#define PWM_FREQ 20000
#define PWM_RES LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY ((1<<10)-1)

void pwm_init();

void motor_forward(uint16_t duty);

void motor_stop();

void motor_backward(uint16_t duty);

uint16_t get_position(engine_t *engine);

e_status_t get_engine_status(engine_t *engine);



#endif 



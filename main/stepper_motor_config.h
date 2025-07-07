#ifndef STEPPER_MOTOR_CONFIG_H
#define STEPPER_MOTOR_CONFIG_H

#include "driver/gpio.h"
#include <stdint.h>
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define SMALL_PIN1 GPIO_NUM_14
#define SMALL_PIN2 GPIO_NUM_12
#define SMALL_PIN3 GPIO_NUM_13
#define SMALL_PIN4 GPIO_NUM_15

#define BIG_PIN1 GPIO_NUM_16
#define BIG_PIN2 GPIO_NUM_5
#define BIG_PIN3 GPIO_NUM_4
#define BIG_PIN4 GPIO_NUM_2
/*
only for me to remember the pinout of the stepper motors for the blinds ;p
physical pinout:
    small:
    1 yellow
    2 light gray
    3 black
    4 dark gray

    big:
    1 yellow
    2 dark gray
    3 black
    4 light gray
*/

// to left - up = 1 | to right - down = 0
typedef enum {
    DOWN = 0,
    UP = 1
}Direction;

typedef struct {
    const uint8_t pins_blind_big[4];
    const uint8_t pins_blind_small[4];
    const uint8_t phase_pattern[4][4];
}blinds_configuration_t;

extern blinds_configuration_t blind_config;

typedef struct {
    int slider_value;
    int current_steps_blind_big;
    int max_down_position_big;
}current_parameters_big_blind_t;

extern current_parameters_big_blind_t current_parameters_big_blind;

typedef struct {
    int slider_value;
    int current_steps_blind_small;
    int max_down_position_small; 
}current_parameters_small_blind_t;

extern current_parameters_small_blind_t current_parameters_small_blind;

typedef struct {
    int blind_model;
    const uint8_t *pind_blind;
    Direction direction;
    int *max_down_position;
    int *current_step_state;
    int slider_value;
    int steps_to_do;
    int step_state_to_do;
    int *pv_to_slider_value;
}Blind_to_do_parameters_t;

Blind_to_do_parameters_t blind_to_do_parameters;

extern SemaphoreHandle_t big_blind_current_parameters_semaphore;
extern SemaphoreHandle_t small_blind_current_parameters_semaphore;


/**
 * @brief Sets the GPIO configuration for stepper motors and sets pin levels.
 * Creates semaphores for the large and small blinds as well as a semaphore for the scheduler.
 */
void stepper_motor_config();


/**
 * @brief Function for blind calibration.
 * Sets the motor direction and performs the appropriate number of steps.
 * 
 * @param pvParameters Pointer to parameters for the function.
 */
void calibration_blind(void *pvParameters);


/**
 * @brief Function to operate the blind.
 * Sets the motor direction and performs the appropriate number of steps.
 * 
 * @param pvParameters Pointer to parameters for the function.
 */
void rolling_blind(void *pvParameters);


/**
 * @brief Function to confirm full raising of the large blind.
 * 
 * @param pvParameters Pointer to parameters for the function.
 */
void confirm_full_up_big_blind(void *pvParameters);


/**
 * @brief Function to confirm full raising of the small blind.
 * 
 * @param pvParameters Pointer to parameters for the function.
 */
void confirm_full_up_small_blind(void *pvParameters);


#endif
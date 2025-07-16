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
    SMALL_BLIND = 0,
    BIG_BLIND = 1
}blind_model_t;

typedef enum {
    DOWN = 0,
    UP = 1
}direction;

typedef enum {
    WORKING,
    REQIRED_CALIBRATION,
    READY
}blind_status;

typedef struct {
    const uint8_t pins_blind_big[4];
    const uint8_t pins_blind_small[4];
    const uint8_t phase_pattern[4][4];
}blinds_configuration_t;

extern blinds_configuration_t blinds_config;

typedef struct {
    int32_t current_steps_state;
    int max_down_position;
}blind_parameters_t;

extern blind_parameters_t big_blind_parameters;
extern blind_parameters_t small_blind_parameters;

typedef struct {
    blind_model_t blind_model;
    const uint8_t *pind_blind;
    direction direction;
    int steps_state_to_do;
    int steps_to_do_calibration;
    int *max_down_position;
    int *current_steps_state;
    bool increase_values;
}blind_to_do_parameters_t;

extern SemaphoreHandle_t big_blind_current_parameters_semaphore;
extern SemaphoreHandle_t small_blind_current_parameters_semaphore;


/**
 * @brief Sets the GPIO configuration for stepper motors and sets pin levels.
 * Creates semaphores for the large and small blinds.
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


/**
 * @brief Function to confirm full lowering of the large blind.
 * 
 * @param pvParameters Pointer to parameters for the function. */
void confirm_full_down_big_blind(void *pvParameters);


/**
 * @brief Function to confirm full lowering of the small blind.
 * 
 * @param pvParameters Pointer to parameters for the function.
 */
void confirm_full_down_small_blind(void *pvParameters);

/**
 * @brief Function to initialize the start values for the blinds.
 */
void init_start_values();

/**
 * @brief Function to block the semaphores for the blinds.
 * 
 * @param blind_model The model of the blind (small or big).
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t block_semaphores(blind_model_t blind_model);

/**
 * @brief Function to release the semaphores for the blinds.
 * 
 * @param blind_model The model of the blind (small or big).
 */
void release_semaphores(blind_model_t blind_model);

void full_up_after_calib_small_blind(void *pvParameters);

void full_down_after_calib_small_blind(void *pvParameters);

void full_up_after_calib_big_blind(void *pvParameters);

void full_down_after_calib_big_blind(void *pvParameters);

#endif
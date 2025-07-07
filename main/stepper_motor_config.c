#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"       
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"    

#include "stepper_motor_config.h"
#include "scheduler_config.h"
#include "nvs_config.h"


blinds_configuration_t blind_config = {
    .phase_pattern = { 
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1} 
    },
    .pins_blind_big = {
    BIG_PIN1,
    BIG_PIN2,
    BIG_PIN3,
    BIG_PIN4
    },
    .pins_blind_small = {
    SMALL_PIN1,
    SMALL_PIN2,
    SMALL_PIN3,
    SMALL_PIN4
    }
    
};

current_parameters_big_blind_t current_parameters_big_blind = {
    .slider_value = 0,
    .current_steps_blind_big = 0,
    .max_down_position_big = 0,
};

current_parameters_small_blind_t current_parameters_small_blind = {
    .slider_value = 0,
    .current_steps_blind_small = 0,
    .max_down_position_small = 0
};

SemaphoreHandle_t big_blind_current_parameters_semaphore;
SemaphoreHandle_t small_blind_current_parameters_semaphore;


void stepper_motor_config() {

    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask = (1ULL<<BIG_PIN1) | (1ULL<<BIG_PIN2) | (1ULL<<BIG_PIN3) | (1ULL<<BIG_PIN4) | (1ULL<<SMALL_PIN1) | (1ULL<<SMALL_PIN2) | (1ULL<<SMALL_PIN3) | (1ULL<<SMALL_PIN4);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&gpio_conf);

    gpio_set_level(BIG_PIN1, 0);
    gpio_set_level(BIG_PIN2, 0);
    gpio_set_level(BIG_PIN3, 0);
    gpio_set_level(BIG_PIN4, 0);

    gpio_set_level(SMALL_PIN1, 0);
    gpio_set_level(SMALL_PIN2, 0);
    gpio_set_level(SMALL_PIN3, 0);
    gpio_set_level(SMALL_PIN4, 0);

    big_blind_current_parameters_semaphore = xSemaphoreCreateMutex();
    small_blind_current_parameters_semaphore = xSemaphoreCreateMutex();
}

void calibration_blind(void *pvParameters) {

    const char *TAG = "calibrationBlind";

    uint8_t currentStepOfMotor = 0;
    int i = 0;

    int blind_model;
    const uint8_t *pind_blind;
    Direction direction;
    int steps_to_do;
    int *max_down_position;
    int *current_step_state;
    

    Blind_to_do_parameters_t *param = (Blind_to_do_parameters_t *)pvParameters;
    blind_model = param->blind_model;
    pind_blind = param->pind_blind;
    direction = param->direction;
    steps_to_do = param->steps_to_do;
    max_down_position = param->max_down_position;
    current_step_state = param->current_step_state;
    

    printf("blind model: %d, direction: %d, steps_to_do: %d, max_down_position: %d, currentStepState: %d\n", blind_model, direction, steps_to_do, *max_down_position,  *current_step_state);


    if(blind_model){ // big blind
       if (xSemaphoreTake(big_blind_current_parameters_semaphore, 0) == pdTRUE){
            ESP_LOGI(TAG, "semaphore for big blind has been taken successfully!");
       }else{
            ESP_LOGE(TAG, "semaphore for big blind is busy! removing task");
            free(param);
            vTaskDelete(NULL);
       }
    }else{
       if (xSemaphoreTake(small_blind_current_parameters_semaphore, 0) == pdTRUE){
            ESP_LOGI(TAG, "semaphore for small blind has been taken successfully!");
       }else{
            ESP_LOGE(TAG, "semaphore for small blind is busy! removing task");
            free(param);
            vTaskDelete(NULL);
       }
    }

    switch (direction)
    {
        case UP:

            while(i < steps_to_do){
        
                currentStepOfMotor = (currentStepOfMotor + 1) % 4;
                
                for(uint8_t i=0; i<4; i++){
                    gpio_set_level(pind_blind[i], blind_config.phase_pattern[currentStepOfMotor][i]);
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                i++;
            }
        break;
    
        case DOWN:

            *max_down_position += steps_to_do; // making sure that the max_down_position is set to the new value
            *current_step_state += steps_to_do;
            
            while(i < steps_to_do){
        
                currentStepOfMotor = (currentStepOfMotor + 3) % 4;
                
                for(uint8_t i = 0; i < 4; i++){
                    gpio_set_level(pind_blind[i], blind_config.phase_pattern[currentStepOfMotor][i]);
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                i++;
            }
        break;

        default:
        break;
    }

    for(uint8_t i = 0; i < 4; i++){ // turn off the motor
        gpio_set_level(pind_blind[i], 0);
    }

    ESP_LOGI(TAG, "calibration done!");

    if(blind_model){ // big blind
        xSemaphoreGive(big_blind_current_parameters_semaphore);
            ESP_LOGI(TAG, "semaphore for big blind has been released!");
    }else{
        xSemaphoreGive(small_blind_current_parameters_semaphore);
            ESP_LOGI(TAG, "semaphore for small blind has been released!");
    }

    free(param);
    vTaskDelete(NULL);
}

void rolling_blind(void *pvParameters) { //!! used nvs to save state of blinds

    const char *TAG = "rollingBlind";
    uint8_t currentStepOfMotor = 0;
    int i = 0;

    int blind_model;
    const uint8_t *pind_blind;
    Direction direction = UP; // =UP for compilation, but it will be set in the code
    int slider_value;
    int *max_down_position;
    int *current_step_state;
    int steps_to_do;
    int step_state_to_do;
    int *strSliderValue;

    Blind_to_do_parameters_t *param = (Blind_to_do_parameters_t *)pvParameters;
    blind_model = param->blind_model;
    pind_blind = param->pind_blind;
    slider_value = param->slider_value;
    max_down_position = param->max_down_position;
    current_step_state = param->current_step_state;
    strSliderValue = param->pv_to_slider_value;

    // Blocking semapores
    if (blind_model) { // big blind
        if (xSemaphoreTake(big_blind_current_parameters_semaphore, 0) == pdTRUE) {
            ESP_LOGI(TAG, "semaphore for big blind has been taken successfully!");
            save_int_to_nvs("big", 1);
        } else {
            ESP_LOGE(TAG, "semaphore for big blind is busy! removing task");
            save_int_to_nvs("big", 0);
            free(param);
            vTaskDelete(NULL);
        }
    } else { // small blind
        if (xSemaphoreTake(small_blind_current_parameters_semaphore, 0) == pdTRUE) {
            ESP_LOGI(TAG, "semaphore for small blind has been taken successfully!");
            save_int_to_nvs("small", 1);
        } else {
            ESP_LOGE(TAG, "semaphore for small blind is busy! removing task");
            save_int_to_nvs("small", 0);
            free(param);
            vTaskDelete(NULL);
        }
    }

    printf("blind_model: %d, slider_value: %d, max_down_position: %d, current_step_state: %d\n", 
           blind_model, slider_value, *max_down_position, *current_step_state);

    step_state_to_do = (slider_value * (*max_down_position)) / 100;
    steps_to_do = *current_step_state - step_state_to_do;

    if (steps_to_do > 0) {
        direction = UP;
    } else if (steps_to_do < 0) {
        steps_to_do = steps_to_do * (-1);
        direction = DOWN;
    } else { // nothing to do
        ESP_LOGI(TAG, "nothing to do, going to clean up, steps_to_do: %d", steps_to_do);
        goto cleanup;
    }

    printf("after calculations - blind_model: %d, direction: %d, steps_to_do: %d, max_down_position: %d, currentStepState: %d, step_state_to_do: %d, slider_value: %d\n", 
           blind_model, direction, steps_to_do, *max_down_position, *current_step_state, step_state_to_do, slider_value);

    switch (direction) {
        case UP:
            if ((*current_step_state - steps_to_do) < 0) {
                ESP_LOGE(TAG, "too many steps to do UP!");
                goto cleanup;
            }

            while (i < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 1) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blind_config.phase_pattern[currentStepOfMotor][j]);
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                i++;
            }
            break;

        case DOWN:
            if ((*current_step_state + steps_to_do) > *max_down_position) {
                ESP_LOGE(TAG, "too many steps to do DOWN!");
                goto cleanup;
            }

            while (i < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 3) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blind_config.phase_pattern[currentStepOfMotor][j]);
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                i++;
            }
            break;

        default:
            break;
    }

    *current_step_state = step_state_to_do;
    *strSliderValue = slider_value;

    // turn off the motor
    for (uint8_t j = 0; j < 4; j++) {
        gpio_set_level(pind_blind[j], 0);
    }

    // write state of blinds to NVS
    if(blind_model){
        save_int_to_nvs("duzastan", *current_step_state);
        ESP_LOGI(TAG, "value of duzastan: %d", *current_step_state);
    }else{
        save_int_to_nvs("malastan", *current_step_state);
        ESP_LOGI(TAG, "value of malastan to: %d", *current_step_state);
    }
   
   // release semaphores and flags
    cleanup:
    if (blind_model) { // big blind
        xSemaphoreGive(big_blind_current_parameters_semaphore);
        save_int_to_nvs("big", 0);
        ESP_LOGI(TAG, "semaphore and flag for big blind has been taken successfully!");
    } else { // small blind
        xSemaphoreGive(small_blind_current_parameters_semaphore);
        save_int_to_nvs("small", 0);
        ESP_LOGI(TAG, "semaphore and flag for small blind has been taken successfully!");
    }


    free(param);
    vTaskDelete(NULL);
}

void confirm_full_up_big_blind(void *pvParameters) {

    const char *TAG = "confirmBigBlind";

    if (xSemaphoreTake(big_blind_current_parameters_semaphore, 0) == pdTRUE){
        ESP_LOGI(TAG, "semaphore has been taken successfully!");
    } else{
        ESP_LOGE(TAG, "semaphore is busy! removing task");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "value of max_down_position before 0: %i", current_parameters_big_blind.max_down_position_big);
    current_parameters_big_blind.max_down_position_big = 0; 
    current_parameters_big_blind.current_steps_blind_big = 0; // making sure that the currentStepsBlind is set to 0, because it is the place where the blinds are supposed to be zeroed
    ESP_LOGI(TAG, "zeroed max_down_position!");
    ESP_LOGI(TAG, "value of max_down_position after 0: %i", current_parameters_big_blind.max_down_position_big);

    xSemaphoreGive(big_blind_current_parameters_semaphore);
    ESP_LOGI(TAG, "semaphore has been released!");

    vTaskDelete(NULL);
} 

void confirm_full_up_small_blind(void *pvParameters) {

    const char *TAG = "confirmSmallBlind";

    if (xSemaphoreTake(small_blind_current_parameters_semaphore, 0) == pdTRUE){
        ESP_LOGI(TAG, "semaphore has been taken successfully!");
    } else{
        ESP_LOGE(TAG, "semaphore is busy! removing task");
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "value of max_down_position before 0: %i", current_parameters_small_blind.max_down_position_small);
    current_parameters_small_blind.max_down_position_small = 0;
    current_parameters_small_blind.current_steps_blind_small = 0; // making sure that the currentStepsBlind is set to 0, because it is the place where the blinds are supposed to be zeroed
    ESP_LOGI(TAG, "zeroed max_down_position!");
    ESP_LOGI(TAG, "value of max_down_position after 0: %i", current_parameters_small_blind.max_down_position_small);
    
    xSemaphoreGive(small_blind_current_parameters_semaphore);
    ESP_LOGI(TAG, "semaphore has been released!");

    vTaskDelete(NULL);
}
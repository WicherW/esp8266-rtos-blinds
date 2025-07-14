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

//!! check in practice if this value is good
#define DELAY_BETWEEN_STEPS  10 // milliseconds

blinds_configuration_t blinds_config = {
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

blind_parameters_t big_blind_parameters = {
    .max_down_position = 0,
    .current_steps_state = 0,
    .status = REQIRED_CALIBRATION
};

blind_parameters_t small_blind_parameters = {
    .max_down_position = 0,
    .current_steps_state = 0,
    .status = REQIRED_CALIBRATION
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
    int step_count = 0;

    blind_model_t blind_model;
    const uint8_t *pind_blind;
    direction direction;
    int steps_to_do;
    int *max_down_position;
    int *current_steps_state;

    blind_to_do_parameters_t *param = (blind_to_do_parameters_t *)pvParameters;
    blind_model = param->blind_model;
    pind_blind = param->pind_blind;
    direction = param->direction;
    max_down_position = param->max_down_position;
    current_steps_state = param->current_steps_state;
    steps_to_do = param->steps_to_do_calibration;

    esp_err_t block_check = block_semaphores(blind_model);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for blind model %d, goto cleanup", blind_model);
        goto cleanup;
    }

    switch (direction) {
        case UP:
            while (step_count < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 1) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blinds_config.phase_pattern[currentStepOfMotor][j]);
                }
                vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_STEPS));
                step_count++;
            }
            break;

        case DOWN:
            *max_down_position += steps_to_do;
            *current_steps_state += steps_to_do;

            while (step_count < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 3) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blinds_config.phase_pattern[currentStepOfMotor][j]);
                }
                vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_STEPS));
                step_count++;
            }
            break;

        default:
            break;
    }

    // turn off the motor
    for (uint8_t j = 0; j < 4; j++) {
        gpio_set_level(pind_blind[j], 0);
    }

    ESP_LOGI(TAG, "calibration done!");

    release_semaphores(blind_model);

    cleanup:
        free(param);
        vTaskDelete(NULL);
}

void rolling_blind(void *pvParameters) {

    const char *TAG = "rollingBlind";
    uint8_t currentStepOfMotor = 0;
    int steps_count = 0;

    blind_model_t blind_model;
    const uint8_t *pind_blind;
    direction direction = UP; // =UP for compilation, but it will be set later
    int *max_down_position;
    int *current_steps_state;
    int steps_to_do;
    int steps_state_to_do;

    blind_to_do_parameters_t *param = (blind_to_do_parameters_t *)pvParameters;
    blind_model = param->blind_model;
    pind_blind = param->pind_blind;
    steps_state_to_do = param->steps_state_to_do;
    max_down_position = param->max_down_position;
    current_steps_state = param->current_steps_state;

    esp_err_t block_check = block_semaphores(blind_model);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for blind model %d, goto cleanup", blind_model);
        goto cleanup;
    }

    steps_to_do = *current_steps_state - steps_state_to_do;

    if (steps_to_do > 0) {
        direction = UP;
    } else if (steps_to_do < 0) {
        steps_to_do = steps_to_do * (-1);
        direction = DOWN;
    } else if( steps_to_do == 0) {
        ESP_LOGI(TAG, "nothing to do, going to clean up, steps_to_do: %d", steps_to_do);
        release_semaphores(blind_model);
        goto cleanup;
    }

    switch (direction) {
        case UP:
            while (steps_count < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 1) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blinds_config.phase_pattern[currentStepOfMotor][j]);
                    vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_STEPS));
                }
                steps_count++;
            }
            break;

        case DOWN:
            while (steps_count < steps_to_do) {
                currentStepOfMotor = (currentStepOfMotor + 3) % 4;

                for (uint8_t j = 0; j < 4; j++) {
                    gpio_set_level(pind_blind[j], blinds_config.phase_pattern[currentStepOfMotor][j]);
                    vTaskDelay(pdMS_TO_TICKS(DELAY_BETWEEN_STEPS));
                }
                steps_count++;
            }
            break;

        default:
            break;
    }

    *current_steps_state = steps_state_to_do;

    // turn off the motor
    for (uint8_t j = 0; j < 4; j++) {
        gpio_set_level(pind_blind[j], 0);
    }

    //TODO refactor this
    // if(blind_model){
    //     save_int_to_nvs("cur_steps_big", *current_step_state);
    //     ESP_LOGI(TAG, "value of current_steps_big: %d", *current_step_state);
    // }else{
    //     save_int_to_nvs("cur_steps_sml", *current_step_state);
    //     ESP_LOGI(TAG, "value of current_steps_small: %d", *current_step_state);
    // }

    release_semaphores(blind_model);

    cleanup:
        free(param);
        vTaskDelete(NULL);
}

void confirm_full_up_big_blind(void *pvParameters) {

    const char *TAG = "confirmBigBlind";

    esp_err_t block_check = block_semaphores(BIG_BLIND);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for big blind, goto cleanup");
        goto cleanup;
    }

    big_blind_parameters.max_down_position = 0; 
    big_blind_parameters.current_steps_state = 0;

    release_semaphores(BIG_BLIND);

    cleanup:
        vTaskDelete(NULL);
} 

void confirm_full_up_small_blind(void *pvParameters) {

    const char *TAG = "confirmSmallBlind";

    esp_err_t block_check = block_semaphores(SMALL_BLIND);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for small blind, goto cleanup");
        goto cleanup;
    }

    small_blind_parameters.max_down_position = 0;
    small_blind_parameters.current_steps_state = 0; 

    release_semaphores(SMALL_BLIND);

    cleanup:
        vTaskDelete(NULL);
}

void init_start_values(void *pvParameters){

    //! max_steps_value is the same as max_down_position!!!!
    int32_t max_down_position_big = read_int_from_nvs("max_pos_big");
    int32_t max_down_position_small = read_int_from_nvs("max_pos_sml"); 
    int32_t current_steps_state_big;
    int32_t current_steps_state_small;

    if (max_down_position_big == 0) {
        big_blind_parameters.status = REQIRED_CALIBRATION;
    } else{
        current_steps_state_big = read_int_from_nvs("cur_steps_big");
        printf("current_step_state_big: %d\n", current_steps_state_big);

        big_blind_parameters.max_down_position = max_down_position_big;
        big_blind_parameters.current_steps_state = current_steps_state_big;
        big_blind_parameters.status = READY;
    }

    if (max_down_position_small == 0) {
        small_blind_parameters.status = REQIRED_CALIBRATION;
    }else {
        current_steps_state_small = read_int_from_nvs("cur_steps_sml");
        printf("current_step_state_small: %d\n", current_steps_state_small);

        small_blind_parameters.max_down_position = max_down_position_small;
        small_blind_parameters.current_steps_state = current_steps_state_small;
        small_blind_parameters.status = READY;
    }

}

esp_err_t block_semaphores(blind_model_t blind_model) {
    if (blind_model == BIG_BLIND) {
        if (xSemaphoreTake(big_blind_current_parameters_semaphore, 0) == pdTRUE) {
            ESP_LOGI("block_semaphores", "semaphore for big blind has been taken successfully!");
            return ESP_OK;
        } else {
            ESP_LOGE("block_semaphores", "semaphore for big blind is busy!");
            return ESP_FAIL;
        }
    } else {
        if (xSemaphoreTake(small_blind_current_parameters_semaphore, 0) == pdTRUE) {
            ESP_LOGI("block_semaphores", "semaphore for small blind has been taken successfully!");
            return ESP_OK;
        } else {
            ESP_LOGE("block_semaphores", "semaphore for small blind is busy!");
            return ESP_FAIL;
        }
    }

}

void release_semaphores(blind_model_t blind_model) {
    if (blind_model == BIG_BLIND) {
        xSemaphoreGive(big_blind_current_parameters_semaphore);
        ESP_LOGI("release_semaphores", "semaphore for big blind has been released!");
    } else {
        xSemaphoreGive(small_blind_current_parameters_semaphore);
        ESP_LOGI("release_semaphores", "semaphore for small blind has been released!");
    }
}

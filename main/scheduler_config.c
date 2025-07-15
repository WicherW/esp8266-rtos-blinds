#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"
#include "time.h"

#include "stepper_motor_config.h"
#include "scheduler_config.h"
#include "sntp_config.h"


Schedule scheduleArray[7] = {
    // Sunday
    {0, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Monday
    {1, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
   // Thuesday
    {2, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Thirsday
    {3, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Thursday
    {4, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Friday
    {5, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
     // Saturday
    {6, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
};

SemaphoreHandle_t scheduleSemaphore;
TimerHandle_t scheduleTimer = NULL;

void schedule_timer_callback(TimerHandle_t xTimer) {

    // const char *TAG = "scheduleTimerCallback";

    // // if(xSemaphoreTake(scheduleSemaphore, portMAX_DELAY) == pdTRUE) {
    // //     ESP_LOGI(TAG, "semaphore taken successfully!");
    // // }else{
    // //     ESP_LOGI(TAG, "blocking semaphore failed!");
    // //     return;
    // // }

    // esp_err_t err = block_semaphores(SMALL_BLIND);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to block semaphores for small blind, returning");
    //     return;
    // }
    // esp_err_t err = block_semaphores(BIG_BLIND);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to block semaphores for big blind, returning");
    //     return;
    // }

    // struct tm current_time = get_date_and_time();
    // int current_day = current_time.tm_wday;
    // int current_hour = current_time.tm_hour;
    // int current_min = current_time.tm_min;

    // // rolling up check
    // if (current_hour == scheduleArray[current_day].open_time.tm_hour &&
    //     current_min == scheduleArray[current_day].open_time.tm_min) {
    //     ESP_LOGI(TAG, "blinds are going up!");

    //     blind_to_do_parameters_t *paramSmall = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
    //     blind_to_do_parameters_t *paramBig = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));

    //     paramSmall->blind_model = SMALL_BLIND;
    //     paramSmall->pind_blind = blinds_config.pins_blind_small;
    //     paramSmall->direction = UP;
    //     paramSmall->steps_state_to_do = 0;

    //     paramBig->blind_model = BIG_BLIND;
    //     paramBig->pind_blind = blinds_config.pins_blind_big;
    //     paramBig->direction = UP;
    //     paramBig->steps_state_to_do = 0;

    //     xTaskCreate(&rolling_blind, "schedulerUpSmall", 2048, (void*)paramSmall, 3, NULL);
    //     xTaskCreate(&rolling_blind, "schedulerUpBig", 2048, (void*)paramBig, 3, NULL);
    // }

    // // rolling down check
    // if (current_hour == scheduleArray[current_day].close_time.tm_hour &&
    //     current_min == scheduleArray[current_day].close_time.tm_min) {
    //     ESP_LOGI(TAG, "blinds are going down!");
        
    //     blind_to_do_parameters_t *paramSmall = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
    //     blind_to_do_parameters_t *paramBig = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));

    //     paramSmall->blind_model = SMALL_BLIND;
    //     paramSmall->pind_blind = blinds_config.pins_blind_small;
    //     paramSmall->direction = DOWN;
    //     paramSmall->steps_state_to_do = 100;

    //     paramBig->blind_model = BIG_BLIND;
    //     paramBig->pind_blind = blinds_config.pins_blind_big;
    //     paramBig->direction = DOWN;
    //     paramBig->steps_state_to_do = 100;

    //     xTaskCreate(&rolling_blind, "schedulerDownSmall", 2048, (void*)paramSmall, 3, NULL);
    //     xTaskCreate(&rolling_blind, "schedulerDownBig", 2048, (void*)paramBig, 3, NULL);
    // }

    // return;
}
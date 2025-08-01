#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "cJSON.h"
#include <time.h>
#include "stdio.h"
#include "string.h"

#include "stepper_motor_config.h"
#include "scheduler_config.h"
#include "sntp_config.h"
#include "nvs_config.h"


schedule scheduleArray[7] = {
    // Sunday
    {0, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Monday
    {1, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Tuesday
    {2, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Wednesday
    {3, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Thursday
    {4, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Friday
    {5, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
    // Saturday
    {6, {.tm_hour=7, .tm_min=0}, {.tm_hour=20, .tm_min=0}},
};

SemaphoreHandle_t scheduleSemaphore;
TimerHandle_t scheduleTimer;

void print_schedule_array() {
    const char* days[] = {
        "Sunday", "Monday", "Tuesday",
        "Wednesday", "Thursday", "Friday", "Saturday"
    };

    ESP_LOGI("schedule", "=== Schedule Dump ===");
    for (int i = 0; i < 7; i++) {
        if (scheduleArray[i].open_time.tm_hour >= 0 && scheduleArray[i].close_time.tm_hour >= 0) {
            ESP_LOGI("schedule", "%s: %02d:%02d - %02d:%02d",
                days[scheduleArray[i].day],
                scheduleArray[i].open_time.tm_hour,
                scheduleArray[i].open_time.tm_min,
                scheduleArray[i].close_time.tm_hour,
                scheduleArray[i].close_time.tm_min);
        } else {
            ESP_LOGI("schedule", "%s: disabled (--:--)", days[scheduleArray[i].day]);
        }
    }
}

void schedule_timer_callback(TimerHandle_t xTimer) {

    const char *TAG = "scheduleTimerCallback";

    if(xSemaphoreTake(scheduleSemaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "schedule semaphore taken successfully!");
    }else{
        ESP_LOGI(TAG, "blocking schedule semaphore failed!");
        return;
    }

    esp_err_t err1 = block_semaphore(SMALL_BLIND);
    if (err1 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for small blind, returning");
        xSemaphoreGive(scheduleSemaphore);
        return;
    }
    esp_err_t err2 = block_semaphore(BIG_BLIND);
    if (err2 != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for big blind, returning");
        xSemaphoreGive(scheduleSemaphore);
        release_semaphore(SMALL_BLIND);
        return;
    }

    struct tm current_time = get_date_and_time();
    int current_day = current_time.tm_wday;
    int current_hour = current_time.tm_hour;
    int current_min = current_time.tm_min;

    ESP_LOGI(TAG, "Current time: %02d:%02d on day %d", current_hour, current_min, current_day);

    // rolling up check
    if (current_hour == scheduleArray[current_day].open_time.tm_hour &&
        current_min == scheduleArray[current_day].open_time.tm_min) {
        ESP_LOGI(TAG, "blinds are going up!");

        blind_to_do_parameters_t *paramSmall = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
        blind_to_do_parameters_t *paramBig = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));

        paramSmall->blind_model = SMALL_BLIND;
        paramSmall->pind_blind = blinds_config.pins_blind_small;
        paramSmall->direction = UP;
        paramSmall->steps_state_to_do = 0;

        paramBig->blind_model = BIG_BLIND;
        paramBig->pind_blind = blinds_config.pins_blind_big;
        paramBig->direction = UP;
        paramBig->steps_state_to_do = 0;
        
        xSemaphoreGive(scheduleSemaphore);
        xTaskCreate(&rolling_blind, "schedulerUpSmall", 2048, (void*)paramSmall, 3, NULL);
        xTaskCreate(&rolling_blind, "schedulerUpBig", 2048, (void*)paramBig, 3, NULL);
    }

    // rolling down check
    if (current_hour == scheduleArray[current_day].close_time.tm_hour &&
        current_min == scheduleArray[current_day].close_time.tm_min) {
        ESP_LOGI(TAG, "blinds are going down!");
        
        blind_to_do_parameters_t *paramSmall = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
        blind_to_do_parameters_t *paramBig = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));

        paramSmall->blind_model = SMALL_BLIND;
        paramSmall->pind_blind = blinds_config.pins_blind_small;
        paramSmall->direction = DOWN;
        paramSmall->steps_state_to_do = small_blind_parameters.max_down_position;

        paramBig->blind_model = BIG_BLIND;
        paramBig->pind_blind = blinds_config.pins_blind_big;
        paramBig->direction = DOWN;
        paramBig->steps_state_to_do = big_blind_parameters.max_down_position;

        xSemaphoreGive(scheduleSemaphore);
        xTaskCreate(&rolling_blind, "schedulerDownSmall", 2048, (void*)paramSmall, 3, NULL);
        xTaskCreate(&rolling_blind, "schedulerDownBig", 2048, (void*)paramBig, 3, NULL);
    }

    release_semaphore(SMALL_BLIND);
    release_semaphore(BIG_BLIND);
    xSemaphoreGive(scheduleSemaphore);
    return;
}


void parse_schedule_json_from_client(void *pvParameters) {
    const char *json_data = (const char *)pvParameters;
    cJSON *json_schedule = cJSON_Parse(json_data);
    if (json_schedule == NULL) {
        ESP_LOGE("schedule", "error parsing schedule JSON!");
        free((void *)json_data);
        vTaskDelete(NULL);
        return;
    }

    if (!cJSON_IsArray(json_schedule)) {
        ESP_LOGE("schedule", "JSON is not an array!");
        cJSON_Delete(json_schedule);
        free((void *)json_data);
        vTaskDelete(NULL);
        return;
    }

    for (int i = 0; i < cJSON_GetArraySize(json_schedule); i++) {
        cJSON *entry = cJSON_GetArrayItem(json_schedule, i);
        if (!cJSON_IsObject(entry)) {
            continue;
        }

        cJSON *day_item = cJSON_GetObjectItem(entry, "day");
        cJSON *open_item = cJSON_GetObjectItem(entry, "open_time");
        cJSON *close_item = cJSON_GetObjectItem(entry, "close_time");

        if (!cJSON_IsNumber(day_item) || !cJSON_IsString(open_item) || !cJSON_IsString(close_item)) {
            ESP_LOGW("schedule", "invalid schedule entry at index %d", i);
            continue;
        }

        int day = day_item->valueint;

        if (day < 0 || day > 6) {
            ESP_LOGW("schedule", "invalid day value: %d", day);
            continue;
        }

        const char *open_time = open_item->valuestring;
        const char *close_time = close_item->valuestring;

        if (strcmp(open_time, "--:--") == 0 || strcmp(close_time, "--:--") == 0) {
            scheduleArray[day].open_time.tm_hour = -1;
            scheduleArray[day].close_time.tm_hour = -1;
        } else {
            sscanf(open_time,  "%2d:%2d", &scheduleArray[day].open_time.tm_hour,  &scheduleArray[day].open_time.tm_min);
            sscanf(close_time, "%2d:%2d", &scheduleArray[day].close_time.tm_hour, &scheduleArray[day].close_time.tm_min);
        }

        scheduleArray[day].day = day;
    }

    save_schedule_to_nvs();

    cJSON_Delete(json_schedule);
    free((void *)json_data);
    vTaskDelete(NULL);
}

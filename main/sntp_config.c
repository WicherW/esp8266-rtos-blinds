#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <time.h>

#include "project_config.h"

void sntp_initialize() {

    const char *TAG = "sntp_initialize";

    ESP_LOGI(TAG, "Inicialization SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pl.pool.ntp.org");
    sntp_setservername(1, "pool.ntp.org"); 
    sntp_init();
    
    ESP_LOGI(TAG, "SNTP has been initialized");
}

struct tm get_date_and_time() {

    const char *TAG = "getDataAndTime";

    ESP_LOGI(TAG, "function started");

    time_t now = 0;
    struct tm timeinfo = { 0 };

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        
       sntp_initialize();

        time_t now = 0;
        struct tm timeinfo = { 0 };
        int retry = 0;
        const int retry_count = 10;

        while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            time(&now);
            localtime_r(&now, &timeinfo);
        }
    }
    
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();  // time zone set to Central European Time (CET)

    int retry = 0;
    const int max_retries = 30;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGE(TAG, "The current date/time error");
    } else {
        ESP_LOGE(TAG, "Current date/time reached!");
    }

    return timeinfo;
}


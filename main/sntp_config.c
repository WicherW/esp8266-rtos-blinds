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

    ESP_LOGI(TAG, "Initialization SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pl.pool.ntp.org");
    sntp_setservername(1, "pool.ntp.org");
    sntp_init();

    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1); // Set timezone to Central European Time (CET)
    tzset();

    ESP_LOGI(TAG, "SNTP has been initialized and timezone set");
}


struct tm get_date_and_time() {
    const char *TAG = "getDataAndTime";

    ESP_LOGI(TAG, "Function started");

    time_t now = 0;
    struct tm timeinfo = { 0 };

    time(&now);
    localtime_r(&now, &timeinfo);

    int retry = 0;
    const int retry_count = 15;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGW(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGE(TAG, "System time is not set after retries.");
    } else {
        ESP_LOGI(TAG, "Current date/time successfully obtained.");
    }

    return timeinfo;
}
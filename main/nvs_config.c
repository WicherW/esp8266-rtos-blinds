#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"
#include "scheduler_config.h"
#include "esp_log.h"


void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}


void save_int_to_nvs(const char *key, int32_t value) {
    nvs_handle handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        err = nvs_set_i32(handle, key, value);
        if (err == ESP_OK) {
            nvs_commit(handle);
        }
        nvs_close(handle);
    }
}


int32_t read_int_from_nvs(const char *key) {
    nvs_handle handle;
    int32_t value = 0;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);
    if (err == ESP_OK) {
        err = nvs_get_i32(handle, key, &value);
        nvs_close(handle);
    }
    return value;
}


esp_err_t read_schedule_from_nvs() {

    nvs_handle handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) return err;

    size_t required_size = sizeof(scheduleArray);
    err = nvs_get_blob(handle, "schedule", scheduleArray, &required_size); // scheduleArray will be filled with data
    nvs_close(handle);

    ESP_LOGI("read_schedule_from_nvs", "AFTER READING FROM NVS, print schedule_array");
    print_schedule_array();
    return err;
}


esp_err_t save_schedule_to_nvs() {

    ESP_LOGI("save_schedule_to_nvs", "SAVE TO NVS, print schedule_array");
    print_schedule_array();

    nvs_handle handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, "schedule", scheduleArray, sizeof(scheduleArray));
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}
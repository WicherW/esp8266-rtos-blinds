#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"


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


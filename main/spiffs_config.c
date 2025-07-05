#include "esp_spiffs.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "project_config.h"

#define TAG_SPIFFS "spiffs"

void init_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = false
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG_SPIFFS, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG_SPIFFS, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG_SPIFFS, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    } else{
        ESP_LOGI(TAG_SPIFFS, "Spiffs registered successfuly");
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("storage", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_SPIFFS, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG_SPIFFS, "Partition size: total: %d, used: %d", total, used);
    }
}

esp_err_t read_file(const char *file_path, char **buffer, size_t *length) {
    struct stat st;

    if (stat(file_path, &st) != 0) {
        ESP_LOGE(TAG_SPIFFS, "File does not exist: %s", file_path);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG_SPIFFS, "File size from stat: %ld", (long)st.st_size);

    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG_SPIFFS, "Failed to open file for reading: %s", file_path);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG_SPIFFS, "File opened successfully: %s", file_path);

    *buffer = malloc(st.st_size + 1);
    if (*buffer == NULL) {
        ESP_LOGE(TAG_SPIFFS, "Failed to allocate memory for file buffer");
        fclose(f);
        return ESP_FAIL;
    }else{
        ESP_LOGI(TAG_SPIFFS, "Allocated %ld bytes for file buffer", st.st_size + 1);
    }

    size_t total_read = 0;
    while (total_read < st.st_size) {
        size_t bytes = fread(*buffer + total_read, 1, st.st_size - total_read, f);
        if (bytes == 0) {

            if (feof(f)) ESP_LOGE(TAG_SPIFFS, "EOF reached prematurely");
            if (ferror(f)) ESP_LOGE(TAG_SPIFFS, "File error occurred during fread");

            ESP_LOGE(TAG_SPIFFS, "Error reading file at byte %u", total_read);
            free(*buffer);
            fclose(f);
            return ESP_FAIL;
        }
        total_read += bytes;
    }

    (*buffer)[total_read] = '\0';
    *length = total_read;

    fclose(f);
    ESP_LOGI(TAG_SPIFFS, "File closed: %s", file_path);
    return ESP_OK;
}



#include "esp_log.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"    
#include "stdio.h"            
#include "stdlib.h"            
#include "string.h"

#include "http_get_handlers.h"           
#include "project_config.h"
#include "spiffs_config.h"
#include "stepper_motor_config.h"
#include "nvs_config.h"

#define JSON_BUFFER_SIZE 2048

esp_err_t html_get_handler(httpd_req_t *req) {
    char *file_buffer = NULL;
    size_t file_length = 0;

    if (read_file("/spiffs/index.html", &file_buffer, &file_length) == ESP_OK) {
        httpd_resp_set_type(req, "text");
        esp_err_t ret = httpd_resp_send(req, file_buffer, file_length);
        free(file_buffer);
        return ret;
    }

    return httpd_resp_send_500(req);
}
httpd_uri_t main_page_t = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = html_get_handler,
    .user_ctx  = NULL
};

// TODO refactor this, move whole logic to tasks
esp_err_t devdata_get_feedback_smallblind_handler(httpd_req_t *req) {
    
    const char *TAG = "smallBlindDevDataHandler";

    esp_err_t block_check = block_semaphore(SMALL_BLIND);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for small blind");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "semaphore has been taken successfully!");

    char *json_string = malloc(JSON_BUFFER_SIZE);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        release_semaphore(SMALL_BLIND);
        return ESP_FAIL;
    }

    snprintf(json_string, JSON_BUFFER_SIZE, "{\"smallmaxdownposition\":%i,\"smallcurrentstepsstate\":%i}",
             small_blind_parameters.max_down_position,
             small_blind_parameters.current_steps_state);

    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "JSON sent successfully!");
    } else {
        ESP_LOGE(TAG, "JSON respond sending failed!");
    }

    free(json_string);
    release_semaphore(SMALL_BLIND);
    ESP_LOGI(TAG, "semaphore has been released!");
    return ret;
}
httpd_uri_t small_dev_data_t = {
    .uri       = "/smalldevdata",
    .method    = HTTP_GET,
    .handler   = devdata_get_feedback_smallblind_handler,
    .user_ctx  = NULL
};


esp_err_t devdata_get_feedback_bigblind_handler(httpd_req_t *req) {

    const char *TAG = "bigBlindDevDataHandler";

    esp_err_t block_check = block_semaphore(BIG_BLIND);
    if(block_check != ESP_OK) {
        ESP_LOGE(TAG, "Failed to block semaphores for big blind");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "semaphore has been taken successfully!");

    char *json_string = malloc(JSON_BUFFER_SIZE);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        release_semaphore(BIG_BLIND);
        return ESP_FAIL;
    }

    snprintf(json_string, JSON_BUFFER_SIZE, "{\"bigmaxdownposition\":%i,\"bigcurrentstepsstate\":%i}",
             big_blind_parameters.max_down_position,
             big_blind_parameters.current_steps_state);

    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "JSON sent successfully!");
    } else {
        ESP_LOGE(TAG, "JSON respond sending failed!");
    }

    free(json_string);
    release_semaphore(BIG_BLIND);
    ESP_LOGI(TAG, "semaphore has been released!");
    return ret;
}
httpd_uri_t big_dev_data_t = {
    .uri       = "/bigdevdata",
    .method    = HTTP_GET,
    .handler   = devdata_get_feedback_bigblind_handler,
    .user_ctx  = NULL
};


esp_err_t fillinputs_get_feedback_handler(httpd_req_t *req) {

    const char *TAG = "fillInputsHandler";
    esp_err_t res = ESP_FAIL;
    char *json_string = NULL;

    if (xSemaphoreTake(big_blind_current_parameters_semaphore, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "semaphore for big blind is busy!");
        return ESP_FAIL;
    }
    
    if (xSemaphoreTake(small_blind_current_parameters_semaphore, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "semaphore for small blind is busy!");
        goto release_big;
    }

    ESP_LOGI(TAG, "semaphores have been taken successfully!");


    json_string = malloc(JSON_BUFFER_SIZE);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        goto release_both;
    }

    char schedule_part[1500];
    char entry[128];        

    schedule_part[0] = '\0';

    for (int i = 0; i < 7; i++) {
        if (scheduleArray[i].open_time.tm_hour >= 0 && scheduleArray[i].close_time.tm_hour >= 0) {
            snprintf(entry, sizeof(entry),
                     "{\"day\":%d,\"open_time\":\"%02d:%02d\",\"close_time\":\"%02d:%02d\"}",
                     scheduleArray[i].day,
                     scheduleArray[i].open_time.tm_hour, scheduleArray[i].open_time.tm_min,
                     scheduleArray[i].close_time.tm_hour, scheduleArray[i].close_time.tm_min);
        } else {
            snprintf(entry, sizeof(entry),
                     "{\"day\":%d,\"open_time\":\"--:--\",\"close_time\":\"--:--\"}",
                     scheduleArray[i].day);
        }

        strcat(schedule_part, entry);
        if (i < 6) strcat(schedule_part, ",");
    }


    snprintf(json_string, JSON_BUFFER_SIZE,
             "{\"smallmaxdownposition\":%d,"
             "\"bigmaxdownposition\":%d,"
             "\"schedule\":[%s]}",
             small_blind_parameters.max_down_position,
             big_blind_parameters.max_down_position,
             schedule_part);


    if (httpd_resp_send(req, json_string, strlen(json_string)) != ESP_OK) {
        ESP_LOGE(TAG, "JSON respond sending failed!");
        goto release_both;
    }

    ESP_LOGI(TAG, "JSON sent successfully!");
    res = ESP_OK;

    release_both:
    xSemaphoreGive(small_blind_current_parameters_semaphore);
    release_big:
    xSemaphoreGive(big_blind_current_parameters_semaphore);
    if (json_string) {
        free(json_string);
    }
    
    ESP_LOGI(TAG, "semaphore has been released!");
    return res;
}
httpd_uri_t fill_inputs_t = {
    .uri       = "/fillinputs",
    .method    = HTTP_GET,
    .handler   = fillinputs_get_feedback_handler,
    .user_ctx  = NULL
};
// TODO -----------------------------------------------
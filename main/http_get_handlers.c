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


esp_err_t devdata_get_feedback_smallblind_handler(httpd_req_t *req) {
    
    const char *TAG = "smallBlindDevDataHandler";

    if (xSemaphoreTake(small_blind_current_parameters_semaphore, 0) != pdTRUE) {
        ESP_LOGE(TAG, "semaphore is busy!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "semaphore has been taken successfully!");

    char *json_string = malloc(120);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        xSemaphoreGive(small_blind_current_parameters_semaphore);
        return ESP_FAIL;
    }

    snprintf(json_string, 120, "{\"maxdownsmallblind\":%i,\"smallcurrentstepsstate\":%i}",
             current_parameters_small_blind.max_down_position_small,
             current_parameters_small_blind.current_steps_blind_small);

    esp_err_t ret = httpd_resp_send(req, json_string, strlen(json_string));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "JSON sent successfully!");
    } else {
        ESP_LOGE(TAG, "JSON respond sending failed!");
    }

    free(json_string);
    xSemaphoreGive(small_blind_current_parameters_semaphore);
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

    if (xSemaphoreTake(big_blind_current_parameters_semaphore, 0) != pdTRUE) {
        ESP_LOGE(TAG, "semaphore is busy!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "semaphore has been taken successfully!");

    char *json_string = malloc(JSON_BLIND_DATA);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        xSemaphoreGive(big_blind_current_parameters_semaphore);
        return ESP_FAIL;
    }

    sprintf(json_string, "{\"maxdownbigblind\":%i,\"bigcurrentstepsstate\":%i}", 
            current_parameters_big_blind.max_down_position_big, 
            current_parameters_big_blind.current_steps_blind_big);

    esp_err_t res = ESP_OK;
    if (httpd_resp_send(req, json_string, strlen(json_string)) == ESP_OK) {
        ESP_LOGI(TAG, "JSON sent successfully!");
    } else {
        ESP_LOGE(TAG, "JSON respond sending failed!");
        res = ESP_FAIL;
    }

    xSemaphoreGive(big_blind_current_parameters_semaphore);
    free(json_string);

    return res;
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

    int32_t smallFlag = read_int_from_nvs("small");
    int32_t bigFlag = read_int_from_nvs("big");

    json_string = malloc(JSON_CONST_SIZE);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "error of malloc!");
        goto release_both;
    }

    snprintf(json_string, JSON_CONST_SIZE, 
        "{\"smallSlider\":%i,\"bigSlider\":%i,\"smallFlag\":%i,\"bigFlag\":%i}", 
        current_parameters_small_blind.slider_value, 
        current_parameters_big_blind.slider_value, 
        smallFlag,
        bigFlag);

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
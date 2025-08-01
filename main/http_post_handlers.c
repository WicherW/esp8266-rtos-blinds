#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "sys/param.h"
#include "esp_log.h"       
#include "cJSON.h"            
#include <stdlib.h>             
#include <string.h>             

#include "project_config.h"
#include "stepper_motor_config.h"
#include "scheduler_config.h"

esp_err_t calibration_post_handler(httpd_req_t *req){
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }

        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);
     
        // BIG BLIND
        // down calibration
        cJSON *move_down_big_blind_JSON = cJSON_GetObjectItem(json, "bigCalibDown");
        if(cJSON_IsNumber(move_down_big_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "big blind - move_down_big_blind_JSON: %i", move_down_big_blind_JSON->valueint);
            if(move_down_big_blind_JSON->valueint){
                blind_to_do_parameters_t *param_big_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_big_blind->blind_model = BIG_BLIND;
                param_big_blind->pind_blind = blinds_config.pins_blind_big;
                param_big_blind->direction = DOWN;
                param_big_blind->max_down_position = &big_blind_parameters.max_down_position;
                param_big_blind->current_steps_state = &big_blind_parameters.current_steps_state;
                param_big_blind->steps_to_do_calibration = move_down_big_blind_JSON->valueint;
                param_big_blind->increase_values = true;
                xTaskCreate(&calibration_blind, "calibrateRollblindDownBig", 2048, (void*)param_big_blind, 3, NULL);
            }
           
        }

        // SMALL BLIND
        // down calibration
        cJSON *move_down_small_blind_JSON = cJSON_GetObjectItem(json, "smallCalibDown");
        if(cJSON_IsNumber(move_down_small_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "move_down_small_blind_JSON: %i", move_down_small_blind_JSON->valueint);
            if(move_down_small_blind_JSON->valueint){
                blind_to_do_parameters_t *param_small_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_small_blind->blind_model = SMALL_BLIND;
                param_small_blind->pind_blind = blinds_config.pins_blind_small;
                param_small_blind->direction = DOWN;
                param_small_blind->max_down_position = &small_blind_parameters.max_down_position;
                param_small_blind->current_steps_state = &small_blind_parameters.current_steps_state;
                param_small_blind->steps_to_do_calibration = move_down_small_blind_JSON->valueint;
                param_small_blind->increase_values = true;
                
                xTaskCreate(&calibration_blind, "calibrateRollblindDownSmall", 2048, (void*)param_small_blind, 3, NULL);
            }
        }
        
        if (json == NULL) {
            ESP_LOGE(TAG_SERVER, "error parsing JSON");
            return ESP_FAIL;
        }

        cJSON_Delete(json);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;  
}
httpd_uri_t calibration_t = {
    .uri       = "/calibration",
    .method    = HTTP_POST,
    .handler   = calibration_post_handler,
    .user_ctx  = NULL
};

esp_err_t sliders_post_handler(httpd_req_t *req){
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }

            return ESP_FAIL;
        }

        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);
     
        // small blind - slider value
        cJSON *small_blind_slider_JSON = cJSON_GetObjectItem(json, "smallSlider");
        if(cJSON_IsNumber(small_blind_slider_JSON)){

            ESP_LOGI(TAG_SERVER, "small_blind_slider_JSON: %i", small_blind_slider_JSON->valueint);

            blind_to_do_parameters_t *small_param = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
            small_param->blind_model = SMALL_BLIND;
            small_param->pind_blind = blinds_config.pins_blind_small;
            small_param->max_down_position = &small_blind_parameters.max_down_position;
            small_param->current_steps_state = &small_blind_parameters.current_steps_state;
            small_param->steps_state_to_do = small_blind_slider_JSON->valueint;

            xTaskCreate(&rolling_blind, "moveSmallBlind", 2048, (void*)small_param, 3, NULL);
        }

        // big blind - slider value
        cJSON *big_blind_slider_JSON = cJSON_GetObjectItem(json, "bigSlider");
        if(cJSON_IsNumber(big_blind_slider_JSON)){

            ESP_LOGI(TAG_SERVER, "big_blind_slider_JSON: %i", big_blind_slider_JSON->valueint);

            blind_to_do_parameters_t *big_param = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
            big_param->blind_model = BIG_BLIND;
            big_param->pind_blind = blinds_config.pins_blind_big;
            big_param->max_down_position = &big_blind_parameters.max_down_position;
            big_param->current_steps_state = &big_blind_parameters.current_steps_state;
            big_param->steps_state_to_do = big_blind_slider_JSON->valueint;
            
            xTaskCreate(&rolling_blind, "moveBigBlind", 2048, (void*)big_param, 3, NULL);
        }

        
        if (json == NULL) {
            ESP_LOGE(TAG_SERVER, "error parsing JSON");
            return ESP_FAIL;
        }

        cJSON_Delete(json);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
httpd_uri_t sliders_t = {
    .uri       = "/sliders",
    .method    = HTTP_POST,
    .handler   = sliders_post_handler,
    .user_ctx  = NULL
};

esp_err_t confirm_full_up_post_handler(httpd_req_t *req){
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }

            return ESP_FAIL;
        }

        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);
        cJSON *confirm_full_up_position_JSON = cJSON_GetObjectItem(json, "confirmUpBlindId");
        if (cJSON_IsString(confirm_full_up_position_JSON) && (confirm_full_up_position_JSON->valuestring != NULL)) {

            ESP_LOGI(TAG_SERVER, "confirm_full_up_position_JSON: %s", confirm_full_up_position_JSON->valuestring);

            if(strcmp(confirm_full_up_position_JSON->valuestring, "smallUpConfirmButton") == 0){
                xTaskCreate(&confirm_full_up_small_blind, "ConfirmFullUpSmallBlind", 2048, NULL, 3, NULL);

            } else if(strcmp(confirm_full_up_position_JSON->valuestring, "bigUpConfirmButton") == 0){
                xTaskCreate(&confirm_full_up_big_blind, "ConfirmFullUpBigBlind", 2048, NULL, 3, NULL);
            }
        } 
        
        if (json == NULL) {
            ESP_LOGE(TAG_SERVER, "error parsing JSON");
            return ESP_FAIL;
        }

        cJSON_Delete(json);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
httpd_uri_t confirm_full_up_t = {
    .uri       = "/confirm-full-up",
    .method    = HTTP_POST,
    .handler   = confirm_full_up_post_handler,
    .user_ctx  = NULL
};

esp_err_t schedule_post_handler(httpd_req_t *req) {
    ESP_LOGI("harmonogramposthandler", "weszlo na poczatek");
    
    char *total_data = (char *)pvPortMalloc(req->content_len + 1);
    if (total_data == NULL) {
        return ESP_ERR_NO_MEM;
    }
    memset(total_data, 0, req->content_len + 1);

    ESP_LOGI("harmonogramposthandler", "after memory allocation, content_len: %d", req->content_len);

    int ret;
    int remaining = req->content_len;
    int received = 0;

    while (remaining > 0) {
        ret = httpd_req_recv(req, total_data + received, remaining);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            free(total_data);
            return ESP_FAIL;
        }
        received += ret;
        remaining -= ret;
    }

    ESP_LOGI("harmonogramposthandler", "after receiving data, received: %d, total_data: %s", received, total_data);

    total_data[received] = '\0';

    xTaskCreate(&parse_schedule_json_from_client, "calibrateRollblindDownBig", 2048, (void*)total_data, 3, NULL);

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
httpd_uri_t schedule_t = {
    .uri       = "/schedule",
    .method    = HTTP_POST,
    .handler   = schedule_post_handler,
    .user_ctx  = NULL
};

esp_err_t move_blind_post_handler(httpd_req_t *req){
 char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return ESP_FAIL;
        }

        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);

        if (json == NULL) {
            ESP_LOGE(TAG_SERVER, "error parsing JSON");
            return ESP_FAIL;
        }
     
        // BIG BLIND
        // move down
        cJSON *move_down_big_blind_JSON = cJSON_GetObjectItem(json, "bigMoveDown");
        if(cJSON_IsNumber(move_down_big_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "big blind - move_down_big_blind_JSON: %i", move_down_big_blind_JSON->valueint);
            if(move_down_big_blind_JSON->valueint){
                blind_to_do_parameters_t *param_big_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_big_blind->blind_model = BIG_BLIND;
                param_big_blind->pind_blind = blinds_config.pins_blind_big;
                param_big_blind->direction = DOWN;
                param_big_blind->steps_to_do_calibration = move_down_big_blind_JSON->valueint;
                param_big_blind->increase_values = false;

                xTaskCreate(&calibration_blind, "calibrateRollblindDownBig", 2048, (void*)param_big_blind, 3, NULL);
            }
           
        }
        // move up
        cJSON *move_up_big_blind_JSON = cJSON_GetObjectItem(json, "bigMoveUp");
        if(cJSON_IsNumber(move_up_big_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "big blind - move_down_big_blind_JSON: %i", move_up_big_blind_JSON->valueint);
            if(move_up_big_blind_JSON->valueint){
                blind_to_do_parameters_t *param_big_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_big_blind->blind_model = BIG_BLIND;
                param_big_blind->pind_blind = blinds_config.pins_blind_big;
                param_big_blind->direction = UP;
                param_big_blind->steps_to_do_calibration = move_up_big_blind_JSON->valueint;
                param_big_blind->increase_values = false;

                xTaskCreate(&calibration_blind, "calibrateRollblindDownBig", 2048, (void*)param_big_blind, 3, NULL);
            }
           
        }

        // SMALL BLIND
        // move down
        cJSON *move_down_small_blind_JSON = cJSON_GetObjectItem(json, "smallMoveDown");
        if(cJSON_IsNumber(move_down_small_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "move_down_small_blind_JSON: %i", move_down_small_blind_JSON->valueint);
            if(move_down_small_blind_JSON->valueint){
                blind_to_do_parameters_t *param_small_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_small_blind->blind_model = SMALL_BLIND;
                param_small_blind->pind_blind = blinds_config.pins_blind_small;
                param_small_blind->direction = DOWN;
                param_small_blind->steps_to_do_calibration = move_down_small_blind_JSON->valueint;
                param_small_blind->increase_values = false;
                
                xTaskCreate(&calibration_blind, "calibrateRollblindDownSmall", 2048, (void*)param_small_blind, 3, NULL);
            }
        }

        // move up
        cJSON *move_up_small_blind_JSON = cJSON_GetObjectItem(json, "smallMoveUp");
        if(cJSON_IsNumber(move_up_small_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "move_down_small_blind_JSON: %i", move_up_small_blind_JSON->valueint);
            if(move_up_small_blind_JSON->valueint){
                blind_to_do_parameters_t *param_small_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_small_blind->blind_model = SMALL_BLIND;
                param_small_blind->pind_blind = blinds_config.pins_blind_small;
                param_small_blind->direction = UP;
                param_small_blind->steps_to_do_calibration = move_up_small_blind_JSON->valueint;
                param_small_blind->increase_values = false;
                
                xTaskCreate(&calibration_blind, "calibrateRollblindDownSmall", 2048, (void*)param_small_blind, 3, NULL);
            }
        }
        
        cJSON_Delete(json);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;  
};
httpd_uri_t move_blind_t = {
    .uri       = "/move-blind",
    .method    = HTTP_POST,
    .handler   = move_blind_post_handler,
    .user_ctx  = NULL
};

esp_err_t after_calib_state_handler(httpd_req_t *req){
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }

            return ESP_FAIL;
        }

        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        cJSON *json = cJSON_Parse(buf);

        if (json == NULL) {
            ESP_LOGE(TAG_SERVER, "error parsing JSON");
            return ESP_FAIL;
        }

        cJSON *states_after_calib_JSON = cJSON_GetObjectItem(json, "state_after_calibration");
        if (cJSON_IsString(states_after_calib_JSON) && (states_after_calib_JSON->valuestring != NULL)) {

            ESP_LOGI(TAG_SERVER, "states_after_calib_JSON: %s", states_after_calib_JSON->valuestring);

            if(strcmp(states_after_calib_JSON->valuestring, "smallFullUpAfterCalibration") == 0){
                xTaskCreate(&full_up_after_calib_small_blind, "full_up_after_calib_small_blind", 2048, NULL, 3, NULL);

            } else if(strcmp(states_after_calib_JSON->valuestring, "smallFullDownAfterCalibration") == 0){
                xTaskCreate(&full_down_after_calib_small_blind, "full_down_after_calib_small_blind", 2048, NULL, 3, NULL);

            }else if(strcmp(states_after_calib_JSON->valuestring, "bigFullUpAfterCalibration") == 0){
                xTaskCreate(&full_up_after_calib_big_blind, "full_up_after_calib_big_blind", 2048, NULL, 3, NULL);

            }else if(strcmp(states_after_calib_JSON->valuestring, "bigFullDownAfterCalibration") == 0){
                xTaskCreate(&full_down_after_calib_big_blind, "full_down_after_calib_big_blind", 2048, NULL, 3, NULL);
            }
        } 
        
        cJSON_Delete(json);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
httpd_uri_t after_calib_state_t = {
    .uri       = "/after-calib-state",
    .method    = HTTP_POST,
    .handler   = after_calib_state_handler,
    .user_ctx  = NULL
};

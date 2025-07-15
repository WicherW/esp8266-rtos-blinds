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
        // up calibration
        cJSON *move_up_big_blind_JSON = cJSON_GetObjectItem(json, "bigMoveUp");
        if(cJSON_IsNumber(move_up_big_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "big blind - move_up_big_blind_JSON: %i", move_up_big_blind_JSON->valueint);
            if(move_up_big_blind_JSON->valueint){
                blind_to_do_parameters_t *param_big_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_big_blind->blind_model = BIG_BLIND;
                param_big_blind->pind_blind = blinds_config.pins_blind_big;
                param_big_blind->direction = UP;
                param_big_blind->max_down_position = &big_blind_parameters.max_down_position;
                param_big_blind->current_steps_state = &big_blind_parameters.current_steps_state;
                param_big_blind->steps_to_do_calibration = move_up_big_blind_JSON->valueint;
                
                xTaskCreate(&calibration_blind, "calibrateRollblindUpBig", 2048, (void*)param_big_blind, 3, NULL);
            }
        }
        // down calibration
        cJSON *move_down_big_blind_JSON = cJSON_GetObjectItem(json, "bigMoveDown");
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

                xTaskCreate(&calibration_blind, "calibrateRollblindDownBig", 2048, (void*)param_big_blind, 3, NULL);
            }
           
        }

        // SMALL BLIND
        // up calibration
        cJSON *move_up_small_blind_JSON = cJSON_GetObjectItem(json, "smallMoveUp");
        if(cJSON_IsNumber(move_up_small_blind_JSON)){
            ESP_LOGI(TAG_SERVER, "move_up_small_blind_JSON: %i", move_up_small_blind_JSON->valueint);
            if(move_up_small_blind_JSON->valueint){
                blind_to_do_parameters_t *param_small_blind = (blind_to_do_parameters_t *)pvPortMalloc(sizeof(blind_to_do_parameters_t));
                param_small_blind->blind_model = SMALL_BLIND;
                param_small_blind->pind_blind = blinds_config.pins_blind_small;
                param_small_blind->direction = UP;
                param_small_blind->max_down_position = &small_blind_parameters.max_down_position;
                param_small_blind->current_steps_state = &small_blind_parameters.current_steps_state;
                param_small_blind->steps_to_do_calibration = move_up_small_blind_JSON->valueint;
                
                xTaskCreate(&calibration_blind, "calibrateRollblindUpSmall", 2048, (void*)param_small_blind, 3, NULL);
            }
        }
        // down calibration
        cJSON *move_down_small_blind_JSON = cJSON_GetObjectItem(json, "smallMoveDown");
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
    
    // Alokacja pamięci na dane POST
    char *total_data = (char *)pvPortMalloc(req->content_len + 1);
    if (total_data == NULL) {
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI("harmonogramposthandler", "length:%d ",req->content_len);
    
    int ret;
    int remaining = req->content_len;
    int received = 0;  // Śledzi liczbę odebranych danych

    ESP_LOGI("harmonogramposthandler", "przed whilem odbierania danych");

    while (remaining > 0) {
        // Odebranie części danych POST
        ret = httpd_req_recv(req, total_data + received, remaining);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;  // Timeout, spróbuj ponownie
            }
            free(total_data);
            return ESP_FAIL;  // Inny błąd
        }
        received += ret;
        remaining -= ret;
    }

    total_data[received] = '\0';

    ESP_LOGI("harmonogramposthandler", "Zawartosc total_data: %s", total_data);
    ESP_LOGI("harmonogramposthandler", "przed stworzeniem taska");
    

    if (xSemaphoreTake(scheduleSemaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI("harmonogramposthandler", "semafor zablokowany");
    } else {
        ESP_LOGI("harmonogramposthandler", "nie udalo sie zablokowac semafora");
        return ESP_FAIL;
    }

    cJSON *json = cJSON_Parse(total_data);

    if (json == NULL) {
        ESP_LOGE("harmonogramposthandler", "Nie udalo sie sparsowac JSON-a");
        xSemaphoreGive(scheduleSemaphore);
        vPortFree(total_data);
        return ESP_FAIL;
    }

    ESP_LOGI("harmonogramposthandler", "Free heap size: %d", esp_get_free_heap_size());
    
    ESP_LOGI("harmonogramposthandler", "po udanym parsowaniu JSON-a");

    cJSON *ponDown = cJSON_GetObjectItem(json, "ponTimeDown");
    cJSON *ponUp = cJSON_GetObjectItem(json, "ponTimeUp");

    cJSON *wtDown = cJSON_GetObjectItem(json, "wtTimeDown");
    cJSON *wtUp = cJSON_GetObjectItem(json, "wtTimeUp");

    cJSON *srDown = cJSON_GetObjectItem(json, "srTimeDown");
    cJSON *srUp = cJSON_GetObjectItem(json, "srTimeUp");

    cJSON *czwDown = cJSON_GetObjectItem(json, "czwTimeDown");
    cJSON *czwUp = cJSON_GetObjectItem(json, "czwTimeUp");

    cJSON *ptDown = cJSON_GetObjectItem(json, "ptTimeDown");
    cJSON *ptUp = cJSON_GetObjectItem(json, "ptTimeUp");

    cJSON *sbDown = cJSON_GetObjectItem(json, "sbTimeDown");
    cJSON *sbUp = cJSON_GetObjectItem(json, "sbTimeUp");

    cJSON *ndzDown = cJSON_GetObjectItem(json, "ndzTimeDown");
    cJSON *ndzUp = cJSON_GetObjectItem(json, "ndzTimeUp");

    ESP_LOGI("harmonogramposthandler", "po fali cJSONow getobjectitem");


    if (!ponDown || !ponUp || !wtDown || !wtUp || !srDown || !srUp ||
    !czwDown || !czwUp || !ptDown || !ptUp || !sbDown || !sbUp ||
    !ndzDown || !ndzUp) {
        ESP_LOGI("harmonogramposthandler", "Brak wymaganych pol w JSON");
    // Obsługa błędu
    }else{
        ESP_LOGI("harmonogramposthandler", "Wszystkie pola sa!");
    }

    ESP_LOGI("harmonogramposthandler", "po kontroli ifem");
    
    //Niedziela
    scheduleArray[0].day = 0;
    sscanf(cJSON_GetStringValue(ndzUp), "%d:%d", &scheduleArray[0].open_time.tm_hour, &scheduleArray[0].open_time.tm_min);
    sscanf(cJSON_GetStringValue(ndzDown), "%d:%d", &scheduleArray[0].close_time.tm_hour, &scheduleArray[0].close_time.tm_min);

    // Poniedziałek
    scheduleArray[1].day = 1;
    sscanf(cJSON_GetStringValue(ponUp), "%d:%d", &scheduleArray[1].open_time.tm_hour, &scheduleArray[1].open_time.tm_min);
    sscanf(cJSON_GetStringValue(ponDown), "%d:%d", &scheduleArray[1].close_time.tm_hour, &scheduleArray[1].close_time.tm_min);

    // Wtorek
    scheduleArray[2].day = 2;
    sscanf(cJSON_GetStringValue(wtUp), "%d:%d", &scheduleArray[2].open_time.tm_hour, &scheduleArray[2].open_time.tm_min);
    sscanf(cJSON_GetStringValue(wtDown), "%d:%d", &scheduleArray[2].close_time.tm_hour, &scheduleArray[2].close_time.tm_min);

    // Środa
    scheduleArray[3].day = 3;
    sscanf(cJSON_GetStringValue(srUp), "%d:%d", &scheduleArray[3].open_time.tm_hour, &scheduleArray[3].open_time.tm_min);
    sscanf(cJSON_GetStringValue(srDown), "%d:%d", &scheduleArray[3].close_time.tm_hour, &scheduleArray[3].close_time.tm_min);

    // Czwartek
    scheduleArray[4].day = 4;
    sscanf(cJSON_GetStringValue(czwUp), "%d:%d", &scheduleArray[4].open_time.tm_hour, &scheduleArray[4].open_time.tm_min);
    sscanf(cJSON_GetStringValue(czwDown), "%d:%d", &scheduleArray[4].close_time.tm_hour, &scheduleArray[4].close_time.tm_min);

    // Piątek
    scheduleArray[5].day = 5;
    sscanf(cJSON_GetStringValue(ptUp), "%d:%d", &scheduleArray[5].open_time.tm_hour, &scheduleArray[5].open_time.tm_min);
    sscanf(cJSON_GetStringValue(ptDown), "%d:%d", &scheduleArray[5].close_time.tm_hour, &scheduleArray[5].close_time.tm_min);

    // Sobota
    scheduleArray[6].day = 6;
    sscanf(cJSON_GetStringValue(sbUp), "%d:%d", &scheduleArray[6].open_time.tm_hour, &scheduleArray[6].open_time.tm_min);
    sscanf(cJSON_GetStringValue(sbDown), "%d:%d", &scheduleArray[6].close_time.tm_hour, &scheduleArray[6].close_time.tm_min);

    ESP_LOGI("harmonogramposthandler", "po uzupelnieniu struktury");


    // Zwalnianie pamięci
    cJSON_Delete(json);
    vPortFree(total_data);

    xSemaphoreGive(scheduleSemaphore);

    ESP_LOGI("harmonogramposthandler", "zwolnieniu semafora");

    // Tworzenie i uruchomienie timera
    if (scheduleTimer == NULL) {

        ESP_LOGI("harmonogramposthandler", "jest NULL, tworzenie timera!");

        scheduleTimer = xTimerCreate("MyTimer", pdMS_TO_TICKS(60000), pdTRUE, (void *)0, schedule_timer_callback);

        ESP_LOGI("harmonogramposthandler", "timer stworzony");

        if (scheduleTimer != NULL) {

            ESP_LOGI("harmonogramposthandler", "timer nie jest rowny null");

            if (xTimerStart(scheduleTimer, 0) != pdPASS) {
                ESP_LOGI("harmonogramposthandler", "Nie udalo sie uruchomic timera.");
            }else{
                ESP_LOGI("harmonogramposthandler", "timer zostal odpalony");
            }
        } else {
            ESP_LOGI("harmonogramposthandler", "Nie udalo sie stworzyc timera.");
        }
    } else{
        ESP_LOGI("harmonogramposthandler", "timer jest juz stworzony, zostala tylko zaaktualizowana struktura z godzinami");
    }
    
    // Odpowiedź HTTP (może być zależna od implementacji
    httpd_resp_send_chunk(req, NULL, 0);  // Zakończenie odpowiedzi

    ESP_LOGI("harmonogramposthandler", "przed return OK");
    return ESP_OK;
}
httpd_uri_t schedule_t = {
    .uri       = "/schedule",
    .method    = HTTP_POST,
    .handler   = schedule_post_handler,
    .user_ctx  = NULL
};

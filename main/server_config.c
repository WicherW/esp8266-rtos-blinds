#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_server.h"

#include "project_config.h"
#include "http_handlers.h"

# define HANDLERS_COUNT 10

httpd_handle_t start_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = HANDLERS_COUNT;

    ESP_LOGI(TAG_SERVER, "Starting server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {
        
        ESP_LOGI(TAG_SERVER, "Registering URI handlers");

        esp_err_t err;

        err = httpd_register_uri_handler(server, &main_page_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "main_page_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &calibration_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "calibration_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &sliders_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "sliders_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &confirm_full_up_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "confirm_full_up_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &move_blind_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "move_blind_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &after_calib_state_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "after_calib_state_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }      

        err = httpd_register_uri_handler(server, &big_dev_data_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "big_dev_data_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &small_dev_data_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "small_dev_data_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &fill_inputs_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "fill_inputs_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        err = httpd_register_uri_handler(server, &schedule_t);
        if (err != ESP_OK) {
            ESP_LOGE(TAG_SERVER, "schedule_t registration failed: %s (0x%x)", esp_err_to_name(err), err);
        }

        return server;
    }

    ESP_LOGI(TAG_SERVER, "Error starting server!");
    return NULL;
}

void stop_server(httpd_handle_t server) {
    httpd_stop(server);
}

void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG_SERVER, "Stopping webserver");
        stop_server(*server);
        *server = NULL;
    }
}

void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG_SERVER, "Starting webserver");
        *server = start_server();
    }
}
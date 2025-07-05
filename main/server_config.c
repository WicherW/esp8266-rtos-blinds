#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_server.h"

#include "project_config.h"
#include "http_handlers.h"


httpd_handle_t start_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG_SERVER, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        
        ESP_LOGI(TAG_SERVER, "Registering URI handlers");
        httpd_register_uri_handler(server, &main_page_t);
        httpd_register_uri_handler(server, &parameters_t);
        httpd_register_uri_handler(server, &schedule_t); //!! refactor this
        httpd_register_uri_handler(server, &small_dev_data_t);
        httpd_register_uri_handler(server, &big_dev_data_t);
        httpd_register_uri_handler(server, &fill_inputs_t);
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
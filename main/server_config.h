#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "esp_http_server.h"

/**
 * @brief Starts the HTTP server and registers handlers.
 * 
 * @return Handle to the server.
 * @note The server is started on port 80.
 */
httpd_handle_t start_server(void);


/**
 * @brief Stops the HTTP server.
 * 
 * @param server Handle to the server.
 */
void stop_server(httpd_handle_t server);


/**
 * @brief Handles WiFi connection. Creates the server if it does not exist.
 * 
 * @param arg Handle to the server.
 * @param event_base Type of the event.
 * @param event_id ID of the event.
 * @param event_data Event data.
 */
void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


/**
 * @brief Handles WiFi disconnection. Stops the server if it exists.
 * 
 * @param arg Handle to the server.
 * @param event_base Type of the event.
 * @param event_id ID of the event.
 * @param event_data Event data.
 */
void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


#endif
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "esp_event.h"
#include <stdint.h>


/**
 * @brief Handler for WiFi events. Taken from the ESP-IDF example.
 * @param arg The argument passed to the handler.
 * @param event_base The type of the event.
 * @param event_id The event ID.
 * @param event_data Pointer to event data.
 */
void event_handler_wifi(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


/**
 * @brief Initializes the WiFi connection.
 * All taken from the ESP-IDF example.
 */
void init_wifi(void);

#endif
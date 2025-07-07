#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H

#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"


/**
 * @brief Initializes NVS (Non-Volatile Storage)
 */
void init_nvs();


/**
 * @brief Writes the integer value to NVS memory
 * 
 * @param key The key under which the value will be saved
 * @param value The value to write
 */
void save_int_to_nvs(const char *key, int32_t value);


/**
 * @brief Reads an integer value from NVS memory
 * 
 * @param key The key under which the value was stored
 * @return Value read from NVS memory
 */
int32_t read_int_from_nvs(const char *key);


#endif


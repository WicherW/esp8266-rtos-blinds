#ifndef SPIFFS_CONFIG_H
#define SPIFFS_CONFIG_H

#include "esp_err.h"
#include <string.h>


/**
 * @brief Initializes the SPIFFS file system.
 * Needed to read html files from the board's memory.
 */
void init_spiffs();

/**
 * @brief Reads a file from SPIFFS.
 * @param file_path The path to the file to read.
 * @param buffer Pointer to the buffer where the read file will be saved.
 * @param length Pointer to a variable in which the size of the read file will be saved.
 * @return ESP_OK if the operation was successful, otherwise ESP_FAIL.
 */
esp_err_t read_file(const char *file_path, char **buffer, size_t *length);

#endif
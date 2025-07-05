#ifndef SNTP_CONFIG_H
#define SNTP_CONFIG_H

#include "time.h"


/**
 * @brief Initializes SNTP (Simple Network Time Protocol)
 */
void sntp_initialize();


/**
 * @brief Reads the current date and time from SNTP
 * 
 * @return A "tm" structure containing the current date and time
 */
struct tm get_date_and_time();

#endif
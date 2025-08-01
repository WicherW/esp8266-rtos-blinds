#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include <time.h>


typedef struct {
    int day;
    struct tm open_time;
    struct tm close_time;
} schedule;

extern schedule scheduleArray[7];

extern SemaphoreHandle_t scheduleSemaphore;
extern TimerHandle_t scheduleTimer;

/**
 * @brief Function to check the schedule and start the stepper motor. Checks every minute whether the blinds should be rolled up or unrolled.
 * 
 * @param xTimer pointer to the timer handle
 */
void schedule_timer_callback(TimerHandle_t xTimer);

void parse_schedule_json_from_client(void *pvParameters);

void print_schedule_array();

#endif
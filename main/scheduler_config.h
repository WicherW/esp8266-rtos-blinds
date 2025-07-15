#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "time.h"

typedef struct {
    int day;
    struct tm open_time;
    struct tm close_time;
} Schedule;

extern Schedule scheduleArray[7];

extern SemaphoreHandle_t scheduleSemaphore;
extern TimerHandle_t scheduleTimer;


void UpdateScheduleAndSetTimer(void* pvParameters);

/**
 * @brief Function to check the schedule and start the stepper motor. Checks every minute whether the blinds should be rolled up or unrolled.
 * 
 * @param xTimer pointer to the timer handle
 */
void schedule_timer_callback(TimerHandle_t xTimer);

#endif
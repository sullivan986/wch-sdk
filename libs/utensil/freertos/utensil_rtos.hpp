#ifndef UTENSIL_FREERTOS_HPP
#define UTENSIL_FREERTOS_HPP

#include "FreeRTOS.h"
#include "task.h"
// control
static inline void task_sleep_ms(TickType_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

#endif
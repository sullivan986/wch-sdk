#ifndef UTENSIL_HPP
#define UTENSIL_HPP

#include "FreeRTOS.h"
#include "debug.h"
#include "task.h"
#include <stdio.h>

#define LOG_COLOR_RESET "\x1b[0m"

#define log_debug(fmt, ...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("[%d] [%s] %s(): " fmt "\r\n", xTaskGetTickCount() * 2, "DEBUG", __func__, ##__VA_ARGS__);              \
    } while (0)

#define log_info(fmt, ...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("[%d] [%s] %s() [%s:%d]: " fmt "\r\n", xTaskGetTickCount() * 2, "\x1b[32mINFO" LOG_COLOR_RESET,         \
               __func__, __FILE__, __LINE__, ##__VA_ARGS__);                                                           \
    } while (0)

#define log_warinig(fmt, ...)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("[%d] [%s] %s() [%s:%d]: " fmt "\r\n", xTaskGetTickCount() * 2, "\x1b[33mWARNING" LOG_COLOR_RESET,      \
               __func__, __FILE__, __LINE__, ##__VA_ARGS__);                                                           \
    } while (0)

#define log_error(fmt, ...)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("[%d] [%s] %s() [%s:%d]: " fmt "\r\n", xTaskGetTickCount() * 2, "\x1b[31mERROR" LOG_COLOR_RESET,        \
               __func__, __FILE__, __LINE__, ##__VA_ARGS__);                                                           \
    } while (0)

// control
static inline void task_sleep_ms(TickType_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

#endif
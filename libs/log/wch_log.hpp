#ifndef WCH_LOG_HPP
#define WCH_LOG_HPP

#include <stdio.h>

typedef enum
{
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
} LogLevel;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#define LOG_COLOR_RED "\x1b[31m"
#define LOG_COLOR_YELLOW "\x1b[33m"
#define LOG_COLOR_GREEN "\x1b[32m"
#define LOG_COLOR_RESET "\x1b[0m"

const char *log_level_strings[] = {LOG_COLOR_RED "ERROR" LOG_COLOR_RESET, LOG_COLOR_YELLOW "WARNING" LOG_COLOR_RESET,
                                   LOG_COLOR_GREEN "INFO" LOG_COLOR_RESET, "DEBUG"};

class log
{
  public:
    static void error()
    {
        printf("s");
    }

    static void info()
    {
    }
};

#define log_message(log_level, fmt, ...)                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        int64_t current_time = esp_timer_get_time() / 1000;                                                            \
        printf("[%lld] [%s] %s() [%s:%d]: " fmt "\n", current_time, log_level_strings[log_level], __func__, __FILE__,  \
               __LINE__, ##__VA_ARGS__);                                                                               \
    } while (0)

#endif
#ifndef CH592_BASE_H
#define CH592_BASE_H

#include "CH59x_common.h"

#ifndef BLE_LOGER
#define BLE_LOGER
#define log_info(fmt, ...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        PRINT("[%d]" fmt "\r\n", TMOS_GetSystemClock() * 5 / 8, ##__VA_ARGS__);                                        \
    } while (0)
#endif

#endif
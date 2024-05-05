#include "FreeRTOS.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_opa.h"
#include "coroutine"
#include "debug.h"
#include "main_functions.h"
#include "task.h"
#include "utensil.hpp"
#include <algorithm>
#include <vector>

extern "C" int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    SDI_Printf_Enable();
    log_debug("SystemClk:%d", SystemCoreClock);
    log_debug("ChipID:%08x", DBGMCU_GetCHIPID());
    log_debug("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

    setup();
    /* Note: Modified from original while(true) to accommodate CI */
    for (int i = 0; i < 10; i++)
    {
        loop();
    }
    return 0;
}

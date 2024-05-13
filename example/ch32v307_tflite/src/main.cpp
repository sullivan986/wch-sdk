#include "FreeRTOS.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_opa.h"
#include "coroutine"
#include "debug.h"
#include "portable.h"
#include "task.h"
#include "tflite_use.hpp"
#include "utensil.hpp"
#include <algorithm>
#include <cstdio>
#include <model.hpp>
#include <vector>

TaskHandle_t process_task_Handler;

void process_task(void *p)
{
    tflite_use<tu_AddFullyConnected> tu(g_model, 2 * 1024);
    for (int i = 0; i < 20; i++)
    {
        tu.run();
    }
}

extern "C" int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    SDI_Printf_Enable();
    log_debug("SystemClk:%d", SystemCoreClock);
    log_debug("ChipID:%08x", DBGMCU_GetCHIPID());
    log_debug("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

    xTaskCreate(process_task, "process_task", 4096, nullptr, 5, &process_task_Handler);
    vTaskStartScheduler();
    while (true)
    {
        log_error("in end");
        task_sleep_ms(1000);
    }
    return 0;
}

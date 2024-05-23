#include "FreeRTOS.h"
#include "debug.h"
#include "task.h"
#include "utensil.hpp"

TaskHandle_t Task1Task_Handler;
TaskHandle_t Task2Task_Handler;
void task1(void *pvParameters)
{
    while (1)
    {
        log_debug("task1!");
        task_sleep_ms(1000);
    }
}

void task2(void *pvParameters)
{
    while (1)
    {
        log_debug("task2!");
        task_sleep_ms(1000);
    }
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    SDI_Printf_Enable();
    log_debug("SystemClk:%d", SystemCoreClock);
    log_debug("ChipID:%08x", DBGMCU_GetCHIPID());
    log_debug("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

    /* create two task */
    xTaskCreate(task1, "task1", 256, nullptr, 5, &Task1Task_Handler);
    xTaskCreate(task2, "task2", 256, nullptr, 5, &Task2Task_Handler);
    vTaskStartScheduler();
    while (1)
    {
        log_error("in end");
        task_sleep_ms(1000);
    }
}

#include "FreeRTOS.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_opa.h"
#include "debug.h"
#include "task.h"
#include "utensil.hpp"

TaskHandle_t Task1Task_Handler;
TaskHandle_t Task2Task_Handler;
static bool btn_bit = false;

void task1(void *pvParameters)
{
    while (1)
    {
        log_debug("task1!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task2(void *pvParameters)
{
    while (1)
    {
        log_debug("task2!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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
        Delay_Us(1000);
    }
}

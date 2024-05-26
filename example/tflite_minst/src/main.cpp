#include "FreeRTOS.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_opa.h"
#include "coroutine"
#include "debug.h"
#include "portable.h"
#include "task.h"
#include "test_data.hpp"
#include "utensil.hpp"
#include <algorithm>
#include <cstdio>
#include <model.hpp>
#include <vector>

TaskHandle_t process_task_Handler;

void process_task(void *p)
{
    tflite_use<tu_AddFullyConnected, tu_AddStridedSlice, tu_AddConv2D, tu_AddMaxPool2D, tu_AddPack, tu_AddShape,
               tu_AddReshape, tu_AddSoftmax, tu_AddQuantize, tu_AddDequantize>
        tu(model_tflite, 14 * 1024);

    // auto input_data = tu.get_input_data(2); // if you want multiple inputs and outputs
    // auto data_p = &test_data[0][0];
    // for (int i = 0; i < 28 * 28; i++)
    //{
    //     input_data.f[i] = data_p[i];
    // }
    // tu.run();
    // auto output_data = tu.get_output_data(2);

    auto output = tu.run([](TfLiteTensor *input) {
        auto data_p = &test_data[0][0];
        for (int i = 0; i < 28 * 28; i++)
        {
            input->data.f[i] = data_p[i];
        }
    });

    for (int i = 0; i < 10; i++)
    {
        log_info("p_value %d: %f", i, output->data.f[i]);
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

    xTaskCreate(process_task, "process_task", 1024, nullptr, 5, &process_task_Handler);
    // xTaskCreate(test_task, "test_task", 256, nullptr, 6, &test_task_Handler);
    vTaskStartScheduler();
    while (true)
    {
        log_error("in end");
        task_sleep_ms(1000);
    }
    return 0;
}

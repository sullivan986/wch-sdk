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

extern "C" void process_task(void *p)
{
    auto kXrange = 2.f * 3.14159265359f;
    auto kInferencesPerCycle = 20.f;

    tflite_use<tu_AddFullyConnected> tu(g_model, 2 * 1000);

    for (int i = 0; i < 20; i++)
    {
        // Calculate an x value to feed into the model. We compare the current
        // inference_count to the number of inferences per cycle to determine
        // our position within the range of possible x values the model was
        // trained on, and use this to calculate a value.
        float position = static_cast<float>(i) / kInferencesPerCycle;
        float x = position * kXrange;

        auto output = tu.run([x](TfLiteTensor *input) {
            // Quantize the input from floating-point to integer
            int8_t x_quantized = x / input->params.scale + input->params.zero_point;
            // Place the quantized input in the model's input tensor
            input->data.int8[0] = x_quantized;
        });

        // Obtain the quantized output from model's output tensor
        auto y_quantized = output->data.int8[0];

        // Dequantize the output from integer to floating-point
        float y = (y_quantized - output->params.zero_point) * output->params.scale;

        // Output the results. A custom HandleOutput function can be implemented
        // for each supported hardware target.
        log_info("x_value: %f, y_value: %f", x, y);
    }
    while (true)
    {
        log_debug("entry process task");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

extern "C" int main()
{
    ch_init();
    log_debug("SystemClk:%d", SystemCoreClock);
    log_debug("ChipID:%08x", DBGMCU_GetCHIPID());
    log_debug("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

    xTaskCreate(process_task, "process_task", 512, nullptr, 5, &process_task_Handler);
    vTaskStartScheduler();
    while (true)
    {
        log_error("in end");
        task_sleep_ms(1000);
    }
    return 0;
}

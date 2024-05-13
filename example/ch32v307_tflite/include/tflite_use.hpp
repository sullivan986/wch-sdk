#ifndef TFLITE_USE_HPP
#define TFLITE_USE_HPP

#include "utensil.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_log.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <vector>

const float kXrange = 2.f * 3.14159265359f;
const int kInferencesPerCycle = 20;

// 运算符枚举
enum tu_Resolver_Operator
{
    tu_AddAbs,
    tu_AddAdd,
    tu_AddFullyConnected
};

template <tu_Resolver_Operator... Resolver_Operators> class tflite_use
{
  public:
    // 初始化
    tflite_use(const unsigned char *model_data, size_t buf_size)
        : model(tflite::GetModel(model_data)), tensor_arena(buf_size)
    {
        // 加载模型并 判断tflite模型版本是否为V3
        if (model->version() != TFLITE_SCHEMA_VERSION)
        {
            log_warinig("Model provided is schema version %d not equal "
                        "to supported version %d.",
                        model->version(), TFLITE_SCHEMA_VERSION);
            return;
        }

        // 添加需要的运算符
        (resolver_add_operator(Resolver_Operators), ...);

        // 初始化一个解释器对象
        interpreter =
            std::make_unique<tflite::MicroInterpreter>(model, resolver, tensor_arena.data(), tensor_arena.size());

        // 为模型的张量分配内存
        if (status = interpreter->AllocateTensors(), status != kTfLiteOk)
        {
            log_warinig("AllocateTensors error: %d", status);
            return;
        }

        // 获取指向模型的输入和输出张量的指针
        input = interpreter->input(0);
        output = interpreter->output(0);
    }

    void run()
    {
        // Calculate an x value to feed into the model. We compare the current
        // inference_count to the number of inferences per cycle to determine
        // our position within the range of possible x values the model was
        // trained on, and use this to calculate a value.
        float position = static_cast<float>(inference_count) / static_cast<float>(kInferencesPerCycle);
        float x = position * kXrange;

        // Quantize the input from floating-point to integer
        int8_t x_quantized = x / input->params.scale + input->params.zero_point;
        // Place the quantized input in the model's input tensor
        input->data.int8[0] = x_quantized;

        interpreter_calculate();

        // Obtain the quantized output from model's output tensor
        int8_t y_quantized = output->data.int8[0];
        // Dequantize the output from integer to floating-point
        float y = (y_quantized - output->params.zero_point) * output->params.scale;

        // Output the results. A custom HandleOutput function can be implemented
        // for each supported hardware target.
        log_debug("x_value: %f, y_value: %f", static_cast<float>(x), static_cast<float>(y));

        // Increment the inference_counter, and reset it if we have reached
        // the total number per cycle
        inference_count += 1;
        if (inference_count >= kInferencesPerCycle)
            inference_count = 0;
    }

  private:
    // 计算器
    tflite::MicroMutableOpResolver<sizeof...(Resolver_Operators)> resolver;
    // 解释器及其指针
    // tflite::MicroInterpreter static_interpreter;
    std::unique_ptr<tflite::MicroInterpreter> interpreter;
    // 模型指针
    const tflite::Model *model;
    // 返回状态
    TfLiteStatus status;
    // buff
    // uint8_t tensor_arena[2000];
    std::vector<uint8_t> tensor_arena;
    // 计算次数计数
    int inference_count = 0;
    // 输入张量指针
    TfLiteTensor *input;
    // 输出张量指针
    TfLiteTensor *output;

    constexpr void resolver_add_operator(tu_Resolver_Operator resolver_operator)
    {
        switch (resolver_operator)
        {
        case tu_AddAbs:
            status = resolver.AddAbs();
            break;
        case tu_AddAdd:
            status = resolver.AddAdd();
            break;
        case tu_AddFullyConnected:
            status = resolver.AddFullyConnected();
            break;
        default:
            log_warinig("add operator not find");
        }
        if (status != kTfLiteOk)
        {
            log_warinig("add operator faild: %d", status);
        }
    }

    void interpreter_calculate()
    {
        // Run inference, and report any error
        if (status = interpreter->Invoke(), status != kTfLiteOk)
        {
            log_warinig("Invoke failed on status: %d", status);
            return;
        }
    }
};

#endif
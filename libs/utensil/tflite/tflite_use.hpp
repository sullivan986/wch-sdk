#ifndef TFLITE_USE_HPP
#define TFLITE_USE_HPP

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "utensil.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_log.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <vector>

// 运算符枚举
enum tu_Resolver_Operator
{
    tu_AddAbs,
    tu_AddAdd,
    tu_AddAddN,
    tu_AddArgMax,
    tu_AddArgMin,
    tu_AddAssignVariable,
    tu_AddAveragePool2D,
    tu_AddBatchMatMul,
    tu_AddBatchToSpaceNd,
    tu_AddBroadcastArgs,
    tu_AddBroadcastTo,
    tu_AddCallOnce,
    tu_AddCast,
    tu_AddCeil,
    tu_AddCircularBuffer,
    tu_AddConcatenation,
    tu_AddConv2D,
    tu_AddCos,
    tu_AddCumSum,
    tu_AddDelay,
    tu_AddDepthToSpace,
    tu_AddDepthwiseConv2D,
    tu_AddDequantize,
    tu_AddDetectionPostprocess,
    tu_AddDiv,
    tu_AddEmbeddingLookup,
    tu_AddEnergy,
    tu_AddElu,
    tu_AddEqual,
    tu_AddEthosU,
    tu_AddExp,
    tu_AddExpandDims,
    tu_AddFftAutoScale,
    tu_AddFill,
    tu_AddFilterBank,
    tu_AddFilterBankLog,
    tu_AddFilterBankSquareRoot,
    tu_AddFilterBankSpectralSubtraction,
    tu_AddFloor,
    tu_AddFloorDiv,
    tu_AddFloorMod,
    tu_AddFramer,
    tu_AddFullyConnected,
    tu_AddGather,
    tu_AddGatherNd,
    tu_AddGreater,
    tu_AddGreaterEqual,
    tu_AddHardSwish,
    tu_AddIf,
    tu_AddIrfft,
    tu_AddL2Normalization,
    tu_AddL2Pool2D,
    tu_AddLeakyRelu,
    tu_AddLess,
    tu_AddLessEqual,
    tu_AddLog,
    tu_AddLogicalAnd,
    tu_AddLogicalNot,
    tu_AddLogicalOr,
    tu_AddLogistic,
    tu_AddLogSoftmax,
    tu_AddMaximum,
    tu_AddMaxPool2D,
    tu_AddMirrorPad,
    tu_AddMean,
    tu_AddMinimum,
    tu_AddMul,
    tu_AddNeg,
    tu_AddNotEqual,
    tu_AddOverlapAdd,
    tu_AddPack,
    tu_AddPad,
    tu_AddPadV2,
    tu_AddPCAN,
    tu_AddPrelu,
    tu_AddQuantize,
    tu_AddReadVariable,
    tu_AddReduceMax,
    tu_AddRelu,
    tu_AddRelu6,
    tu_AddReshape,
    tu_AddResizeBilinear,
    tu_AddResizeNearestNeighbor,
    tu_AddRfft,
    tu_AddRound,
    tu_AddRsqrt,
    tu_AddSelectV2,
    tu_AddShape,
    tu_AddSin,
    tu_AddSlice,
    tu_AddSoftmax,
    tu_AddSpaceToBatchNd,
    tu_AddSpaceToDepth,
    tu_AddSplit,
    tu_AddSplitV,
    tu_AddSqueeze,
    tu_AddSqrt,
    tu_AddSquare,
    tu_AddSquaredDifference,
    tu_AddStridedSlice,
    tu_AddStacker,
    tu_AddSub,
    tu_AddSum,
    tu_AddSvdf,
    tu_AddTanh,
    tu_AddTransposeConv,
    tu_AddTranspose,
    tu_AddUnpack,
    tu_AddUnidirectionalSequenceLSTM,
    tu_AddVarHandle,
    tu_AddWhile,
    tu_AddWindow,
    tu_AddZerosLike
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
        interpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena.data(), tensor_arena.size());

        // 为模型的张量分配内存
        if (status = interpreter->AllocateTensors(), status != kTfLiteOk)
        {
            log_warinig("AllocateTensors error: %d", status);
            return;
        }
    }

    // 0 index input and out put
    TfLitePtrUnion &run(std::function<void(TfLitePtrUnion)> func)
    {
        func(get_output_data(0));
        interpreter_calculate();
        return get_input_data(0);
    }

    // for more parameterization
    TfLiteTensor *run(std::function<void(TfLiteTensor *)> func)
    {
        func(interpreter->input(0));
        interpreter_calculate();
        return interpreter->output(0);
    }

    void run()
    {
        interpreter_calculate();
    }

    TfLitePtrUnion &get_input_data(size_t index)
    {
        return interpreter->input(index)->data;
    }

    TfLitePtrUnion &get_output_data(size_t index)
    {
        return interpreter->output(index)->data;
    }

  private:
    // 计算器
    tflite::MicroMutableOpResolver<sizeof...(Resolver_Operators)> resolver;
    // 解释器及其指针
    // tflite::MicroInterpreter static_interpreter;
    tflite::MicroInterpreter *interpreter;
    // 模型指针
    const tflite::Model *model;
    // 返回状态
    TfLiteStatus status;
    // buff
    std::vector<uint8_t> tensor_arena;

    constexpr void resolver_add_operator(tu_Resolver_Operator resolver_operator)
    {
        switch (resolver_operator)
        {
        case tu_AddAdd:
            status = resolver.AddAdd();
            break;
        case tu_AddAddN:
            status = resolver.AddAddN();
            break;
        case tu_AddArgMax:
            status = resolver.AddArgMax();
            break;
        case tu_AddArgMin:
            status = resolver.AddArgMin();
            break;
        case tu_AddAssignVariable:
            status = resolver.AddAssignVariable();
            break;
        case tu_AddAveragePool2D:
            status = resolver.AddAveragePool2D();
            break;
        case tu_AddBatchMatMul:
            status = resolver.AddBatchMatMul();
            break;
        case tu_AddBatchToSpaceNd:
            status = resolver.AddBatchToSpaceNd();
            break;
        case tu_AddBroadcastArgs:
            status = resolver.AddBroadcastArgs();
            break;
        case tu_AddBroadcastTo:
            status = resolver.AddBroadcastTo();
            break;
        case tu_AddCallOnce:
            status = resolver.AddCallOnce();
            break;
        case tu_AddCast:
            status = resolver.AddCast();
            break;
        case tu_AddCeil:
            status = resolver.AddCeil();
            break;
        case tu_AddCircularBuffer:
            status = resolver.AddCircularBuffer();
            break;
        case tu_AddConcatenation:
            status = resolver.AddConcatenation();
            break;
        case tu_AddConv2D:
            status = resolver.AddConv2D();
            break;
        case tu_AddCos:
            status = resolver.AddCos();
            break;
        case tu_AddCumSum:
            status = resolver.AddCumSum();
            break;
        case tu_AddDelay:
            // status = resolver.AddDelay();
            break;
        case tu_AddDepthToSpace:
            status = resolver.AddDepthToSpace();
            break;
        case tu_AddDepthwiseConv2D:
            status = resolver.AddDepthwiseConv2D();
            break;
        case tu_AddDequantize:
            status = resolver.AddDequantize();
            break;
        case tu_AddDetectionPostprocess:
            status = resolver.AddDetectionPostprocess();
            break;
        case tu_AddDiv:
            status = resolver.AddDiv();
            break;
        case tu_AddEmbeddingLookup:
            status = resolver.AddEmbeddingLookup();
            break;
        case tu_AddEnergy:
            // status = resolver.AddEnergy();
            break;
        case tu_AddElu:
            status = resolver.AddElu();
            break;
        case tu_AddEqual:
            status = resolver.AddEqual();
            break;
        case tu_AddEthosU:
            status = resolver.AddEthosU();
            break;
        case tu_AddExp:
            status = resolver.AddExp();
            break;
        case tu_AddExpandDims:
            status = resolver.AddExpandDims();
            break;
        case tu_AddFftAutoScale:
            // status = resolver.AddFftAutoScale();
            break;
        case tu_AddFill:
            status = resolver.AddFill();
            break;
        case tu_AddFilterBank:
            // status = resolver.AddFilterBank();
            break;
        case tu_AddFilterBankLog:
            // status = resolver.AddFilterBankLog();
            break;
        case tu_AddFilterBankSquareRoot:
            // status = resolver.AddFilterBankSquareRoot();
            break;
        case tu_AddFilterBankSpectralSubtraction:
            // status = resolver.AddFilterBankSpectralSubtraction();
            break;
        case tu_AddFloor:
            status = resolver.AddFloor();
            break;
        case tu_AddFloorDiv:
            status = resolver.AddFloorDiv();
            break;
        case tu_AddFloorMod:
            status = resolver.AddFloorMod();
            break;
        case tu_AddFramer:
            // status = resolver.AddFramer();
            break;
        case tu_AddFullyConnected:
            status = resolver.AddFullyConnected();
            break;
        case tu_AddGather:
            status = resolver.AddGather();
            break;
        case tu_AddGatherNd:
            status = resolver.AddGatherNd();
            break;
        case tu_AddGreater:
            status = resolver.AddGreater();
            break;
        case tu_AddGreaterEqual:
            status = resolver.AddGreaterEqual();
            break;
        case tu_AddHardSwish:
            status = resolver.AddHardSwish();
            break;
        case tu_AddIf:
            status = resolver.AddIf();
            break;
        case tu_AddIrfft:
            // status = resolver.AddIrfft();
            break;
        case tu_AddL2Normalization:
            status = resolver.AddL2Normalization();
            break;
        case tu_AddL2Pool2D:
            status = resolver.AddL2Pool2D();
            break;
        case tu_AddLeakyRelu:
            status = resolver.AddLeakyRelu();
            break;
        case tu_AddLess:
            status = resolver.AddLess();
            break;
        case tu_AddLessEqual:
            status = resolver.AddLessEqual();
            break;
        case tu_AddLog:
            status = resolver.AddLog();
            break;
        case tu_AddLogicalAnd:
            status = resolver.AddLogicalAnd();
            break;
        case tu_AddLogicalNot:
            status = resolver.AddLogicalNot();
            break;
        case tu_AddLogicalOr:
            status = resolver.AddLogicalOr();
            break;
        case tu_AddLogistic:
            status = resolver.AddLogistic();
            break;
        case tu_AddLogSoftmax:
            status = resolver.AddLogSoftmax();
            break;
        case tu_AddMaximum:
            status = resolver.AddMaximum();
            break;
        case tu_AddMaxPool2D:
            status = resolver.AddMaxPool2D();
            break;
        case tu_AddMirrorPad:
            status = resolver.AddMirrorPad();
            break;
        case tu_AddMean:
            status = resolver.AddMean();
            break;
        case tu_AddMinimum:
            status = resolver.AddMinimum();
            break;
        case tu_AddMul:
            status = resolver.AddMul();
            break;
        case tu_AddNeg:
            status = resolver.AddNeg();
            break;
        case tu_AddNotEqual:
            status = resolver.AddNotEqual();
            break;
        case tu_AddOverlapAdd:
            // status = resolver.AddOverlapAdd();
            break;
        case tu_AddPack:
            status = resolver.AddPack();
            break;
        case tu_AddPad:
            status = resolver.AddPad();
            break;
        case tu_AddPadV2:
            status = resolver.AddPadV2();
            break;
        case tu_AddPCAN:
            // status = resolver.AddPCAN();
            break;
        case tu_AddPrelu:
            status = resolver.AddPrelu();
            break;
        case tu_AddQuantize:
            status = resolver.AddQuantize();
            break;
        case tu_AddReadVariable:
            status = resolver.AddReadVariable();
            break;
        case tu_AddReduceMax:
            status = resolver.AddReduceMax();
            break;
        case tu_AddRelu:
            status = resolver.AddRelu();
            break;
        case tu_AddRelu6:
            status = resolver.AddRelu6();
            break;
        case tu_AddReshape:
            status = resolver.AddReshape();
            break;
        case tu_AddResizeBilinear:
            status = resolver.AddResizeBilinear();
            break;
        case tu_AddResizeNearestNeighbor:
            status = resolver.AddResizeNearestNeighbor();
            break;
        case tu_AddRfft:
            //  status = resolver.AddRfft();
            break;
        case tu_AddRound:
            status = resolver.AddRound();
            break;
        case tu_AddRsqrt:
            status = resolver.AddRsqrt();
            break;
        case tu_AddSelectV2:
            status = resolver.AddSelectV2();
            break;
        case tu_AddShape:
            status = resolver.AddShape();
            break;
        case tu_AddSin:
            status = resolver.AddSin();
            break;
        case tu_AddSlice:
            status = resolver.AddSlice();
            break;
        case tu_AddSoftmax:
            status = resolver.AddSoftmax();
            break;
        case tu_AddSpaceToBatchNd:
            status = resolver.AddSpaceToBatchNd();
            break;
        case tu_AddSpaceToDepth:
            status = resolver.AddSpaceToDepth();
            break;
        case tu_AddSplit:
            status = resolver.AddSplit();
            break;
        case tu_AddSplitV:
            status = resolver.AddSplitV();
            break;
        case tu_AddSqueeze:
            status = resolver.AddSqueeze();
            break;
        case tu_AddSqrt:
            status = resolver.AddSqrt();
            break;
        case tu_AddSquare:
            status = resolver.AddSquare();
            break;
        case tu_AddSquaredDifference:
            status = resolver.AddSquaredDifference();
            break;
        case tu_AddStridedSlice:
            status = resolver.AddStridedSlice();
            break;
        case tu_AddStacker:
            //  status = resolver.AddStacker();
            break;
        case tu_AddSub:
            status = resolver.AddSub();
            break;
        case tu_AddSum:
            status = resolver.AddSum();
            break;
        case tu_AddSvdf:
            status = resolver.AddSvdf();
            break;
        case tu_AddTanh:
            status = resolver.AddTanh();
            break;
        case tu_AddTransposeConv:
            status = resolver.AddTransposeConv();
            break;
        case tu_AddTranspose:
            status = resolver.AddTranspose();
            break;
        case tu_AddUnpack:
            status = resolver.AddUnpack();
            break;
        case tu_AddUnidirectionalSequenceLSTM:
            status = resolver.AddUnidirectionalSequenceLSTM();
            break;
        case tu_AddVarHandle:
            status = resolver.AddVarHandle();
            break;
        case tu_AddWhile:
            status = resolver.AddWhile();
            break;
        case tu_AddWindow:
            // status = resolver.AddWindow();
            break;
        case tu_AddZerosLike:
            status = resolver.AddZerosLike();
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
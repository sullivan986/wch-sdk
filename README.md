# wch-sdk
CMake SDK port for Qinheng Universal MCU.

## example

### tflite hello_world
Due to compiler issues, if you want to use tflite, you shoule remove `-fsingle-precision-constant` from c++ flag, But if you do that, you won't be able to use float unit acceleration in most libraries, and here are a few ways around that:
1. Remove `-fsingle-precision-constant` flag, this method is used in the example.
2. you should change line 192 in the `tensorflow/lite/micro/kernels/lstm_eval_common.cc`.
```
cell_state_info.quantized_cell_clip = static_cast<int16_t>(
    std::min(std::max(static_cast<double>(cell_clip) /
                          static_cast<double>(cell_state_scale),
                      -32768.0),
             32767.0));
```
Replace it with the following
```
cell_state_info.quantized_cell_clip = static_cast<int16_t>(
    std::min(std::max(static_cast<double>(cell_clip) /
                          static_cast<double>(cell_state_scale),
                      (double)-32768.0),
             (double)32767.0));
```
or
```
cell_state_info.quantized_cell_clip = static_cast<int16_t>(
    std::min(std::max(static_cast<float>(cell_clip) /
                          static_cast<float>(cell_state_scale),
                      -32768.0),
             32767.0));
```

### tflite minst
Use `train/test.ipynb` to train a model that has been quantized by int8, and due to the large number of operators used, you need to enable the non-zero wait area of ch32v307, you should change line 26 in the `{wch-sdk-path}/hal/CH32V307/Link.ld` line 26.
```
FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
```
Replace it with the following
```
FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 480K
```
This slows down the program, but there really isn't a better way until wch's big resource chip comes along.
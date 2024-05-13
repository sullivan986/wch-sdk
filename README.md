# wch-sdk
CMake SDK port for Qinheng Universal MCU.

## example

### tflite
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
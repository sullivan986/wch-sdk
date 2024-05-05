# wch-sdk
CMake SDK port for Qinheng Universal MCU.

## example

### tflite
Due to compiler issues, if you want to use tflite, you should change line 192 in the `tensorflow/lite/micro/kernels/lstm_eval_common.cc`.
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
Since I'm too lazy to fork it, either esp's port or the original port can be used.
# hello world
## how to use

## support chip
|support  chip|
|:----------- |
|CH32V307     |

## notice
I remove `-fsingle-precision-constant` from c++ flag, But if you want more float unit acceleration, you can add it,then we can use more float unit acceleration in most libraries, and here are a few ways around that:
1. Use `-fsingle-precision-constant` flag.
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
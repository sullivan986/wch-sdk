# wch-sdk
CMake SDK port for Qinheng Universal MCU.

## support content
Currently only some chips are supported, if you need more support you can contact me.
|universal MCU|                        |BLE MCU      |                        | 
|:----------- | :---------------------:|:----------- | :--------------------: | 
|CH32V003     | ![alt text][supported] |CH58X        | ![alt text][todo]      | 
|CH32V203     | ![alt text][todo]      |CH59X        | ![alt text][todo]      | 
|CH32V307     | ![alt text][supported] |CH32V208     | ![alt text][todo]      | 

Third-party library support status.
|libs                                                                           |                         |
|:------------------------------------------------------------------------------| :---------------------: |
|[CherryUSB](https://github.com/cherry-embedded/CherryUSB)                      | ![alt text][supported]  |
|[TinyMaix](https://github.com/sipeed/TinyMaix)                                 | ![alt text][todo]       |
|[tflite-micro](https://github.com/tensorflow/tflite-micro)                     | ![alt text][supported]  |
|[wasm-micro-runtime](https://github.com/bytecodealliance/wasm-micro-runtime)   | ![alt text][todo]       |
|[micropython](https://github.com/micropython/micropython)                      | ![alt text][todo]       |
    
[supported]: https://img.shields.io/badge/-supported-green "supported"
[TODO]: https://img.shields.io/badge/-TODO-orange "todo"

## How to use
It is easy to use the library by simply setting up the path to the compilation toolchain and then using the config_app function as in the example.

### clone repository and install toolchains
<strong>warning</strong>: This repository can only be used under linux!

Clone the repository and download the toolchain.

```
git clone --recursive https://github.com/sullivan986/wch-sdk
cd wch-sdk
source install_toolchains.sh
```

Wait for the download to complete and start using the hello_world example.

```
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -Sexample/hello_world -Bbuild -G Ninja
cmake --build build --config Release --target hello_world --
```

Then you will see the app.hex in the build directory.

## example notice
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

<strong>warning</strong>: I've taken care of all the above caveats, and all the routines compile fine!

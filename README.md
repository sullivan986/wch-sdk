# wch-sdk
CMake SDK port for Qinheng Universal MCU.

## support content
Currently only some chips are supported, if you need more support you can contact me.
|universal MCU|                        |BLE MCU      |                        | 
|:----------- | :---------------------:|:----------- | :--------------------: | 
|CH32V003     | ![alt text][supported] |CH58X        | ![alt text][todo]      | 
|CH32V203     | ![alt text][todo]      |CH59X        | ![alt text][supported] | 
|CH32V307     | ![alt text][supported] |CH32V208     | ![alt text][todo]      | 

Third-party library support status.
|libs                                                                           |                         |
|:------------------------------------------------------------------------------| :---------------------: |
|[CherryUSB](https://github.com/cherry-embedded/CherryUSB)                      | ![alt text][supported]  |
|[TinyMaix](https://github.com/sipeed/TinyMaix)                                 | ![alt text][todo]       |
|[tflite-micro](https://github.com/tensorflow/tflite-micro)                     | ![alt text][supported]  |
|[wasm-micro-runtime](https://github.com/bytecodealliance/wasm-micro-runtime)   | ![alt text][todo]       |
|[micropython](https://github.com/micropython/micropython)                      | ![alt text][todo]       |
|[lvgl](https://github.com/lvgl/lvgl)                                           | ![alt text][supported]  |
    
[supported]: https://img.shields.io/badge/-supported-green "supported"
[TODO]: https://img.shields.io/badge/-TODO-orange "todo"

## How to use
It is easy to use the library by simply setting up the path to the compilation toolchain and then using the config_app function as in the example.

### clone repository and install toolchains

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

Then you will see the app.hex file in the build directory.


## Notice
<strong>warning</strong>: This repository can only be used under linux.

<strong>warning</strong>: This repository is primarily focused on c++ support for a number of reasons.

<strong>warning</strong>: This example is mainly based on ch32v307, please refer to the official EVT for other chip routines.
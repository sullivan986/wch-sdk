cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
set(CHIP_NAME "ch32v307")

# 项目设置
project(ch32v307 C CXX ASM)

include(../../../../wch_init.cmake)

config_app(ch32v307_tflite_hello_world)
enable_rtos(10)
enable_tflite()

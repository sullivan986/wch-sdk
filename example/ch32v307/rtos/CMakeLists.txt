cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
set(CHIP_NAME "ch32v307")

# 项目设置
project(ch32v307 C CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

include(../../../wch_init.cmake)

config_app(ch32v307_rtos)
enable_rtos(10)
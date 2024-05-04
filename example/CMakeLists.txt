cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
set(WCH_SDK_PATH ..)
set(CHIP_NAME "ch32v307")

# 项目设置
project(ch582-ninja C CXX ASM)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)

include(${WCH_SDK_PATH}/wch_init.cmake)

config_app(test)
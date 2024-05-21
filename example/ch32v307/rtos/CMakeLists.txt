cmake_minimum_required(VERSION 3.20)

# 工具链设置
set(TOOLPATH  /PATH/TO/GCC/BIN)
project(rtos_test C CXX ASM)
include(../../../wch_init.cmake)

config_app(ch32v307_rtos ch32v307 enable_rtos enable_cpp enable_printf_sdi r64k_f256k)
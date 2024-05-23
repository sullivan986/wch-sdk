cmake_minimum_required(VERSION 3.20)

# 工具链设置
project(rtos C CXX ASM)
include(../../wch_init.cmake)

config_app(rtos ch32v307 enable_rtos enable_cpp enable_printf_sdi r64k_f256k)
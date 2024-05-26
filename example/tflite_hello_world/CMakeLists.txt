cmake_minimum_required(VERSION 3.20)

project(tflite_hello_world C CXX ASM)
include(../../wch_init.cmake)

config_app(tflite_hello_world ch32v307 enable_rtos enable_cpp enable_printf_sdi enable_tflite r128k_f480k)
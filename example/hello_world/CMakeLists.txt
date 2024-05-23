cmake_minimum_required(VERSION 3.25)

project(hello_world C CXX ASM)
include(../../wch_init.cmake)

config_app(hello_world ch32v003 enable_cpp enable_printf_sdi)
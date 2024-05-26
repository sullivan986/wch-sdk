cmake_minimum_required(VERSION 3.25)

project(hello_world_no_cpp C CXX ASM)
include(../../wch_init.cmake)

config_app(hello_world_no_cpp ch32v307 enable_printf_sdi)
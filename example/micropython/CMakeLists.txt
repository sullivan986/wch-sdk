cmake_minimum_required(VERSION 3.25)

project(micropython C CXX ASM)
include(../../wch_init.cmake)

config_app(micropython ch32v307 enable_printf_sdi enable_micropython)
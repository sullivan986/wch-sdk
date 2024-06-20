cmake_minimum_required(VERSION 3.25)

project(ble_uart C CXX ASM)
include(../../wch_init.cmake)

config_app(ble_easy_peripheral ch592 enable_cpp enable_printf_uart1)

target_compile_definitions(ble_easy_peripheral PRIVATE  
    INT_SOFT
    BLE_BUFF_MAX_LEN=251
) 
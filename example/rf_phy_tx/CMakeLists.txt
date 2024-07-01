cmake_minimum_required(VERSION 3.25)

project(ble_uart C CXX ASM)
include(../../wch_init.cmake)

config_app(ble_rf_phy_tx ch592 enable_printf_uart1)

target_compile_definitions(ble_rf_phy_tx PRIVATE  
    INT_SOFT
) 
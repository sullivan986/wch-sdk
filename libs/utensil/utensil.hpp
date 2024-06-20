#ifndef UTENSIL_HPP
#define UTENSIL_HPP

#include "stdio.h"

#ifdef UTENSIL_SET_CHIP_ch32v307
#include "debug.h"
inline void ch_init()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
#ifdef WCH_USE_SDI_PRINTF
    SDI_Printf_Enable();
#elifdef WCH_USE_UART1_PRINTF
#endif
}

#elifdef UTENSIL_SET_CHIP_ch32v003

#elifdef UTENSIL_SET_CHIP_ch592
#include "CH59x_common.h"
#include "ble/ble_controller.hpp"

void ch_init()
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#if DEBUG == Debug_UART1
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#elif DEBUG == Debug_UART2
    GPIOA_SetBits(bTXD2);
    GPIOA_ModeCfg(bTXD2, GPIO_ModeOut_PP_5mA);
    UART2_DefInit();
#endif
}

#endif

#ifdef WCH_USE_LIB_FREERTOS
#include <utensil_rtos.hpp>
#endif

#ifdef WCH_USE_LIB_TFLITE
#include <utensil_tflite.hpp>
#endif

#ifdef WCH_USE_LIB_LVGL
#include <lv_port_wch.hpp>
#endif

// CPP
#ifdef WCH_USE_CPP
extern "C" void _fini()
{
}
extern "C" void _init()
{
}
void *__dso_handle = 0;
#endif

#endif
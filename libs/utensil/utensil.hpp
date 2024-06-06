#ifndef UTENSIL_HPP
#define UTENSIL_HPP

#include "stdio.h"

extern "C"
{

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
    inline void ch_init()
    {
        SetSysClock(CLK_SOURCE_PLL_60MHz);
#if DEBUG == Debug_UART1
        GPIOA_SetBits(GPIO_Pin_9);
        GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
        UART1_DefInit();
#elif DEBUG == Debug_UART2
        GPIOA_SetBits(GPIO_Pin_9);
        GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
        UART1_DefInit();
#endif
    }

#endif
}

#ifdef WCH_USE_LIB_FREERTOS
#include <utensil_rtos.hpp>
#endif

#ifdef WCH_USE_LIB_TFLITE
#include <utensil_tflite.hpp>
#endif

#ifdef WCH_USE_CPP
extern "C"
{
    void _fini()
    {
    }
    void _init()
    {
    }
}
#endif

#endif
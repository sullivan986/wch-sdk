#ifndef UTENSIL_HPP
#define UTENSIL_HPP

#include "stdio.h"
#include <cstdint>

// for chip
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
#undef max
#undef min
#include "ble/ble_controller.hpp"
#include "ble/rf_controller.h"
#include "sensor/vl53l0x.h"
// usb debug
#ifdef Debug_USB
#include "peripherals/usb_cdc.h"
extern "C" __INTERRUPT __HIGH_CODE void TMR0_IRQHandler(void) // TMR0 定时中断
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // 清除中断标志
        USB_IRQProcessHandler();
    }
}
extern "C" int _write(int fd, char *buf, int size)
{
    for (int i = 0; i < size; i++)
    {
        usb_uart_send_data(buf[i]);
    }
    return size;
}
#endif

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

#ifdef Debug_USB
#ifdef PRINT
#undef PRINT
#define PRINT printf
#endif
    InitUSBDevPara();
    InitUSBDevice();
    PFIC_EnableIRQ(USB_IRQn);

    TMR0_TimerInit(FREQ_SYS / 1000);       // 设置定时时间 1ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
    PFIC_EnableIRQ(TMR0_IRQn);
    DelayMs(50);
#endif
    log_info("Start @ChipID=%02X", R8_CHIP_ID);
}

#endif

// custom lib
#ifdef WCH_USE_LIB_FREERTOS
#include <freertos/utensil_rtos.hpp>
#endif

#ifdef WCH_USE_LIB_TFLITE
#include <tflite/utensil_tflite.hpp>
#endif

#ifdef WCH_USE_LIB_LVGL8
#include <lvgl8/lv_port_wch.hpp>
#endif

#ifdef WCH_USE_LIB_LVGL9
#include <lvgl9/lv_port_wch.hpp>
#endif

#endif
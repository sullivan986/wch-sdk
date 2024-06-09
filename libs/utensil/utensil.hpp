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

#ifdef WCH_USE_LIB_LVGL
#include "lvgl.h"
extern "C"
{
    void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM4_IRQHandler(void)
    {
        if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
        {
            lv_tick_inc(LV_TICK_PERIOD_MS);
            TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        }
    }
}

void lv_create_tick(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef timebase_conf;
    timebase_conf.TIM_ClockDivision = TIM_CKD_DIV1;
    timebase_conf.TIM_CounterMode = TIM_CounterMode_Up;
    timebase_conf.TIM_Period = LV_TICK_PERIOD_MS * 10 - 1;
    timebase_conf.TIM_Prescaler = SystemCoreClock / 10000 - 1;
    TIM_TimeBaseInit(TIM4, &timebase_conf);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef nvic_conf;
    nvic_conf.NVIC_IRQChannel = TIM4_IRQn;
    nvic_conf.NVIC_IRQChannelPreemptionPriority = 1;
    nvic_conf.NVIC_IRQChannelSubPriority = 0;
    nvic_conf.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_conf);

    TIM_Cmd(TIM4, ENABLE);
}

static lv_color_t lvgl_draw_buff1[240 * 40];
static lv_color_t lvgl_draw_buff2[240 * 40];
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    while (bsp_lcd_get_trans_stat() == 0)
        ;
    lv_disp_flush_ready(disp_drv);
    bsp_lcd_draw_rect(area->x1, area->y1, area->x2, area->y2, color_p);
}
void lv_port_disp_init(void)
{
    /* 向lvgl注册缓冲区 */
    static lv_disp_draw_buf_t draw_buf_dsc; // 需要全程生命周期，设置为静态变量
    lv_disp_draw_buf_init(&draw_buf_dsc, lvgl_draw_buff1, lvgl_draw_buff2, LVGL_PORT_BUFF_SIZE);
    /* 创建并初始化用于在lvgl中注册显示设备的结构 */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); // 使用默认值初始化该结构
    /* 设置屏幕分辨率 */
    disp_drv.hor_res = BSP_LCD_X_PIXELS;
    disp_drv.ver_res = BSP_LCD_Y_PIXELS;
    /* 设置显示矩形函数，用于将矩形缓冲区刷新到屏幕上 */
    disp_drv.flush_cb = disp_flush;
    /* 设置缓冲区 */
    disp_drv.draw_buf = &draw_buf_dsc;
    /* 注册显示设备 */
    lv_disp_drv_register(&disp_drv);
}
void lib_lvgl_init()
{
    lv_init();
    lv_port_disp_init();
    lv_create_tick();
}
#endif
#elifdef UTENSIL_SET_CHIP_ch32v003

#elifdef UTENSIL_SET_CHIP_ch592
#include "CH59x_common.h"
#include "ble/ble_peripheral_controller.hpp"
void ch_init()
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

#ifdef WCH_USE_LIB_FREERTOS
#include <utensil_rtos.hpp>
#endif

#ifdef WCH_USE_LIB_TFLITE
#include <utensil_tflite.hpp>
#endif

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
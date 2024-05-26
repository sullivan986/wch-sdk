#ifndef UTENSIL_HPP
#define UTENSIL_HPP

#include "debug.h"
#include <stdio.h>

extern "C"
{
    void ch32_init()
    {
        
    }
}
// usb-print
// extern "C"
// {
//     void usb_dc_low_level_init(void)
//     {
//         RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
//         RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
//         RCC_USBHSConfig(RCC_USBPLL_Div2);
//         RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
//         RCC_USBHSPHYPLLALIVEcmd(ENABLE);

//         RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
//         NVIC_EnableIRQ(USBHS_IRQn);

//         RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
//         // EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
//         NVIC_EnableIRQ(OTG_FS_IRQn);
//         Delay_Us(100);
//     }

//     void TIM3_Init(uint16_t arr, uint16_t psc)
//     {
//         TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
//         NVIC_InitTypeDef NVIC_InitStructure = {0};

//         /* Enable timer3 clock */
//         RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

//         /* Initialize timer3 */
//         TIM_TimeBaseStructure.TIM_Period = arr;
//         TIM_TimeBaseStructure.TIM_Prescaler = psc;
//         TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//         TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//         TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

//         /* Enable updating timer3 interrupt */
//         TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

//         /* Configure timer3 interrupt */
//         NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
//         NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//         NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//         NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//         NVIC_Init(&NVIC_InitStructure);

//         /* Enable timer3 */
//         TIM_Cmd(TIM3, ENABLE);

//         /* Enable timer3 interrupt */
//         NVIC_EnableIRQ(TIM3_IRQn);
//     }

//     void board_init(void)
//     {
//         NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//         Delay_Init();
//         USART_Printf_Init(115200);
//         TIM3_Init(9, SystemCoreClock / 1000 - 1);
//     }

//     volatile uint32_t millis_count = 0;

//     void TIM3_IRQHandler(void)
//     {
//         if (TIM_GetITStatus(TIM3, TIM_IT_Update) != 0)
//         {
//             millis_count++;
//             /* Clear interrupt flag */
//             TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//         }
//     }

//     uint32_t chey_board_millis(void)
//     {
//         return millis_count;
//     }
// }

#ifdef USE_UTENSIL_FREERTOS
#include <utensil_rtos.hpp>
#endif

#ifdef USE_UTENSIL_TFLITE
#include <utensil_tflite.hpp>
#endif

#endif
#include "debug.h"
#include "utensil.hpp"

int main(void)
{

    ch_init();
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    while (1)
    {
        printf("hello world!\r\n");
        Delay_Ms(1000);
    }
}

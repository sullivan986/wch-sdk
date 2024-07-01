
#include "base.h"
#include "ble/rf_controller.h"
#include "utensil.hpp"

int main()
{
    ch_init();

    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeIN_PD);

    rf_controller::rf_controller rc(rf_controller::TX);

    rc.set_send_or_recv_periodic(20);

    rc.before_send([&]() {
        if (GPIOA_ReadPortPin(GPIO_Pin_4) == 16)
        {
            rc.set_send_data({1, 1});
        }
        else
        {
            rc.set_send_data({1, 0});
        }
    });

    rc.start();
}

#include "base.h"
#include "ble/rf_controller.h"
#include "utensil.hpp"
#include <cstdint>
#include <vector>

int main()
{
    ch_init();
    GPIOA_ResetBits(GPIO_Pin_4);
    GPIOA_ModeCfg(GPIO_Pin_4, GPIO_ModeOut_PP_20mA);

    rf_controller::rf_controller rc(rf_controller::RX);

    rc.set_send_or_recv_periodic(20);

    rc.set_recv_cb([](std::vector<uint8_t> recv_data) {
        if (recv_data[1] == 0)
        {
            GPIOA_SetBits(GPIO_Pin_4);
        }
        else
        {
            GPIOA_ResetBits(GPIO_Pin_4);
        }
        //  log_info("main recv : 1: %d 2: %d", recv_data[0], recv_data[1]);
    });

    rc.start();
}
#include "utensil.hpp"

int main()
{
    ch_init();
    log_info("Start @ChipID=%02X\r\n", R8_CHIP_ID);

    ble_controller::ble_controller bc;
    bc.set_att_name("att_device")
        .set_mode_peripheral()
        .set_advart_uuid(0xfff0)
        .set_scan_rsp_name("test_01")
        .set_mtu_size(20)
        .set_max_connection(1);

    auto service_1 = bc.add_service_by_id<0>().set_uuid(0xff00);

    auto service_2 = bc.add_service_by_id<1>().set_uuid(0xff10);

    // add characteristic service for service 1

    auto notify_char = service_1.add_characteristic_by_id(0)
                           .set_uuid(0xff01)
                           .set_description("this is char 0")
                           .set_value_size(4)
                           .enable_read()
                           .enable_write()
                           .enable_notify()
                           .enable_indicate();

    service_1.add_characteristic_by_id(1)
        .set_uuid(0xff02)
        .set_description("this is char 1")
        .set_value_size(4)
        .enable_read()
        .enable_write()
        .set_write_cb_func([&](std::vector<uint8_t> characteristic_buff, std::span<const uint8_t> recv_data) {
            log_info("recv data length %d:", recv_data.size());
            for (auto &&data : recv_data)
            {
                log_info("data: %#X", data);
            }
            notify_char.notify(recv_data);
        });

    // add characteristic service for service 2
    service_2.add_characteristic_by_id(0)
        .set_uuid(0xff11)
        .set_description("this is service2 char 0")
        .set_value_size(6)
        .enable_read()
        .enable_write();

    service_2.add_characteristic_by_id(1)
        .set_uuid(0xff12)
        .set_description("this is service2 char 1")
        .set_value_size(4)
        .enable_read()
        .enable_write()
        .enable_notify()
        .enable_indicate();

    auto timer1 = bc.create_steady_timer();
    auto timer2 = bc.create_steady_timer();

    timer1.async_wait([&]() {
        log_info("this is timer 1");
        timer1.expires_after(5000);
    });

    timer2.async_wait([&]() {
        log_info("this is timer 2");
        timer2.expires_after(2000);
    });

    timer1.expires_at(10000);
    timer2.expires_at(5000);

    bc.start();
}
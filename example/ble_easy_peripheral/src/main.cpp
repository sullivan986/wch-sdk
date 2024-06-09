#include "utensil.hpp"

int main()
{
    ch_init();
    PRINT("Start @ChipID=%02X\r\n", R8_CHIP_ID);

    ble_peripheral_controller bc;
    bc.set_att_name("att_device")
        .set_advart_uuid(0xfff0)
        .set_scan_name("test_01")
        .set_max_buff_size(20)
        .set_max_connection(1);

    auto service_1 = bc.add_service_by_id<0>().set_uuid(0xff00);

    auto service_2 = bc.add_service_by_id<1>().set_uuid(0xff10);

    // add characteristic service for service 1
    auto notify_characteristic = service_1.add_characteristic_by_id(0)
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
        .set_write_cb_fun(
            [&notify_characteristic](std::vector<uint8_t> characteristic_buff, std::span<const uint8_t> recv_data) {
                PRINT("recv data length %d :", recv_data.size());
                for (auto &&data : recv_data)
                {
                    PRINT("  %#X", data);
                }
                PRINT("  \r\n");
                notify_characteristic.notify({{0x11, 0x22, 0x33}});
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

    bc.start();
}
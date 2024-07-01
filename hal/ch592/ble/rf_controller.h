#ifndef BLE_CONTROLLER_H
#define BLE_CONTROLLER_H

#include "CH59xBLE_LIB.h"
#include "CH59x_common.h"
#include "CONFIG.h"
#include "HAL.h"
#include "base.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <queue>
#include <vector>

namespace rf_controller
{

enum rf_x_mode
{
    TX,
    RX
};

class rf_controller
{
  public:
    rf_controller(rf_x_mode mode);

    void set_recv_cb(std::function<void(std::vector<uint8_t>)> task);

    void set_send_or_recv_periodic(size_t size);

    void set_send_data(std::vector<uint8_t> data);

    void before_send(std::function<void(void)> task);

    void add_loop_task(std::function<void(void)> task);

    void start();

  private:
    void RF_Init();
};

} // namespace rf_controller
#endif
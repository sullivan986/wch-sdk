#ifndef CH592_I2C_H
#define CH592_I2C_H

#include "CH59xBLE_LIB.h"
#include "CH59x_common.h"
#include "CH59x_i2c.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace peripherals
{
class i2c_device
{
  public:
    i2c_device();

    constexpr i2c_device &set_self_addr(uint16_t addr)
    {
        _self_addr = addr;
        return *this;
    }

    constexpr i2c_device &set_frq(uint32_t frq)
    {
        _i2c_frq = frq;
        return *this;
    }

    constexpr i2c_device &set_addr_7bit()
    {
        _self_addr = I2C_AckAddr_7bit;
        return *this;
    }

    constexpr i2c_device &set_target_addr(uint16_t addr)
    {
        _target_addr = addr;
        return *this;
    }

    void init();

    void send_data(std::span<const uint8_t> data);

    std::vector<uint8_t> send_and_recv(std::span<const uint8_t> send_data, size_t recv_size);

  private:
    uint16_t _self_addr = 0x42;
    uint16_t _target_addr = 0x52;
    uint32_t _i2c_frq = 400000;
    I2C_AckAddrTypeDef _ack_addr_bit = I2C_AckAddr_7bit;

    std::vector<uint8_t> recv_data(size_t len, uint16_t addr);
};
} // namespace peripherals
#endif
#ifndef CH592_I2C_HPP
#define CH592_I2C_HPP

#include "peripherals/i2c.h"
namespace peripherals
{

i2c_device::i2c_device()
{
    GPIOB_ModeCfg(GPIO_Pin_14 | GPIO_Pin_15, GPIO_ModeIN_PU);
}

void i2c_device::init()
{
    I2C_Init(I2C_Mode_I2C, _i2c_frq, I2C_DutyCycle_16_9, I2C_Ack_Enable, I2C_AckAddr_7bit, _self_addr);
    DelayMs(40);
}

void i2c_device::send_data(std::span<const uint8_t> data)
{
    // start
    while (I2C_GetFlagStatus(I2C_FLAG_BUSY) != RESET)
        ;
    I2C_GenerateSTART(ENABLE);

    // 写入从机地址
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
        ;
    I2C_Send7bitAddress(_target_addr << 1, I2C_Direction_Transmitter);

    // 检测状态
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;
    for (auto &&i : data)
    {
        if (I2C_GetFlagStatus(I2C_FLAG_TXE) != RESET)
        {
            I2C_SendData(i);
        }
    }

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;
    I2C_GenerateSTOP(ENABLE);
}

std::vector<uint8_t> i2c_device::send_and_recv(std::span<const uint8_t> sendr_data, size_t recv_size)
{
    std::vector<uint8_t> recv_data;

    while (I2C_GetFlagStatus(I2C_FLAG_BUSY) != RESET)
        ;
    I2C_GenerateSTART(ENABLE);

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
        ;
    I2C_Send7bitAddress(_target_addr << 1, I2C_Direction_Transmitter);

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    for (auto &&i : sendr_data)
    {
        if (I2C_GetFlagStatus(I2C_FLAG_TXE) != RESET)
        {
            I2C_SendData(i);
        }
    }

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        ;

    I2C_GenerateSTART(ENABLE); // 重起始信号

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
        ;

    // 判断BUSY, MSL and SB flags
    I2C_Send7bitAddress((_target_addr << 1) | 1, I2C_Direction_Receiver); // 发送地址+最低位1表示为“读”
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        ;

    for (int i = 0; i < recv_size; i++)
    {
        if (i == recv_size - 1)
        {
            I2C_AcknowledgeConfig(DISABLE); // 清除ACK位 主设备为了能在收到最后一个字节后产生一个NACK脉冲，
                                            // 必须在读取倒数第二个字节之后（倒数第二个RxNE 事件之后）清除ACK位(ACK=0)

            while (!I2C_GetFlagStatus(I2C_FLAG_RXNE))
                ; // 获取RxEN的状态，等待收到数据
            recv_data.emplace_back(I2C_ReceiveData());
        }
    }

    I2C_GenerateSTOP(ENABLE); // 停止信号
    return std::move(recv_data);
}

std::vector<uint8_t> i2c_device::recv_data(size_t len, uint16_t addr)
{
    std::vector<uint8_t> recv_buff(len);
    uint8_t i = 0;
    while (I2C_GetFlagStatus(I2C_FLAG_BUSY) != RESET)
        ;
    I2C_GenerateSTART(ENABLE);

    // 写入从机地址
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
        ;
    I2C_Send7bitAddress(addr, I2C_Direction_Transmitter);

    while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        ;

    if (len == 1)
    {
        I2C_GenerateSTOP(ENABLE);
        while (I2C_GetFlagStatus(I2C_FLAG_RXNE) != RESET)
            ;
        recv_buff[0] = I2C_ReceiveData();
    }
    else
    {
        for (auto &&i : recv_buff)
        {
            while (I2C_GetFlagStatus(I2C_FLAG_RXNE) != RESET)
                ;
            i = I2C_ReceiveData();
        }
        I2C_GenerateSTOP(ENABLE);
    }

    return std::move(recv_buff);
}

} // namespace peripherals
#endif
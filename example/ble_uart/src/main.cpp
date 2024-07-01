#include "ble/ble_controller.hpp"
#include "utensil.hpp"
#include <cstdint>
#include <cstdio>
#include <queue>
#include <span>
#include <vector>

std::queue<uint8_t> bleRX_uartTX_fifo;
std::queue<uint8_t> bleTX_uartRX_fifo;

void uart3_init(int baudrate)
{
    // uart tx io
    GPIOA_SetBits(bTXD3);
    GPIOA_ModeCfg(bTXD3, GPIO_ModeOut_PP_5mA);
    // uart rx io
    GPIOA_SetBits(bRXD3);
    GPIOA_ModeCfg(bRXD3, GPIO_ModeIN_PU);
    UART3_DefInit();
    UART3_BaudRateCfg(baudrate);
    // uart interrupt conf
    UART3_INTCfg(ENABLE, RB_IER_RECV_RDY | RB_IER_LINE_STAT);
    PFIC_EnableIRQ(UART3_IRQn);
}

extern "C" __INTERRUPT __HIGH_CODE void UART3_IRQHandler(void)
{
    switch (UART3_GetITFlag())
    {
    case UART_II_LINE_STAT: // 线路状态错误
        UART3_GetLinSTA();
        break;

    case UART_II_RECV_RDY:  // 数据达到设置触发点 设置了 UART3_ByteTrigCfg 才会触发
    case UART_II_RECV_TOUT: // 接收超时，暂时一帧数据接收完成
    {
        while (R8_UART3_RFC)
        {
            bleTX_uartRX_fifo.push((uint8_t)R8_UART3_RBR);
        }
    }
    break;
    case UART_II_THR_EMPTY: // 发送缓存区空，可继续发送
        break;
    case UART_II_MODEM_CHG: // 只支持串口0
        break;
    default:
        break;
    }
}

int main()
{
    ch_init();
    log_info("Start @ChipID=%02X", R8_CHIP_ID);
    uart3_init(115200);

    auto bc = ble_controller::ble_controller()
                  .set_mode_peripheral()
                  .set_att_name("att_device")
                  .set_advart_uuid(0xfff0)
                  .set_scan_rsp_name("ch592_ble_uart") // needy
                  .set_mtu_size(20)
                  .set_max_connection(1);

    auto service_1 = bc.add_service_by_id<0>().set_uuid(0xff00);

    // add characteristic service for service 1
    auto ble_tx_char = service_1.add_characteristic_by_id(0)
                           .set_uuid(0xff01)
                           .set_description("notify char")
                           .set_value_size(1)
                           .enable_notify();

    auto ble_rx_char =
        service_1.add_characteristic_by_id(1)
            .set_uuid(0xff02)
            .set_description("recv data char")
            .set_value_size(1)
            .enable_write_no_rsp()
            .set_write_cb_func([&](std::vector<uint8_t> characteristic_buff, std::span<const uint8_t> recv_data) {
                for (auto &&byte : recv_data)
                {
                    bleRX_uartTX_fifo.push(byte);
                }
            });

    auto timer1 = bc.create_steady_timer();

    timer1.async_wait([&]() {
        if (!bleTX_uartRX_fifo.empty())
        {
            std::vector<uint8_t> buff;
            while (buff.size() < peripheralMTU && !bleTX_uartRX_fifo.empty())
            {
                buff.emplace_back(bleTX_uartRX_fifo.front());
                bleTX_uartRX_fifo.pop();
            }
            ble_tx_char.notify(buff);
        }
        if (!bleRX_uartTX_fifo.empty())
        {
            for (; !bleRX_uartTX_fifo.empty(); bleRX_uartTX_fifo.pop())
            {
                if (R8_UART3_TFC != UART_FIFO_SIZE)
                {
                    R8_UART3_THR = bleTX_uartRX_fifo.front();
                }
            }
        }
        timer1.expires_after(1);
    });

    timer1.expires_at(10000);

    bc.start();
}
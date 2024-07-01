#include "ble/rf_controller.h"
#include "base.h"
#include <cstdint>
#include <cstdio>
#include <vector>

namespace rf_controller
{

volatile bool tx_end_flag = false;

#define SBP_RF_PERIODIC_EVT 1 << 0

static uint8_t main_task_id = INVALID_TASK_ID; // Task ID for internal task/event processing
static std::vector<uint8_t> send_data = {1, 1};
static std::vector<uint8_t> recv_buf(251);
static uint32_t send_periodic;
static rf_x_mode _mode;

static std::vector<std::function<void(void)>> user_task;
static std::function<void(std::vector<uint8_t>)> user_recv_task;
static std::function<void(void)> before_send_func;

__HIGH_CODE
__attribute__((noinline)) void Main_Circulation()
{
    while (1)
    {
        TMOS_SystemProcess();
        for (auto &&i : user_task)
        {
            i();
        }
    }
}

__HIGH_CODE
__attribute__((noinline)) void RF_Wait_Tx_End()
{
    uint32_t i = 0;
    while (!tx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // Լ5ms��ʱ
        if (i > (FREQ_SYS / 1000))
        {
            tx_end_flag = true;
        }
    }
}

void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch (sta)
    {
    case TX_MODE_TX_FINISH: {
        // log_info("tx finish");
        break;
    }
    case TX_MODE_TX_FAIL: {
        tx_end_flag = true;
        // log_info("tx fail");
        break;
    }
    case TX_MODE_RX_DATA: {
        break;
    }
    case TX_MODE_RX_TIMEOUT: // Timeout is about 200us
    {
        break;
    }
    case RX_MODE_RX_DATA: {
        if (crc == 0)
        {
            uint8_t i;
            // rxBuf[0] 默认情况下是rssi,如果 在LLEMode初始化时同时开启了LLE_MODE_NON_RSSI,那这里将会变为PKT_TYPE
            // log_info("[rf] rx rssi: %d len: %d d1: %d d2: %d", (int8_t)rxBuf[0], rxBuf[1], rxBuf[2], rxBuf[3]);
            std::vector<uint8_t> recv_data;

            for (i = 0; i < rxBuf[1]; i++)
            {
                recv_data.emplace_back(rxBuf[i + 2]);
            }
            user_recv_task(recv_data);
        }
        else
        {
            if (crc & (1 << 0))
            {
                log_info("[rf] crc error");
            }

            if (crc & (1 << 1))
            {
                log_info("[rf] match type error");
            }
        }
        break;
    }
    case RX_MODE_TX_FINISH: {
        break;
    }
    case RX_MODE_TX_FAIL: {
        break;
    }
    }
}

uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if (events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if ((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if (events & SBP_RF_PERIODIC_EVT)
    {
        if (_mode == RX)
        {
            auto state = RF_Rx(recv_buf.data(), recv_buf.size(), 0xFF, 0xFF);
            //   log_info("[rf] RX mode.state = %x\n", state);
            tmos_start_task(main_task_id, SBP_RF_PERIODIC_EVT, send_periodic);
            return events ^ SBP_RF_PERIODIC_EVT;
        }
        else
        {
            RF_Shut();
            tx_end_flag = false;
            before_send_func();
            if (!RF_Tx(send_data.data(), send_data.size(), 0xFF, 0xFF))
            {
                RF_Wait_Tx_End();
            }
            tmos_start_task(main_task_id, SBP_RF_PERIODIC_EVT, send_periodic);
            return events ^ SBP_RF_PERIODIC_EVT;
        }
    }
}

void rf_controller::RF_Init(void)
{
    uint8_t state;
    rfConfig_t rf_Config{};

    main_task_id = TMOS_ProcessEventRegister(RF_ProcessEvent);

    // 这里的 accessAddress 就是preamble后跟的同步字,用于识别一个新包到来,收发设置成一样是是收到包的前提
    // 这个accessAddress 的设置要符合蓝牙规范,比如禁止使用0x55555555以及0xAAAAAAAA (
    // 建议不超过24次位反转，且不超过连续的6个0或1 )
    rf_Config.accessAddress = 0x71764129;
    rf_Config.CRCInit = 0x555555;

    // 这里的工作模式有两种LLE_MODE_BASIC和LLE_MODE_AUTO
    // 其中LLE_MODE_BASIC 是基本的收发,而LLE_MODE_AUTO 是带自动回复的,根据实际场景来选择
    // 另外在最近更新的库里面 增加了两个可配置选项LLE_MODE_NON_RSSI和LLE_MODE_EX_CHANNEL(ch57x m0 系列不支持)
    // 在ch58x平台,通过这里的对应bit来设置不同的PHY 速率,如1M/2M/500K/125K
    // 使能 LLE_MODE_EX_CHANNEL 表示 选择 rfConfig.Frequency 作为通信频点，频点步进1KHz
    // 如果不使能LLE_MODE_EX_CHANNEL 就使用rfConfig.Channel 作为通信频点
    rf_Config.LLEMode = LLE_MODE_BASIC | LLE_MODE_EX_CHANNEL | LLE_MODE_PHY_2M;

    // The channel is mapped to ble channel, ble4.x adv at channel 37/38/39
    // 这里的信道映射是BLE的信道,如37信道对应2402Mhz,这个值在收发也要相同
    rf_Config.Channel = 39;
    rf_Config.Frequency = 2480000;

    // 注册状态回调函数,这里主要是发送完成,接收到数据之类的会进入这个回调函数
    rf_Config.rfStatusCB = RF_2G4StatusCallBack;
    rf_Config.RxMaxlen = 251;
    state = RF_Config(&rf_Config);
    log_info("[rf] rf 2.4g init: %x\n", state);

    tmos_set_event(main_task_id, SBP_RF_PERIODIC_EVT);
}

rf_controller::rf_controller(rf_x_mode mode)
{
    _mode = mode;
}

void rf_controller::set_send_or_recv_periodic(size_t periodic)
{
    send_periodic = periodic * 8 / 5;
}

void rf_controller::set_send_data(std::vector<uint8_t> data)
{
    send_data = data;
}

void rf_controller::before_send(std::function<void(void)> task)
{
    before_send_func = task;
}

void rf_controller::add_loop_task(std::function<void(void)> task)
{
    user_task.emplace_back(task);
}

void rf_controller::set_recv_cb(std::function<void(std::vector<uint8_t>)> task)
{
    user_recv_task = task;
}

void rf_controller::start()
{
    CH59x_BLEInit(); // 初始化蓝牙库
    HAL_Init();
    RF_RoleInit(); // 配置此设备的角色为RF Role
    RF_Init();
    Main_Circulation();
}

} // namespace rf_controller
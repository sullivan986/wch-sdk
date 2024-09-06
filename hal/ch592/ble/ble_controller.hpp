#ifndef BLE_CONTROLLER_HPP
#define BLE_CONTROLLER_HPP

#include "CH59xBLE_LIB.h"
#include "CH59x_common.h"
#include "CONFIG.h"
#include "HAL.h"
#include "base.h"
#include "functional"
#include "string"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <map>
#include <queue>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ble_controller
{

// PERI
// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
static const int DEFAULT_ADVERTISING_INTERVAL = 80;
// Minimum connection interval (units of 1.25ms, 6=7.5ms)
static const int DEFAULT_DESIRED_MIN_CONN_INTERVAL = 6;
// Maximum connection interval (units of 1.25ms, 100=125ms)
static const int DEFAULT_DESIRED_MAX_CONN_INTERVAL = 100;

static uint8_t main_task_id = INVALID_TASK_ID; // Task ID for internal task/event processing
// sys task id
static uint16_t START_EVENT_ID;
static uint16_t peripheralMTU = ATT_MTU_SIZE;
//  characteristic value has changed callback
struct peripheralConnItem_t
{
    uint16_t connHandle; // Connection handle of current connection
    uint16_t connInterval;
    uint16_t connSlaveLatency;
    uint16_t connTimeout;
};
static peripheralConnItem_t peripheralConnList;
// TMOS Main_Circulation
static std::queue<std::function<void(void)>> user_tmp_task;
static std::function<void(void)> link_terminated_cb;
static std::function<void(void)> link_established_cb;

__HIGH_CODE
__attribute__((noinline)) static void Main_Circulation()
{
    while (1)
    {
        for (auto i = 0; i < user_tmp_task.size(); i++)
        {
            user_tmp_task.front()();
            user_tmp_task.pop();
        }
        TMOS_SystemProcess();
    }
}

static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent);
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch (newState & GAPROLE_STATE_ADV_MASK)
    {
    case GAPROLE_STARTED:
        log_info("[ROLE] Initialized..");
        break;

    case GAPROLE_ADVERTISING:
        if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
        {
            Peripheral_LinkTerminated(pEvent);
            log_info("[ROLE] Disconnected.. Reason:%x", pEvent->linkTerminate.reason);
            log_info("[ROLE] Advertising..");
        }
        else if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
        {
            log_info("[ROLE] Advertising..");
        }
        break;

    case GAPROLE_CONNECTED:
        if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
        {
            Peripheral_LinkEstablished(pEvent);
            log_info("[ROLE] Connected..");
        }
        break;

    case GAPROLE_CONNECTED_ADV:
        if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
        {
            log_info("[ROLE] Connected Advertising..");
        }
        break;

    case GAPROLE_WAITING:
        if (pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
        {
            log_info("[ROLE] Waiting for advertising..");
        }
        else if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
        {
            Peripheral_LinkTerminated(pEvent);
            log_info("[ROLE] Disconnected.. Reason:%x", pEvent->linkTerminate.reason);
        }
        else if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
        {
            if (pEvent->gap.hdr.status != SUCCESS)
            {
                log_info("[ROLE] Waiting for advertising..");
            }
            else
            {
                log_info("[ROLE] Error..");
            }
        }
        else
        {
            log_info("[ROLE] Error..%x", pEvent->gap.opcode);
        }
        break;

    case GAPROLE_ERROR:
        log_info("[ROLE] Error..");
        break;

    default:
        break;
    }
}

static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval, uint16_t connSlaveLatency,
                                    uint16_t connTimeout)
{
    if (connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        log_info("[UPDATE] Update %x - Int %x ", connHandle, connInterval);
    }
    else
    {
        log_info("[UPDATE] ERR..");
    }
}

static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks
    NULL,                          // When a valid RSSI is read from controller (not used by application)
    peripheralParamUpdateCB};
// 以上是不用动的

// Peripheral state
struct characteristic_causality
{
    uint8_t uuid_LO_HI[ATT_BT_UUID_SIZE];

    bool flag_bcast = false;
    bool flag_read = false;
    bool flag_write_no_rsp = false;
    bool flag_write = false;
    bool flag_notify = false;
    bool flag_indicate = false;
    bool flag_authen = false;
    bool flag_extended = false;

    gattCharCfg_t config[PERIPHERAL_MAX_CONNECTION];
    uint8_t cfg_table_value_index;

    uint8_t props = 0;

    std::vector<uint8_t> buff;

    std::string description;

    std::function<void(std::span<const uint8_t> recv_data)> write_callback_fun;
};

struct service_causality
{
    // service uuid
    uint8_t uuid_LO_HI[ATT_BT_UUID_SIZE];
    gattAttrType_t service_profile;
    // profile table
    std::vector<gattAttribute_t> gatt_profile_table;

    // characteristics causality
    std::map<uint16_t, characteristic_causality> characteristics;

    // service
    gattServiceCBs_t service_profile_CBs;
};
static std::map<uint16_t, service_causality> service_map;

// Tmos
static std::map<uint16_t, std::function<void(void)>> tmos_process_event_map;

// 建立连接后在这里添加自定义task初始化
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;
    // See if already connected
    if (peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        log_info("[LINKE] Connection max...");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;
        peripheralMTU = ATT_MTU_SIZE;
        link_established_cb();
        // Start read rssi
        log_info("[LINKE] Conn %x - Int %x ", event->connectionHandle, event->connInterval);
    }
}

// 断开连接后在这里停止自定义task
static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)
{
    gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *)pEvent;

    if (event->connectionHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;
        // tmos_stop_task(main_task_id, SBP_READ_RSSI_EVT);
        link_terminated_cb();
        // Restart advertising
        {
            uint8_t advertising_enable = TRUE;
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
        }
    }
    else
    {
        log_info("[LINKT] ERR..");
    }
}

// lib CB 连接状态改变时
static void simpleProfile_HandleConnStatusCB(uint16_t connHandle, uint8_t changeType)
{
    // Make sure this is not loopback connection
    if (connHandle != LOOPBACK_CONNHANDLE)
    {
        // Reset Client Char Config if connection has dropped
        if ((changeType == LINKDB_STATUS_UPDATE_REMOVED) ||
            ((changeType == LINKDB_STATUS_UPDATE_STATEFLAGS) && (!linkDB_Up(connHandle))))
        {
            for (auto &&[service_uuid, service] : service_map)
            {
                for (auto &&[characteristic_uuid, characteristic] : service.characteristics)
                {
                    if (characteristic.flag_indicate || characteristic.flag_notify)
                    {
                        GATTServApp_InitCharCfg(connHandle, (gattCharCfg_t *)characteristic.config);
                    }
                }
            }
        }
    }
}

// instead simpleProfile_ReadAttrCB
template <uint16_t SERVICE_UUID>
bStatus_t service_read_callback(uint16_t connHandle, gattAttribute_t *pAttr, uint8_t *pValue, uint16_t *pLen,
                                uint16_t offset, uint16_t maxLen, uint8_t method)
{
    bStatus_t status = SUCCESS;

    // Make sure it's not a blob operation (no attributes in the profile are long)
    if (offset > 0)
    {
        return (ATT_ERR_ATTR_NOT_LONG);
    }

    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        bool find_flag = false;
        for (auto &&[characteristic_uuid, characteristic] : service_map[SERVICE_UUID].characteristics)
        {
            if (uuid == characteristic_uuid)
            {
                if (maxLen > characteristic.buff.size())
                {
                    *pLen = characteristic.buff.size();
                }
                else
                {
                    *pLen = maxLen;
                }
                log_info("service[%x] char[%x] being read", SERVICE_UUID, characteristic_uuid);
                tmos_memcpy(pValue, pAttr->pValue, *pLen);
                find_flag = true;
            }
        }

        if (find_flag == false)
        {
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else
    {
        // 128-bit UUID
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }
    return (status);
}

// instead simpleProfile_WriteAttrCB
// lib CB 写 Callback
template <uint16_t SERVICE_UUID>
static bStatus_t service_write_callback(uint16_t connHandle, gattAttribute_t *pAttr, uint8_t *pValue, uint16_t len,
                                        uint16_t offset, uint8_t method)
{
    bStatus_t status = SUCCESS;

    // If attribute permissions require authorization to write, return error
    if (gattPermitAuthorWrite(pAttr->permissions))
    {
        // Insufficient authorization
        return (ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if (pAttr->type.len == ATT_BT_UUID_SIZE)
    {
        // 16-bit UUID
        uint16_t uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);

        for (auto &&[characteristic_uuid, characteristic] : service_map[SERVICE_UUID].characteristics)
        {
            if (uuid == characteristic_uuid)
            {
                // 自己处理收到的数据, 不一定要写给CHAR1, 写入最大不能超过 mtu - 3
                //  Make sure it's not a blob oper
                if (offset != 0)
                {
                    status = ATT_ERR_ATTR_NOT_LONG;
                }

                // Write the value
                if (status == SUCCESS)
                {
                    log_info("service[%d] char[%d] being write", SERVICE_UUID, characteristic_uuid);
                    //  tmos_memcpy(pAttr->pValue, pValue, characteristic.buff.size()); // 将recvdata 写入
                    std::span<uint8_t> recv_data((uint8_t *)pValue, len);
                    characteristic.write_callback_fun(recv_data);
                }
            }
        }

        if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
        {
            if (pValue[0] == 1)
            {
                log_info("[CFG] notify enable");
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY);
            }
            else if (pValue[0] == 2)
            {
                log_info("[CFG] indicate enable");
                status =
                    GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_INDICATE);
            }
            else if (pValue[0] == 0)
            {
                log_info("[CFG] indicate/notify disable");
                status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset, GATT_CFG_NO_OPERATION);
            }
            else
            {
                log_info("[CFG] cfg error recv: %d %d", pValue[0], pValue[1]);
            }
        }
        else
        {
            status = ATT_ERR_ATTR_NOT_FOUND;
        }
    }
    else
    {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }
    return (status);
}

// CENTRAL
static const int DEFAULT_DISCOVERY_MODE = DEVDISC_MODE_ALL;
static const int DEFAULT_DISCOVERY_ACTIVE_SCAN = TRUE;
static const int DEFAULT_DISCOVERY_WHITE_LIST = FALSE;

// SCAN
static const int DEFAULT_MAX_SCAN_RES = 10;
static uint8_t centralScanRes;
static gapDevRec_t centralDevList[DEFAULT_MAX_SCAN_RES];

// Peer device address
static uint8_t PeerAddrDef[B_ADDR_LEN] = {0x02, 0x02, 0x03, 0xE4, 0xC2, 0x84};

static void centralAddDeviceInfo(uint8_t *pAddr, uint8_t addrType)
{
    uint8_t i;

    // If result count not at max
    if (centralScanRes < DEFAULT_MAX_SCAN_RES)
    {
        // Check if device is already in scan results
        for (i = 0; i < centralScanRes; i++)
        {
            if (tmos_memcmp(pAddr, centralDevList[i].addr, B_ADDR_LEN))
            {
                return;
            }
        }
        // Add addr to scan result list
        tmos_memcpy(centralDevList[centralScanRes].addr, pAddr, B_ADDR_LEN);
        centralDevList[centralScanRes].addrType = addrType;
        // Increment scan result count
        centralScanRes++;
        // Display device addr
        log_info("[scan] Device %d - Addr %x %x %x %x %x %x \n", centralScanRes,
                 centralDevList[centralScanRes - 1].addr[0], centralDevList[centralScanRes - 1].addr[1],
                 centralDevList[centralScanRes - 1].addr[2], centralDevList[centralScanRes - 1].addr[3],
                 centralDevList[centralScanRes - 1].addr[4], centralDevList[centralScanRes - 1].addr[5]);
    }
}

static void centralEventCB(gapRoleEvent_t *pEvent)
{
    switch (pEvent->gap.opcode)
    {
    case GAP_DEVICE_INIT_DONE_EVENT: {
        PRINT("[central] Discovering...\n");
        GAPRole_CentralStartDiscovery(DEFAULT_DISCOVERY_MODE, DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                      DEFAULT_DISCOVERY_WHITE_LIST);
    }
    break;

    case GAP_DEVICE_INFO_EVENT: {
        // Add device to list
        centralAddDeviceInfo(pEvent->deviceInfo.addr, pEvent->deviceInfo.addrType);
    }
    break;

    case GAP_DEVICE_DISCOVERY_EVENT: {
        uint8_t i;

        // See if peer device has been discovered
        for (i = 0; i < centralScanRes; i++)
        {
            if (tmos_memcmp(PeerAddrDef, centralDevList[i].addr, B_ADDR_LEN))
                break;
        }

        // Peer device not found
        if (i == centralScanRes)
        {
            PRINT("Device not found...\n");
            centralScanRes = 0;
            GAPRole_CentralStartDiscovery(DEFAULT_DISCOVERY_MODE, DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                          DEFAULT_DISCOVERY_WHITE_LIST);
            PRINT("Discovering...\n");
        }

        // Peer device found
        else
        {
            PRINT("Device found...\n");
            GAPRole_CentralEstablishLink(FALSE, FALSE, centralDevList[i].addrType, centralDevList[i].addr);

            // Start establish link timeout event
            tmos_start_task(main_task_id, ESTABLISH_LINK_TIMEOUT_EVT, ESTABLISH_LINK_TIMEOUT);
            PRINT("Connecting...\n");
        }
    }
    break;

    case GAP_LINK_ESTABLISHED_EVENT: {
        tmos_stop_task(centralTaskId, ESTABLISH_LINK_TIMEOUT_EVT);
        if (pEvent->gap.hdr.status == SUCCESS)
        {
            centralState = BLE_STATE_CONNECTED;
            centralConnHandle = pEvent->linkCmpl.connectionHandle;
            centralProcedureInProgress = TRUE;

            // Update MTU
            attExchangeMTUReq_t req = {
                .clientRxMTU = BLE_BUFF_MAX_LEN - 4,
            };

            GATT_ExchangeMTU(centralConnHandle, &req, centralTaskId);

            // Initiate service discovery
            tmos_start_task(centralTaskId, START_SVC_DISCOVERY_EVT, DEFAULT_SVC_DISCOVERY_DELAY);

            // See if initiate connect parameter update
            if (centralParamUpdate)
            {
                tmos_start_task(centralTaskId, START_PARAM_UPDATE_EVT, DEFAULT_PARAM_UPDATE_DELAY);
            }
            // See if initiate phy update
            if (centralPhyUpdate)
            {
                tmos_start_task(centralTaskId, START_PHY_UPDATE_EVT, DEFAULT_PHY_UPDATE_DELAY);
            }
            // See if start RSSI polling
            if (centralRssi)
            {
                tmos_start_task(centralTaskId, START_READ_RSSI_EVT, DEFAULT_RSSI_PERIOD);
            }

            PRINT("Connected...\n");
        }
        else
        {
            PRINT("Connect Failed...Reason:%X\n", pEvent->gap.hdr.status);
            PRINT("Discovering...\n");
            centralScanRes = 0;
            GAPRole_CentralStartDiscovery(DEFAULT_DISCOVERY_MODE, DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                          DEFAULT_DISCOVERY_WHITE_LIST);
        }
    }
    break;

    case GAP_LINK_TERMINATED_EVENT: {
        centralState = BLE_STATE_IDLE;
        centralConnHandle = GAP_CONNHANDLE_INIT;
        centralDiscState = BLE_DISC_STATE_IDLE;
        centralCharHdl = 0;
        centralScanRes = 0;
        centralProcedureInProgress = FALSE;
        tmos_stop_task(centralTaskId, START_READ_RSSI_EVT);
        PRINT("Disconnected...Reason:%x\n", pEvent->linkTerminate.reason);
        PRINT("Discovering...\n");
        GAPRole_CentralStartDiscovery(DEFAULT_DISCOVERY_MODE, DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                      DEFAULT_DISCOVERY_WHITE_LIST);
    }
    break;

    case GAP_LINK_PARAM_UPDATE_EVENT: {
        PRINT("Param Update...\n");
    }
    break;

    case GAP_PHY_UPDATE_EVENT: {
        PRINT("PHY Update...\n");
    }
    break;

    case GAP_EXT_ADV_DEVICE_INFO_EVENT: {
        // Display device addr
        PRINT("Recv ext adv \n");
        // Add device to list
        centralAddDeviceInfo(pEvent->deviceExtAdvInfo.addr, pEvent->deviceExtAdvInfo.addrType);
    }
    break;

    case GAP_DIRECT_DEVICE_INFO_EVENT: {
        // Display device addr
        PRINT("Recv direct adv \n");
        // Add device to list
        centralAddDeviceInfo(pEvent->deviceDirectInfo.addr, pEvent->deviceDirectInfo.addrType);
    }
    break;

    default:
        break;
    }
}

// TMOS 处理函数
static uint16_t TMOS_process_event(uint8_t task_id, uint16_t events)
{
    if (events & SYS_EVENT_MSG)
    {
        auto pMsg = tmos_msg_receive(main_task_id);
        if (pMsg != NULL)
        {
            // gatt
            gattMsgEvent_t *gatt_msg_evt = (gattMsgEvent_t *)pMsg;
            if (gatt_msg_evt->method == ATT_MTU_UPDATED_EVENT)
            {
                peripheralMTU = gatt_msg_evt->msg.exchangeMTUReq.clientRxMTU;
                log_info("[GATT] mtu exchange: %d", gatt_msg_evt->msg.exchangeMTUReq.clientRxMTU);
            }

            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    for (auto &&[event_id, event_func] : tmos_process_event_map)
    {
        if (events & event_id)
        {
            event_func();
            return (events ^ event_id);
        }
    }
    // Discard unknown events
    return 0;
}

class ble_controller
{
  public:
    enum class BleState
    {
        peripheral,
        broadcaster,
        central
    } ble_state;

    // 增加 characteristic操作类
    class characteristic_operator
    {
      public:
        characteristic_operator(ble_controller *parent, uint16_t p_service_uuid, uint16_t p_characteristic_uuid)
            : _parent(parent), _parent_service_uuid(p_service_uuid)
        {
            service_map[p_service_uuid].characteristics[p_characteristic_uuid] = {};
            target_characteristic = &service_map[p_service_uuid].characteristics[p_characteristic_uuid];

            target_characteristic->uuid_LO_HI[0] = (uint8_t)(LO_UINT16(p_characteristic_uuid));
            target_characteristic->uuid_LO_HI[1] = (uint8_t)(HI_UINT16(p_characteristic_uuid));
        }

        characteristic_operator &set_value_size(size_t size)
        {
            target_characteristic->buff.resize(size);
            return *this;
        }

        characteristic_operator &set_description(std::string str)
        {
            target_characteristic->description = str;
            return *this;
        }

        characteristic_operator &set_write_cb_func(std::function<void(std::span<const uint8_t> recv_data)> func)
        {
            target_characteristic->write_callback_fun = func;
            return *this;
        }

        std::vector<uint8_t> &value()
        {
            return target_characteristic->buff;
        }

        characteristic_operator &enable_read()
        {
            target_characteristic->flag_read = true;
            return *this;
        }

        characteristic_operator &enable_write()
        {
            target_characteristic->flag_write = true;
            return *this;
        }

        characteristic_operator &enable_write_no_rsp()
        {
            target_characteristic->flag_write_no_rsp = true;
            return *this;
        }

        characteristic_operator &enable_notify()
        {
            target_characteristic->flag_notify = true;
            return *this;
        }

        characteristic_operator &enable_indicate()
        {
            target_characteristic->flag_indicate = true;
            return *this;
        }

        void notify(std::span<const uint8_t> data)
        {
            // If notifications enabled
            if (GATTServApp_ReadCharCfg(peripheralConnList.connHandle, target_characteristic->config) &
                GATT_CLIENT_CFG_NOTIFY)
            {
                bStatus_t status = SUCCESS;
                attHandleValueNoti_t noti;
                noti.len = data.size();
                noti.pValue =
                    (uint8_t *)GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0);

                if (noti.pValue != NULL)
                {
                    tmos_memcpy(noti.pValue, data.data(), noti.len);
                }
                // Set the handle
                noti.handle = service_map[_parent_service_uuid]
                                  .gatt_profile_table[target_characteristic->cfg_table_value_index]
                                  .handle;
                // Send the notification
                status = GATT_Notification(peripheralConnList.connHandle, &noti, FALSE);
                if (status != SUCCESS)
                {
                    log_info("[notify] faild status:%d", status);
                }
            }
            else
            {
                log_info("[notify] notify is disable");
            }
        }

        uint16_t _parent_service_uuid;
        characteristic_causality *target_characteristic;
        ble_controller *_parent;
    };

    // 增加 service 操作类
    class service_operator
    {
      public:
        service_operator(ble_controller *parent, uint16_t service_uuid) : _parent(parent), _service_uuid(service_uuid)
        {
            service_map[_service_uuid] = {};
            target_service = &service_map[_service_uuid];

            target_service->uuid_LO_HI[0] = (uint8_t)(LO_UINT16(service_uuid));
            target_service->uuid_LO_HI[1] = (uint8_t)(HI_UINT16(service_uuid));
            target_service->service_profile.uuid = service_map[_service_uuid].uuid_LO_HI;
            target_service->service_profile.len = ATT_BT_UUID_SIZE;
            gattAttribute_t primary_profile_table{
                {ATT_BT_UUID_SIZE, primaryServiceUUID},     /* type */
                GATT_PERMIT_READ,                           /* permissions */
                0,                                          /* handle */
                (uint8_t *)&target_service->service_profile /* pValue */
            };
            target_service->gatt_profile_table.emplace_back(primary_profile_table);
        }

        characteristic_operator add_characteristic_by_uuid(uint16_t uuid)
        {
            characteristic_operator f(_parent, _service_uuid, uuid);
            return std::move(f);
        }

        service_causality *target_service;
        uint16_t _service_uuid;
        ble_controller *_parent;
    };

    class ble_steady_timer
    {
      public:
        ble_steady_timer(ble_controller *bc) : _bc(bc)
        {
        }

        void async_wait(std::function<void(void)> task)
        {
            task_id = _bc->add_tmos_event_func(task);
        }

        uint32_t expiry()
        {
            return TMOS_GetSystemClock() * 5 / 8;
        }

        void expires_at(uint32_t ms)
        {
            auto current_time = TMOS_GetSystemClock() * 5 / 8;
            if (ms < current_time)
            {
                return;
            }
            auto real_ms = (ms - current_time) * 8 / 5;
            user_tmp_task.push([&]() { tmos_start_task(main_task_id, task_id, real_ms); });
        }

        void expires_after(uint32_t ms)
        {
            auto real_ms = ms * 8 / 5;
            user_tmp_task.push([&]() { tmos_start_task(main_task_id, task_id, real_ms); });
        }

        void cancel()
        {
            user_tmp_task.push([&]() { tmos_stop_task(main_task_id, task_id); });
        }

      private:
        ble_controller *_bc;
        uint16_t task_id;
    };

    ble_controller()
    {
    }

    ble_controller &set_mode_peripheral()
    {
        ble_state = BleState::peripheral;
        START_EVENT_ID = add_tmos_event_func([]() {
            // Start the Device
            GAPRole_PeripheralStartDevice(main_task_id, nullptr, &Peripheral_PeripheralCBs);
        });

        advert_data_group.emplace_back(
            // Limited discoverable mode advertises for 30.72s, and then stops
            // General discoverable mode advertises indefinitely
            get_uint8_vector({GAP_ADTYPE_FLAGS, GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED}));

        return *this;
    }

    ble_controller &set_mode_broadcaster()
    {
        ble_state = BleState::broadcaster;
        START_EVENT_ID = add_tmos_event_func([&]() {
            // Start the Device
            GAPRole_BroadcasterStartDevice(&Broadcaster_BroadcasterCBs);
        });

        advert_data_group.emplace_back(get_uint8_vector({GAP_ADTYPE_FLAGS, GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED}));

        advert_data_group.emplace_back(get_uint8_vector({GAP_ADTYPE_MANUFACTURER_SPECIFIC, 'b', 'l', 'e'}));

        advert_data_group.emplace_back(get_uint8_vector({GAP_ADTYPE_LOCAL_NAME_SHORT, 'a', 'b', 'c'}));

        return *this;
    }

    ble_controller &set_mode_central()
    {
        ble_state = BleState::central;
        START_EVENT_ID = add_tmos_event_func([&]() {
            // Start the Device
            GAPRole_CentralStartDevice(main_task_id, &centralBondCB, &centralRoleCB);
        });

        return *this;
    }

    constexpr ble_controller &set_att_name(std::string_view name)
    {
        att_device_name = name;
        return *this;
    }

    constexpr ble_controller &set_mac(std::string_view name)
    {
        return *this;
    }

    ble_controller &set_link_terminated_cb(std::function<void(void)> func)
    {
        link_terminated_cb = func;
        return *this;
    }

    ble_controller &set_link_established_cb(std::function<void(void)> func)
    {
        link_established_cb = func;
        return *this;
    }

    ble_controller &set_advart_uuid(uint16_t uuid)
    {
        advert_data_group.emplace_back(get_uint8_vector({0x03,                  // length of this data
                                                         GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all
                                                         (uint8_t)LO_UINT16(uuid), (uint8_t)HI_UINT16(uuid)}));

        return *this;
    }

    ble_controller &set_scan_rsp_name(std::string_view name)
    {
        std::vector<uint8_t> tmp_vector = {(uint8_t)(name.length() + 1), GAP_ADTYPE_LOCAL_NAME_COMPLETE};
        for (auto &i : name)
        {
            tmp_vector.emplace_back(i);
        }
        scan_rsp_data_group.emplace_back(tmp_vector);

        switch (ble_state)
        {
        case BleState::peripheral:
            scan_rsp_data_group.emplace_back(get_uint8_vector(
                {GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE, LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
                 HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
                 HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL)}));
            break;
        case BleState::broadcaster:
            break;
        case BleState::central:
            break;
        }
        scan_rsp_data_group.emplace_back(get_uint8_vector({GAP_ADTYPE_POWER_LEVEL, 0}));
        return *this;
    }

    // TODO
    constexpr ble_controller &set_mtu_size(size_t size)
    {
        return *this;
    }

    constexpr ble_controller &set_max_connection(size_t size)
    {
        return *this;
    }
    constexpr ble_controller &set_mac()
    {
        return *this;
    }

    template <uint16_t UUID> service_operator add_service_by_uuid()
    {
        service_operator _service_operator(this, UUID);
        service_map[UUID].service_profile_CBs = {service_read_callback<UUID>, service_write_callback<UUID>, nullptr};
        return std::move(_service_operator);
    }

    uint16_t add_tmos_event_func(std::function<void(void)> task)
    {
        if (tmos_process_event_map.size() == 15)
        {
            log_info("[tmos] Error, the task queue is full.");
            return 0x8000;
        }
        auto event_id = 1 << tmos_process_event_map.size();
        log_info("[tmos] create tmos task id:%d", event_id);
        tmos_process_event_map[event_id] = task;
        return event_id;
    }

    ble_steady_timer create_steady_timer()
    {
        return std::move(ble_steady_timer(this));
    }

    void chang_broadcast_content(std::span<const uint8_t> data)
    {
        uint8_t initial_advertising_enable = FALSE;
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable); // 关闭广播包
    }

    // connTimeout (units of 10ms, 100=1s)
    void update_conn_param(uint16_t connHandle, uint16_t connIntervalMin, uint16_t connIntervalMax, uint16_t latency,
                           uint16_t connTimeout)
    {
        GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle, connIntervalMin, connIntervalMax, latency,
                                             connTimeout, main_task_id);
    }

    constexpr uint16_t get_mtu_size()
    {
        return peripheralMTU;
    }

    void start()
    {
        // 为每个 service 生成一张 profile table
        for (auto &&[service_uuid, service] : service_map)
        {
            for (auto &&[characteristic_uuid, characteristic] : service.characteristics)
            {
                if (characteristic.flag_bcast)
                {
                    characteristic.props |= GATT_PROP_BCAST;
                }
                if (characteristic.flag_read)
                {
                    characteristic.props |= GATT_PROP_READ;
                }
                if (characteristic.flag_write)
                {
                    characteristic.props |= GATT_PROP_WRITE;
                }
                if (characteristic.flag_write_no_rsp)
                {
                    characteristic.props |= GATT_PROP_WRITE_NO_RSP;
                }
                if (characteristic.flag_notify)
                {
                    characteristic.props |= GATT_PROP_NOTIFY;
                }
                if (characteristic.flag_indicate)
                {
                    characteristic.props |= GATT_PROP_INDICATE;
                }
                if (characteristic.flag_authen)
                {
                    characteristic.props |= GATT_PROP_AUTHEN;
                }
                if (characteristic.flag_extended)
                {
                    characteristic.props |= GATT_PROP_EXTENDED;
                }

                // Declaration
                gattAttribute_t declaration_attr{
                    {ATT_BT_UUID_SIZE, characterUUID}, /* type */
                    GATT_PERMIT_READ,                  /* permissions */
                    0,                                 /* handle */
                    (uint8_t *)&characteristic.props   /* pValue */
                };
                service.gatt_profile_table.emplace_back(declaration_attr);

                // Value
                gattAttribute_t value_attr{
                    {ATT_BT_UUID_SIZE, characteristic.uuid_LO_HI}, /* type */
                    0,                                             /* permissions */
                    0,                                             /* handle */
                    (uint8_t *)characteristic.buff.data()          /* pValue */
                };
                if (characteristic.flag_read)
                {
                    value_attr.permissions |= GATT_PERMIT_READ;
                }
                if (characteristic.flag_write || characteristic.flag_write_no_rsp)
                {
                    value_attr.permissions |= GATT_PERMIT_WRITE;
                }
                service.gatt_profile_table.emplace_back(value_attr);

                // configuration
                if (characteristic.flag_indicate || characteristic.flag_notify)
                {
                    gattAttribute_t configuration_attr{
                        {ATT_BT_UUID_SIZE, clientCharCfgUUID}, /* type */
                        GATT_PERMIT_READ | GATT_PERMIT_WRITE,  /* permissions */
                        0,                                     /* handle */
                        (uint8_t *)characteristic.config       /* pValue */
                    };
                    characteristic.cfg_table_value_index = service.gatt_profile_table.size() - 1;
                    service.gatt_profile_table.emplace_back(configuration_attr);
                }

                // User Description
                if (!characteristic.description.empty())
                {
                    gattAttribute_t user_description_attr{
                        {ATT_BT_UUID_SIZE, charUserDescUUID},        /* type */
                        GATT_PERMIT_READ,                            /* permissions */
                        0,                                           /* handle */
                        (uint8_t *)characteristic.description.data() /* pValue */
                    };
                    service.gatt_profile_table.emplace_back(user_description_attr);
                }
            }
        }

        log_info("[START] ver_lib: %s", VER_LIB);
        CH59x_BLEInit();
        HAL_Init();

        for (auto &&i1 : scan_rsp_data_group)
        {
            for (auto &&i2 : i1)
            {
                scan_rsp_data.emplace_back(i2);
            }
        }
        for (auto &&i1 : advert_data_group)
        {
            for (auto &&i2 : i1)
            {
                advert_data.emplace_back(i2);
            }
        }
        scan_rsp_data_group.clear();
        advert_data_group.clear();

        switch (ble_state)
        {
        case BleState::peripheral:
            GAPRole_PeripheralInit();
            Peripheral_Init();
            break;
        case BleState::broadcaster:
            GAPRole_BroadcasterInit();
            Broadcaster_Init();
            break;
        case BleState::central:
            GAPRole_CentralInit();
            Central_Init();
            break;
        }
        Main_Circulation();
    }

  private:
    // controller parameters
    std::string att_device_name;
    std::vector<std::vector<uint8_t>> scan_rsp_data_group;
    std::vector<std::vector<uint8_t>> advert_data_group;
    std::vector<uint8_t> scan_rsp_data;
    std::vector<uint8_t> advert_data;

    // CB struct
    gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
        NULL, // Not used in peripheral role
        NULL  // Receive scan request callback
    };

    // GAP Role Callbacks
    gapCentralRoleCB_t centralRoleCB = {
        NULL,                 // RSSI callback
        centralEventCB,       // Event callback
        centralHciMTUChangeCB // MTU change callback
    };

    // Bond Manager Callbacks
    gapBondCBs_t centralBondCB = {centralPasscodeCB, centralPairStateCB};

    // func
    void Peripheral_Init()
    {
        main_task_id = TMOS_ProcessEventRegister(TMOS_process_event);
        // Setup the GAP Peripheral Role Profile
        {
            // Setup the GAP Peripheral Role Profile
            uint8_t initial_advertising_enable = TRUE;
            uint16_t desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
            uint16_t desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
            // Set the GAP Role Parameters
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
            GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, scan_rsp_data.size(), scan_rsp_data.data());
            GAPRole_SetParameter(GAPROLE_ADVERT_DATA, advert_data.size(), advert_data.data());
            GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t), &desired_min_interval);
            GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t), &desired_max_interval);
        }
        {
            // Set advertising interval
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, DEFAULT_ADVERTISING_INTERVAL);
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, DEFAULT_ADVERTISING_INTERVAL);
            // Enable scan req notify
            GAP_SetParamValue(TGAP_ADV_SCAN_REQ_NOTIFY, ENABLE);
        }
        // Setup the GAP Bond Manager
        {
            uint32_t passkey = 0; // passkey "000000"
            uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
            uint8_t mitm = TRUE;
            uint8_t bonding = TRUE;
            uint8_t ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
            GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
            GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
            GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
            GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
            GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
        }
        // Initialize GATT attributes
        GGS_AddService(GATT_ALL_SERVICES);         // GAP
        GATTServApp_AddService(GATT_ALL_SERVICES); // GATT attributes
                                                   // Register GATT attribute list and CBs with GATT Server App
        service_init();
        // Set the GAP Characteristics
        GGS_SetParameter(GGS_DEVICE_NAME_ATT, att_device_name.length(), att_device_name.data());
        // Init Connection Item
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;
        // Register receive scan request callback
        GAPRole_BroadcasterSetCB(&Broadcaster_BroadcasterCBs);
        tmos_set_event(main_task_id, START_EVENT_ID);
    }

    void Broadcaster_Init()
    {
        main_task_id = TMOS_ProcessEventRegister(TMOS_process_event);
        // Setup the GAP Broadcaster Role Profile
        {
            // Device starts advertising upon initialization
            uint8_t initial_advertising_enable = TRUE;
            uint8_t initial_adv_event_type = GAP_ADTYPE_ADV_NONCONN_IND;
            // Set the GAP Role Parameters
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
            GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &initial_adv_event_type);
            GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, scan_rsp_data.size(), scan_rsp_data.data());
            GAPRole_SetParameter(GAPROLE_ADVERT_DATA, advert_data.size(), advert_data.data());
        }
        // Set advertising interval
        {
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, DEFAULT_ADVERTISING_INTERVAL);
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, DEFAULT_ADVERTISING_INTERVAL);
        }
        // Setup a delayed profile startup
        tmos_set_event(main_task_id, START_EVENT_ID);
    }

    void Central_Init()
    {
        main_task_id = TMOS_ProcessEventRegister(TMOS_process_event);
        // Setup GAP
        GAP_SetParamValue(TGAP_DISC_SCAN, 2400);              // Scan duration in 0.625ms
        GAP_SetParamValue(TGAP_CONN_EST_INT_MIN, 20);         // Connection min interval in 1.25ms
        GAP_SetParamValue(TGAP_CONN_EST_INT_MAX, 100);        // Connection max interval in 1.25ms
        GAP_SetParamValue(TGAP_CONN_EST_SUPERV_TIMEOUT, 100); // Connection supervision timeout in 10ms

        // Setup the GAP Bond Manager
        {
            uint32_t passkey = 0;
            uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
            uint8_t mitm = TRUE;
            uint8_t ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
            uint8_t bonding = TRUE;

            GAPBondMgr_SetParameter(GAPBOND_CENT_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
            GAPBondMgr_SetParameter(GAPBOND_CENT_PAIRING_MODE, sizeof(uint8_t), &pairMode);
            GAPBondMgr_SetParameter(GAPBOND_CENT_MITM_PROTECTION, sizeof(uint8_t), &mitm);
            GAPBondMgr_SetParameter(GAPBOND_CENT_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
            GAPBondMgr_SetParameter(GAPBOND_CENT_BONDING_ENABLED, sizeof(uint8_t), &bonding);
        }
        // Initialize GATT Client
        GATT_InitClient();
        // Register to receive incoming ATT Indications/Notifications
        GATT_RegisterForInd(main_task_id);
        // Setup a delayed profile startup
        tmos_set_event(main_task_id, START_EVENT_ID);
    }

    // instead SimpleProfile_AddService
    void service_init()
    {
        for (auto &&[service_uuid, service] : service_map)
        {
            for (auto &&[characteristic_uuid, characteristic] : service.characteristics)
            {
                if (characteristic.flag_indicate || characteristic.flag_notify)
                {
                    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, characteristic.config);
                }
            }
            linkDB_Register(simpleProfile_HandleConnStatusCB);
            GATTServApp_RegisterService(service.gatt_profile_table.data(), service.gatt_profile_table.size(),
                                        GATT_MAX_ENCRYPT_KEY_SIZE, &service.service_profile_CBs);
        }
    }

    std::vector<uint8_t> get_uint8_vector(std::initializer_list<uint8_t> data)
    {
        std::vector<uint8_t> tmp;
        tmp.emplace_back(data.size());
        for (auto &&i : data)
        {
            tmp.emplace_back(i);
        }
        return std::move(tmp);
    }
};

} // namespace ble_controller

#endif
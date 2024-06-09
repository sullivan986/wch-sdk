#ifndef BLE_PERIPHERAL_CONTROLLER
#define BLE_PERIPHERAL_CONTROLLER

#include "CH59xBLE_LIB.h"
#include "CH59x_common.h"
#include "CONFIG.h"
#include "HAL.h"
#include "functional"
#include "string"
#include "utensil.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#define SBP_START_DEVICE_EVT 0x0001
#define SBP_READ_RSSI_EVT 0x0004
#define SBP_PARAM_UPDATE_EVT 0x0008
#define SBP_PHY_UPDATE_EVT 0x0010
// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD 6400
// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY 6400
// PHY update delay
#define SBP_PHY_UPDATE_DELAY 2400
// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL 80
// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE GAP_ADTYPE_FLAGS_GENERAL
// Minimum connection interval (units of 1.25ms, 6=7.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL 6
// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL 100
// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY 0
// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT 100

static uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing
// 蓝牙协议栈大小 6k
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];
static uint16_t peripheralMTU = ATT_MTU_SIZE;
//  characteristic value has changed callback
typedef struct
{
    uint16_t connHandle; // Connection handle of current connection
    uint16_t connInterval;
    uint16_t connSlaveLatency;
    uint16_t connTimeout;
} peripheralConnItem_t;
// Connection item list
static peripheralConnItem_t peripheralConnList;
// Profile
__HIGH_CODE
__attribute__((noinline)) void Main_Circulation()
{
    while (1)
    {
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
        PRINT("[ROLE] Initialized..\n\r");
        break;

    case GAPROLE_ADVERTISING:
        if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
        {
            Peripheral_LinkTerminated(pEvent);
            PRINT("[ROLE] Disconnected.. Reason:%x\n\r", pEvent->linkTerminate.reason);
            PRINT("[ROLE] Advertising..\n\r");
        }
        else if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
        {
            PRINT("[ROLE] Advertising..\n\r");
        }
        break;

    case GAPROLE_CONNECTED:
        if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
        {
            Peripheral_LinkEstablished(pEvent);
            PRINT("[ROLE] Connected..\n\r");
        }
        break;

    case GAPROLE_CONNECTED_ADV:
        if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
        {
            PRINT("[ROLE] Connected Advertising..\n\r");
        }
        break;

    case GAPROLE_WAITING:
        if (pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
        {
            PRINT("[ROLE] Waiting for advertising..\n\r");
        }
        else if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
        {
            Peripheral_LinkTerminated(pEvent);
            PRINT("[ROLE] Disconnected.. Reason:%x\n\r", pEvent->linkTerminate.reason);
        }
        else if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
        {
            if (pEvent->gap.hdr.status != SUCCESS)
            {
                PRINT("[ROLE] Waiting for advertising..\n\r");
            }
            else
            {
                PRINT("[ROLE] Error..\n\r");
            }
        }
        else
        {
            PRINT("[ROLE] Error..%x\n\r", pEvent->gap.opcode);
        }
        break;

    case GAPROLE_ERROR:
        PRINT("[ROLE] Error..\n\r");
        break;

    default:
        break;
    }
}
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi)
{
    PRINT("[RSSI] RSSI -%d dB Conn  %x \n\r", -rssi, connHandle);
}
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval, uint16_t connSlaveLatency,
                                    uint16_t connTimeout)
{
    if (connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        PRINT("[UPDATE] Update %x - Int %x \n\r", connHandle, connInterval);
    }
    else
    {
        PRINT("[UPDATE] ERR..\n\r");
    }
}

static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks
    peripheralRssiCB,              // When a valid RSSI is read from controller (not used by application)
    peripheralParamUpdateCB};

// 以上是不用动的
//
//
//
//

struct characteristic_causality
{
    uint16_t uuid;
    uint8_t uuid_LO_HI[ATT_BT_UUID_SIZE];

    bool flag_read = false;
    bool flag_write = false;
    bool flag_notify = false;
    bool flag_indicate = false;
    gattCharCfg_t config[PERIPHERAL_MAX_CONNECTION];

    uint8_t props = 0;

    std::vector<uint8_t> buff;

    std::string description;

    std::function<void(std::vector<uint8_t> characteristic_buff, std::span<const uint8_t> recv_data)>
        write_callback_fun;
};

struct service_causality
{
    // service uuid
    uint16_t uuid;
    uint8_t uuid_LO_HI[ATT_BT_UUID_SIZE];
    gattAttrType_t service_profile;
    // profile table
    std::vector<gattAttribute_t> gatt_profile_table;

    // characteristics causality
    std::map<uint8_t, characteristic_causality> characteristics;

    // service
    gattServiceCBs_t service_profile_CBs;
};
static std::map<uint8_t, service_causality> service_map;

// 建立连接后在这里添加自定义task初始化
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;
    // See if already connected
    if (peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        PRINT("[LINKE] Connection max...\r\n");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;
        peripheralMTU = ATT_MTU_SIZE;
        // Set timer for param update event
        tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);
        // Start read rssi
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
        PRINT("[LINKE] Conn %x - Int %x \r\n", event->connectionHandle, event->connInterval);
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
        tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);

        // Restart advertising
        {
            uint8_t advertising_enable = TRUE;
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
        }
    }
    else
    {
        PRINT("[LINKT] ERR..\n\r");
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
            for (auto &&[service_id, service] : service_map)
            {
                for (auto &&[characteristic_id, characteristic] : service.characteristics)
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

// static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len)
// {
//     // TODO 多service
//     for (auto &&[service_id, service] : service_map)
//     {
//         for (auto &&[characteristic_id, characteristic] : service.characteristics)
//         {
//             if (characteristic_id == paramID)
//             {
//                 uint8_t newValue[characteristic.buff.size()];
//                 tmos_memcpy(newValue, pValue, len);
//                 PRINT("[CHANGE] profile ChangeCB char[%d].. \n\r", characteristic_id);
//                 break;
//             }
//         }
//     }
// }

class ble_peripheral_controller
{
  public:
    // 增加 characteristic操作类
    struct characteristic_operator
    {
        characteristic_operator(ble_peripheral_controller *parent, uint8_t p_service_id, uint8_t p_characteristic_id)
            : _parent(parent)
        {
            service_map[p_service_id].characteristics[p_characteristic_id] = {};
            target_characteristic = &service_map[p_service_id].characteristics[p_characteristic_id];
        }

        characteristic_operator &set_uuid(uint16_t uuid)
        {
            target_characteristic->uuid = uuid;
            target_characteristic->uuid_LO_HI[0] = (uint8_t)(LO_UINT16(uuid));
            target_characteristic->uuid_LO_HI[1] = (uint8_t)(HI_UINT16(uuid));

            return *this;
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

        characteristic_operator &set_write_cb_fun(
            std::function<void(std::vector<uint8_t> characteristic_buff, std::span<const uint8_t> recv_data)> func)
        {
            target_characteristic->write_callback_fun = func;
            return *this;
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
        characteristic_causality *target_characteristic;
        ble_peripheral_controller *_parent;
    };

    // 增加 service 操作类
    struct service_operator
    {
        service_operator(ble_peripheral_controller *parent, uint8_t service_id)
            : _parent(parent), _service_id(service_id)
        {
            service_map[_service_id] = {};
            target_service = &service_map[_service_id];
        }

        characteristic_operator add_characteristic_by_id(uint8_t id)
        {
            characteristic_operator f(_parent, _service_id, id);
            return std::move(f);
        }

        service_operator &set_uuid(uint16_t uuid)
        {
            target_service->uuid = uuid;
            target_service->uuid_LO_HI[0] = (uint8_t)(LO_UINT16(uuid));
            target_service->uuid_LO_HI[1] = (uint8_t)(HI_UINT16(uuid));
            target_service->service_profile.uuid = service_map[_service_id].uuid_LO_HI;
            target_service->service_profile.len = ATT_BT_UUID_SIZE;
            gattAttribute_t primary_profile_table{
                {ATT_BT_UUID_SIZE, primaryServiceUUID},     /* type */
                GATT_PERMIT_READ,                           /* permissions */
                0,                                          /* handle */
                (uint8_t *)&target_service->service_profile /* pValue */
            };
            target_service->gatt_profile_table.emplace_back(primary_profile_table);
            return *this;
        }

        void notify(std::span<uint8_t> data)
        {
        }

        service_causality *target_service;
        uint8_t _service_id;
        ble_peripheral_controller *_parent;
    };

    ble_peripheral_controller()
    {
    }

    constexpr ble_peripheral_controller &set_att_name(std::string_view name)
    {
        att_device_name = name;
        return *this;
    }

    constexpr ble_peripheral_controller &set_advart_uuid(uint16_t uuid)
    {
        advert_data.emplace_back(LO_UINT16(uuid));
        advert_data.emplace_back(HI_UINT16(uuid));
        return *this;
    }

    constexpr ble_peripheral_controller &set_scan_name(std::string_view name)
    {
        scan_rsp_data.emplace_back(name.length() + 1);
        scan_rsp_data.emplace_back(GAP_ADTYPE_LOCAL_NAME_COMPLETE);
        for (auto &i : name)
        {
            scan_rsp_data.emplace_back(i);
        }
        scan_rsp_data.emplace_back(0x05);
        scan_rsp_data.emplace_back(GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE);
        scan_rsp_data.emplace_back(LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL));
        scan_rsp_data.emplace_back(HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL));
        scan_rsp_data.emplace_back(LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL));
        scan_rsp_data.emplace_back(HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL));
        scan_rsp_data.emplace_back(0x02);
        scan_rsp_data.emplace_back(GAP_ADTYPE_POWER_LEVEL);
        scan_rsp_data.emplace_back(0);
        return *this;
    }

    template <uint8_t ID> service_operator add_service_by_id()
    {
        service_operator _service_operator(this, ID);
        service_map[ID].service_profile_CBs = {service_read_callback<ID>, service_write_callback<ID>, nullptr};
        return std::move(_service_operator);
    }

    void start()
    {
        // 为每个 service 生成一张 profile table
        for (auto &&[service_id, service] : service_map)
        {
            for (auto &&[characteristic_id, characteristic] : service.characteristics)
            {
                uint8_t permissions = 0;
                if (characteristic.flag_read)
                {
                    characteristic.props |= GATT_PROP_READ;
                }
                if (characteristic.flag_write)
                {
                    characteristic.props |= GATT_PROP_WRITE;
                }
                if (characteristic.flag_notify)
                {
                    characteristic.props |= GATT_PROP_NOTIFY;
                }
                if (characteristic.flag_indicate)
                {
                    characteristic.props |= GATT_PROP_INDICATE;
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
                if (characteristic.flag_read)
                {
                    permissions |= GATT_PERMIT_READ;
                }
                if (characteristic.flag_write)
                {
                    permissions |= GATT_PERMIT_WRITE;
                }
                gattAttribute_t value_attr{
                    {ATT_BT_UUID_SIZE, characteristic.uuid_LO_HI}, /* type */
                    permissions,                                   /* permissions */
                    0,                                             /* handle */
                    (uint8_t *)characteristic.buff.data()          /* pValue */
                };
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
                    service.gatt_profile_table.emplace_back(configuration_attr);
                }

                // User Description
                gattAttribute_t user_description_attr{
                    {ATT_BT_UUID_SIZE, charUserDescUUID},        /* type */
                    GATT_PERMIT_READ,                            /* permissions */
                    0,                                           /* handle */
                    (uint8_t *)characteristic.description.data() /* pValue */
                };
                service.gatt_profile_table.emplace_back(user_description_attr);
            }
        }

        PRINT("[START] ver_lib: %s\r\n", VER_LIB);
        CH59x_BLEInit();
        HAL_Init();
        GAPRole_PeripheralInit();
        Peripheral_Init();
        Main_Circulation();
    }

    // void notify(std::span<uint8_t> data)
    // {
    //     attHandleValueNoti_t noti;
    //     if (data.size() > (peripheralMTU - 3))
    //     {
    //         PRINT("Too large noti\n\r");
    //         return;
    //     }
    //     noti.len = data.size();
    //     noti.pValue = (uint8_t *)GATT_bm_alloc(peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL,
    //     0); if (noti.pValue)
    //     {
    //         tmos_memcpy(noti.pValue, data.data(), noti.len);
    //         if (simpleProfile_Notify(peripheralConnList.connHandle, &noti) != SUCCESS)
    //         {
    //             GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
    //         }
    //     }
    // }

  private:
    // controller parameters
    std::string att_device_name;
    std::vector<uint8_t> scan_rsp_data;
    std::vector<uint8_t> advert_data{
        // GAP_ADTYPE_FLAGS: 每次发布广告 30 秒
        0x02, // length of this data
        GAP_ADTYPE_FLAGS, DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
        // service UUID, to notify central devices what services are included
        // in this peripheral
        0x03,                  // length of this data
        GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all
    };

    // CB struct

    gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
        NULL, // Not used in peripheral role
        NULL  // Receive scan request callback
    };

    // func
    void Peripheral_Init()
    {
        Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);
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
            uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;
            // Set advertising interval
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
            GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);
            // Enable scan req notify
            GAP_SetParamValue(TGAP_ADV_SCAN_REQ_NOTIFY, ENABLE);
        }
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

        tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);
    }

    // instead SimpleProfile_AddService
    void service_init()
    {
        for (auto &&[service_id, service] : service_map)
        {
            for (auto &&[characteristic_id, characteristic] : service.characteristics)
            {
                if (characteristic.flag_indicate || characteristic.flag_notify)
                {
                    GATTServApp_InitCharCfg(INVALID_CONNHANDLE, characteristic.config);
                }
            }
            GATTServApp_RegisterService(service.gatt_profile_table.data(), service.gatt_profile_table.size(),
                                        GATT_MAX_ENCRYPT_KEY_SIZE, &service.service_profile_CBs);
        }

        linkDB_Register(simpleProfile_HandleConnStatusCB);
    }

    // bStatus_t simpleProfile_Notify(uint16_t connHandle, attHandleValueNoti_t *pNoti)
    // {
    //     uint16_t value;
    //     for (const auto &[id, characteristic] : characteristic_map)
    //     {
    //         if (characteristic.flag_indicate || characteristic.flag_notify)
    //         {
    //             value = GATTServApp_ReadCharCfg(connHandle, (gattCharCfg_t *)characteristic.config);
    //         }
    //     }

    //     // If notifications enabled
    //     if (value & GATT_CLIENT_CFG_NOTIFY)
    //     {
    //         // Set the handle
    //         pNoti->handle = gatt_profile_table[2].handle;
    //         // Send the notification
    //         return GATT_Notification(connHandle, pNoti, FALSE);
    //     }
    //     return bleIncorrectMode;
    // }

    // instead simpleProfile_ReadAttrCB
    template <uint8_t ID>
    static bStatus_t service_read_callback(uint16_t connHandle, gattAttribute_t *pAttr, uint8_t *pValue, uint16_t *pLen,
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
            for (auto &&[characteristic_id, characteristic] : service_map[ID].characteristics)
            {
                if (uuid == characteristic.uuid)
                {
                    if (maxLen > characteristic.buff.size())
                    {
                        *pLen = characteristic.buff.size();
                    }
                    else
                    {
                        *pLen = maxLen;
                    }
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
    template <uint8_t ID>
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
            bool find_flag = false;

            for (auto &&[characteristic_id, characteristic] : service_map[ID].characteristics)
            {
                if (uuid == characteristic.uuid)
                {
                    // 自己处理收到的数据, 不一定要写给CHAR1, 写入最大不能超过 mtu (23 - 3)
                    //  Make sure it's not a blob oper
                    if (offset == 0)
                    {
                        if (len > characteristic.buff.size())
                        {
                            status = ATT_ERR_INVALID_VALUE_SIZE;
                        }
                    }
                    else
                    {
                        status = ATT_ERR_ATTR_NOT_LONG;
                    }

                    // Write the value
                    if (status == SUCCESS)
                    {
                        tmos_memcpy(pAttr->pValue, pValue, characteristic.buff.size());
                        std::span<uint8_t> recv_data((uint8_t *)pValue, len);
                        characteristic.write_callback_fun(characteristic.buff, recv_data);
                    }
                    find_flag = true;
                }
            }

            if (find_flag == false)
            {
                if (uuid == GATT_CLIENT_CHAR_CFG_UUID)
                {
                    PRINT("[CFG] recv cfg: %d %d\r\n", pValue[0], pValue[1]);
                    if (pValue[0] == 1)
                    {
                        status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset,
                                                                GATT_CLIENT_CFG_NOTIFY);
                    }
                    else if (pValue[0] == 2)
                    {
                        status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset,
                                                                GATT_CLIENT_CFG_INDICATE);
                    }
                    else if (pValue[0] == 0)
                    {
                        status = GATTServApp_ProcessCCCWriteReq(connHandle, pAttr, pValue, len, offset,
                                                                GATT_CFG_NO_OPERATION);
                    }
                    else
                    {
                        PRINT("[CFG] cfg error recv: %d %d\r\n", pValue[0], pValue[1]);
                    }
                }
                else
                {
                    status = ATT_ERR_ATTR_NOT_FOUND;
                }
            }
        }
        else
        {
            // 128-bit UUID
            status = ATT_ERR_INVALID_HANDLE;
        }
        return (status);
    }

    // TMOS 处理函数
    static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
    {
        switch (pMsg->event)
        {
        case GAP_MSG_EVENT: {
            auto pEvent = (gapRoleEvent_t *)pMsg;
            switch (pEvent->gap.opcode)
            {
            case GAP_SCAN_REQUEST_EVENT: {
                PRINT("[GAP] Receive scan req from %x %x %x %x %x %x  ..\r\n", pEvent->scanReqEvt.scannerAddr[0],
                      pEvent->scanReqEvt.scannerAddr[1], pEvent->scanReqEvt.scannerAddr[2],
                      pEvent->scanReqEvt.scannerAddr[3], pEvent->scanReqEvt.scannerAddr[4],
                      pEvent->scanReqEvt.scannerAddr[5]);
                break;
            }

            case GAP_PHY_UPDATE_EVENT: {
                PRINT("[GAP] Phy update Rx:%x Tx:%x ..\r\n", pEvent->linkPhyUpdate.connRxPHYS,
                      pEvent->linkPhyUpdate.connTxPHYS);
                break;
            }

            default:
                break;
            }
            break;
        }

        case GATT_MSG_EVENT: {
            gattMsgEvent_t *pMsgEvent;

            pMsgEvent = (gattMsgEvent_t *)pMsg;
            if (pMsgEvent->method == ATT_MTU_UPDATED_EVENT)
            {
                peripheralMTU = pMsgEvent->msg.exchangeMTUReq.clientRxMTU;
                PRINT("[GATT] mtu exchange: %d\r\n", pMsgEvent->msg.exchangeMTUReq.clientRxMTU);
            }
            break;
        }

        default:
            break;
        }
    }

    static uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events)
    {
        //  VOID task_id; // TMOS required parameter that isn't used in this function

        if (events & SYS_EVENT_MSG)
        {
            uint8_t *pMsg;

            if ((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
            {
                Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
                // Release the TMOS message
                tmos_msg_deallocate(pMsg);
            }
            // return unprocessed events
            return (events ^ SYS_EVENT_MSG);
        }

        if (events & SBP_START_DEVICE_EVT)
        {
            // Start the Device
            GAPRole_PeripheralStartDevice(Peripheral_TaskID, nullptr, &Peripheral_PeripheralCBs);
            return (events ^ SBP_START_DEVICE_EVT);
        }

        if (events & SBP_PARAM_UPDATE_EVT)
        {
            // Send connect param update request
            GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle, DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                                 DEFAULT_DESIRED_MAX_CONN_INTERVAL, DEFAULT_DESIRED_SLAVE_LATENCY,
                                                 DEFAULT_DESIRED_CONN_TIMEOUT, Peripheral_TaskID);

            return (events ^ SBP_PARAM_UPDATE_EVT);
        }

        if (events & SBP_PHY_UPDATE_EVT)
        {
            // start phy update
            PRINT("[task] PHY Update %x...\n\r",
                  GAPRole_UpdatePHY(peripheralConnList.connHandle, 0, GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));
            return (events ^ SBP_PHY_UPDATE_EVT);
        }

        if (events & SBP_READ_RSSI_EVT)
        {
            GAPRole_ReadRssiCmd(peripheralConnList.connHandle);
            tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
            return (events ^ SBP_READ_RSSI_EVT);
        }

        // Discard unknown events
        return 0;
    }
};

#endif
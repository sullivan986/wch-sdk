#include "base.h"
#include "CH59x_common.h"
#include "CONFIG.h"

#if (defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

// 蓝牙协议栈大小 6k
__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

extern "C" void _fini()
{
}
extern "C" void _init()
{
}

void *__dso_handle = 0;

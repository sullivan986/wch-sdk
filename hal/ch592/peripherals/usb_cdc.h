#ifndef USB_CDC_HPP
#define USB_CDC_HPP

#include "stdint.h"
#include <span>

void USB_IRQProcessHandler(void);
void InitUSBDevPara(void);
void InitUSBDevice(void);

void usb_uart_send_data(uint8_t data);

#endif
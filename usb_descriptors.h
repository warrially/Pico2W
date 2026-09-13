#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_

#include <stdint.h>

// --------------------------------------------------------------------
// HID Report ID
// --------------------------------------------------------------------

enum
{
    REPORT_ID_GAMEPAD = 1,
};

// --------------------------------------------------------------------
// Gamepad Report
// --------------------------------------------------------------------

// 第一版先做标准 HID Gamepad。
// 后面接 Switch 2 时，我们再根据兼容协议调整这里的格式。
typedef struct __attribute__((packed))
{
    uint16_t nButtons;

    uint8_t nHat;

    uint8_t nLX;
    uint8_t nLY;

    uint8_t nRX;
    uint8_t nRY;
} TGamepadReport;

#endif
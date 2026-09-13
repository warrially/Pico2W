#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_

#include <stdint.h>

// --------------------------------------------------------------------
// Switch 2 Wired Controller Report
// --------------------------------------------------------------------
//
// 总长度：8 bytes
//
// Byte 0-1 : Buttons 1-16
//
// Byte 2:
//   bit0-bit3 : D-Pad Hat
//   bit4      : C Button
//   bit5-bit7 : Reserved
//
// Byte 3 : Left Stick X
// Byte 4 : Left Stick Y
// Byte 5 : Right Stick X
// Byte 6 : Right Stick Y
//
// Byte 7 : Reserved
//

typedef struct __attribute__((packed))
{
    // Byte 0-1
    uint16_t nButtons;

    // Byte 2
    // bit0-bit3 = Hat
    // bit4      = C
    uint8_t nHat;

    // Byte 3-6
    uint8_t nLX;
    uint8_t nLY;

    uint8_t nRX;
    uint8_t nRY;

    // Byte 7
    uint8_t nReserved;

} TSwitch2Report;


// --------------------------------------------------------------------
// Button Mask
// --------------------------------------------------------------------

#define BUTTON_Y        (1u << 0)
#define BUTTON_B        (1u << 1)
#define BUTTON_A        (1u << 2)
#define BUTTON_X        (1u << 3)

#define BUTTON_L        (1u << 4)
#define BUTTON_R        (1u << 5)
#define BUTTON_ZL       (1u << 6)
#define BUTTON_ZR       (1u << 7)

#define BUTTON_MINUS    (1u << 8)
#define BUTTON_PLUS     (1u << 9)

#define BUTTON_LSTICK   (1u << 10)
#define BUTTON_RSTICK   (1u << 11)

#define BUTTON_HOME     (1u << 12)
#define BUTTON_CAPTURE  (1u << 13)

#define BUTTON_GL       (1u << 14)
#define BUTTON_GR       (1u << 15)


// --------------------------------------------------------------------
// C Button
// --------------------------------------------------------------------
//
// C键不在nButtons里。
// 使用Byte 2的bit4。
//

#define BUTTON_C        (1u << 4)


// --------------------------------------------------------------------
// D-Pad Hat
// --------------------------------------------------------------------
//
// Byte 2低4位。
//

#define HAT_UP          0
#define HAT_UP_RIGHT    1
#define HAT_RIGHT       2
#define HAT_DOWN_RIGHT  3
#define HAT_DOWN        4
#define HAT_DOWN_LEFT   5
#define HAT_LEFT        6
#define HAT_UP_LEFT     7

#define HAT_CENTER      8


// --------------------------------------------------------------------
// Helper
// --------------------------------------------------------------------

// 设置D-Pad，同时保留C按钮等高4位。
static inline void switch2SetHat(TSwitch2Report *pReport, uint8_t nHat)
{
    pReport->nHat = (pReport->nHat & 0xF0) | (nHat & 0x0F);
}


// 设置C按钮。
static inline void switch2SetCButton(TSwitch2Report *pReport, uint8_t nPressed)
{
    if (nPressed)
    {
        pReport->nHat |= BUTTON_C;
    }
    else
    {
        pReport->nHat &= (uint8_t)~BUTTON_C;
    }
}

#endif
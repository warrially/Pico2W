#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pico/stdlib.h"

// --------------------------------------------------------------------
// TinyUSB 基础配置
// --------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_PICO
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

// --------------------------------------------------------------------
// USB Root Hub Port
// --------------------------------------------------------------------
//
// Pico / Pico 2 使用RHPORT0作为USB Device。
// 这里明确告诉TinyUSB：
// RHPORT0 = Device + Full Speed
//

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

// --------------------------------------------------------------------
// USB Device
// --------------------------------------------------------------------

#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 0

#define CFG_TUD_ENDPOINT0_SIZE 64

// --------------------------------------------------------------------
// USB Device Class
// --------------------------------------------------------------------

#define CFG_TUD_CDC    0
#define CFG_TUD_MSC    0
#define CFG_TUD_MIDI   0
#define CFG_TUD_VENDOR 0

// HID手柄
#define CFG_TUD_HID 1

// HID Endpoint缓冲区
#define CFG_TUD_HID_EP_BUFSIZE 64

// --------------------------------------------------------------------
// USB Memory
// --------------------------------------------------------------------

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

#ifdef __cplusplus
}
#endif

#endif
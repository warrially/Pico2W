#include <string.h>

#include "tusb.h"
#include "usb_descriptors.h"

// --------------------------------------------------------------------
// USB Device Descriptor
// --------------------------------------------------------------------
//
// HORI HORIPAD O for Nintendo Switch 2
//
// VID : 0x0F0D
// PID : 0x0202
//

static tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,

    .bcdUSB             = 0x0200,

    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0x0F0D,
    .idProduct          = 0x0202,

    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01,
};


uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}


// --------------------------------------------------------------------
// HID Report Descriptor
// --------------------------------------------------------------------
//
// Report总长度：8字节
//
// Byte 0-1 : 16个按钮
//
// Byte 2:
//   bit0-bit3 : D-Pad Hat
//   bit4      : C按钮
//   bit5-bit7 : padding
//
// Byte 3 : Left X
// Byte 4 : Left Y
// Byte 5 : Right X
// Byte 6 : Right Y
//
// Byte 7 : Reserved
//
// 注意：
// 没有Report ID。
// 因此发送时使用：
//
// tud_hid_report(0, &report, sizeof(report));
//

static uint8_t const desc_hid_report[] =
{
    // Usage Page (Generic Desktop)
    0x05, 0x01,

    // Usage (Game Pad)
    0x09, 0x05,

    // Collection (Application)
    0xA1, 0x01,

        // ------------------------------------------------------------
        // 16 Buttons
        // ------------------------------------------------------------

        // Usage Page (Button)
        0x05, 0x09,

        // Usage Minimum (Button 1)
        0x19, 0x01,

        // Usage Maximum (Button 16)
        0x29, 0x10,

        // Logical Minimum = 0
        0x15, 0x00,

        // Logical Maximum = 1
        0x25, 0x01,

        // Report Size = 1 bit
        0x75, 0x01,

        // Report Count = 16
        0x95, 0x10,

        // Input (Data, Variable, Absolute)
        0x81, 0x02,

        // ------------------------------------------------------------
        // D-Pad Hat
        // ------------------------------------------------------------

        // Usage Page (Generic Desktop)
        0x05, 0x01,

        // Usage (Hat Switch)
        0x09, 0x39,

        // Logical Minimum = 0
        0x15, 0x00,

        // Logical Maximum = 7
        0x25, 0x07,

        // Physical Minimum = 0
        0x35, 0x00,

        // Physical Maximum = 315
        0x46, 0x3B, 0x01,

        // Unit = degrees
        0x65, 0x14,

        // Report Size = 4
        0x75, 0x04,

        // Report Count = 1
        0x95, 0x01,

        // Null state enabled
        0x81, 0x42,

        // Clear Unit
        0x65, 0x00,

        // ------------------------------------------------------------
        // Button 17 = C button
        // ------------------------------------------------------------

        // Usage Page (Button)
        0x05, 0x09,

        // Usage Minimum = Button 17
        0x19, 0x11,

        // Usage Maximum = Button 17
        0x29, 0x11,

        // Logical Minimum = 0
        0x15, 0x00,

        // Logical Maximum = 1
        0x25, 0x01,

        // 1 bit
        0x75, 0x01,

        // 1 field
        0x95, 0x01,

        // Input
        0x81, 0x02,

        // ------------------------------------------------------------
        // Padding 3 bits
        // ------------------------------------------------------------

        0x75, 0x03,
        0x95, 0x01,
        0x81, 0x03,

        // ------------------------------------------------------------
        // Left / Right sticks
        // ------------------------------------------------------------

        // Generic Desktop
        0x05, 0x01,

        // X
        0x09, 0x30,

        // Y
        0x09, 0x31,

        // Rx
        0x09, 0x33,

        // Ry
        0x09, 0x34,

        // Logical Minimum = 0
        0x15, 0x00,

        // Logical Maximum = 255
        0x26, 0xFF, 0x00,

        // Report Size = 8
        0x75, 0x08,

        // Report Count = 4
        0x95, 0x04,

        // Input
        0x81, 0x02,

        // ------------------------------------------------------------
        // Reserved byte
        // ------------------------------------------------------------

        // Report Size = 8
        0x75, 0x08,

        // Report Count = 1
        0x95, 0x01,

        // Constant
        0x81, 0x03,

    // End Collection
    0xC0
};


uint8_t const *tud_hid_descriptor_report_cb(uint8_t nInstance)
{
    (void)nInstance;

    return desc_hid_report;
}


// --------------------------------------------------------------------
// Configuration Descriptor
// --------------------------------------------------------------------

enum
{
    ITF_NUM_HID = 0,

    ITF_NUM_TOTAL
};


#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)


#define EPNUM_HID_OUT 0x01
#define EPNUM_HID_IN  0x81


static uint8_t const desc_configuration[] =
{
    // Configuration
    TUD_CONFIG_DESCRIPTOR(
        1,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        0x00,
        100
    ),

    // HID
    //
    // Switch控制器使用IN + OUT两个Interrupt Endpoint。
    //
    TUD_HID_INOUT_DESCRIPTOR(
        ITF_NUM_HID,
        0,
        HID_ITF_PROTOCOL_NONE,
        sizeof(desc_hid_report),

        EPNUM_HID_OUT,
        EPNUM_HID_IN,

        CFG_TUD_HID_EP_BUFSIZE,

        8
    )
};


uint8_t const *tud_descriptor_configuration_cb(uint8_t nIndex)
{
    (void)nIndex;

    return desc_configuration;
}


// --------------------------------------------------------------------
// String Descriptors
// --------------------------------------------------------------------

static char const *string_desc_arr[] =
{
    (const char[]){ 0x09, 0x04 },

    "HORI CO.,LTD.",
    "HORIPAD O",
    "000001",
};


static uint16_t desc_str[32];


uint16_t const *tud_descriptor_string_cb(
    uint8_t nIndex,
    uint16_t nLangID)
{
    (void)nLangID;

    uint8_t nChrCount = 0;

    if (nIndex == 0)
    {
        memcpy(&desc_str[1], string_desc_arr[0], 2);

        nChrCount = 1;
    }
    else
    {
        if (nIndex >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
        {
            return NULL;
        }

        const char *strValue = string_desc_arr[nIndex];

        nChrCount = strlen(strValue);

        if (nChrCount > 31)
        {
            nChrCount = 31;
        }

        for (uint8_t nIndexChar = 0; nIndexChar < nChrCount; nIndexChar++)
        {
            desc_str[1 + nIndexChar] = strValue[nIndexChar];
        }
    }

    desc_str[0] =
        (TUSB_DESC_STRING << 8) |
        (2 * nChrCount + 2);

    return desc_str;
}


// --------------------------------------------------------------------
// HID callbacks
// --------------------------------------------------------------------

uint16_t tud_hid_get_report_cb(
    uint8_t nInstance,
    uint8_t nReportID,
    hid_report_type_t reportType,
    uint8_t *pBuffer,
    uint16_t nReqLen)
{
    (void)nInstance;
    (void)nReportID;
    (void)reportType;
    (void)pBuffer;
    (void)nReqLen;

    return 0;
}


void tud_hid_set_report_cb(
    uint8_t nInstance,
    uint8_t nReportID,
    hid_report_type_t reportType,
    uint8_t const *pBuffer,
    uint16_t nBufferSize)
{
    (void)nInstance;
    (void)nReportID;
    (void)reportType;
    (void)pBuffer;
    (void)nBufferSize;
}
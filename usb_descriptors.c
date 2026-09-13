#include <string.h>

#include "tusb.h"
#include "usb_descriptors.h"

// --------------------------------------------------------------------
// Device Descriptor
// --------------------------------------------------------------------

tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,

    .bcdUSB             = 0x0200,

    // HID设备一般可以保持0，由Interface定义具体类型。
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    // 第一阶段只是Mac测试。
    // 这里先使用测试VID/PID，后面适配Switch 2时再调整。
    .idVendor           = 0xCAFE,
    .idProduct          = 0x4001,

    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};


// TinyUSB会调用这个函数获取Device Descriptor。
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}


// --------------------------------------------------------------------
// HID Report Descriptor
// --------------------------------------------------------------------
//
// 第一版：
// 16个按钮
// 1个Hat
// 4个8bit摇杆轴
//
// Report ID = 1
//

uint8_t const desc_hid_report[] =
{
    // Usage Page (Generic Desktop)
    0x05, 0x01,

    // Usage (Game Pad)
    0x09, 0x05,

    // Collection (Application)
    0xA1, 0x01,

        // Report ID
        0x85, REPORT_ID_GAMEPAD,

        // ------------------------------------------------------------
        // Buttons 1 - 16
        // ------------------------------------------------------------

        // Usage Page (Button)
        0x05, 0x09,

        // Usage Minimum (Button 1)
        0x19, 0x01,

        // Usage Maximum (Button 16)
        0x29, 0x10,

        // Logical Minimum
        0x15, 0x00,

        // Logical Maximum
        0x25, 0x01,

        // Report Count = 16
        0x95, 0x10,

        // Report Size = 1 bit
        0x75, 0x01,

        // Input (Data, Variable, Absolute)
        0x81, 0x02,

        // ------------------------------------------------------------
        // Hat switch
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

        // Unit = Degrees
        0x65, 0x14,

        // Report Size = 4 bits
        0x75, 0x04,

        // Report Count = 1
        0x95, 0x01,

        // Input (Data, Variable, Absolute, Null State)
        0x81, 0x42,

        // Padding 4 bits
        0x75, 0x04,
        0x95, 0x01,
        0x81, 0x03,

        // ------------------------------------------------------------
        // Left / Right Stick
        // ------------------------------------------------------------

        // Usage Page (Generic Desktop)
        0x05, 0x01,

        // Usage X
        0x09, 0x30,

        // Usage Y
        0x09, 0x31,

        // Usage Rx
        0x09, 0x33,

        // Usage Ry
        0x09, 0x34,

        // Logical Minimum = 0
        0x15, 0x00,

        // Logical Maximum = 255
        0x26, 0xFF, 0x00,

        // Report Size = 8 bits
        0x75, 0x08,

        // Report Count = 4
        0x95, 0x04,

        // Input (Data, Variable, Absolute)
        0x81, 0x02,

    // End Collection
    0xC0
};


// TinyUSB获取HID Report Descriptor。
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

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID_IN 0x81


uint8_t const desc_configuration[] =
{
    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(
        1,                  // Configuration number
        ITF_NUM_TOTAL,      // Interface count
        0,                  // String index
        CONFIG_TOTAL_LEN,   // Total length
        0x00,               // Attributes
        100                 // Power in mA
    ),

    // HID Descriptor
    TUD_HID_DESCRIPTOR(
        ITF_NUM_HID,                    // Interface number
        0,                              // String index
        HID_ITF_PROTOCOL_NONE,          // Protocol
        sizeof(desc_hid_report),        // Report descriptor length
        EPNUM_HID_IN,                   // Endpoint
        CFG_TUD_HID_EP_BUFSIZE,         // Endpoint size
        1                               // Poll interval ms
    )
};


// TinyUSB获取Configuration Descriptor。
uint8_t const *tud_descriptor_configuration_cb(uint8_t nIndex)
{
    (void)nIndex;

    return desc_configuration;
}


// --------------------------------------------------------------------
// String Descriptors
// --------------------------------------------------------------------

char const *string_desc_arr[] =
{
    (const char[]){ 0x09, 0x04 }, // 0: English (0x0409)

    "Pico Controller",            // 1: Manufacturer
    "Pico USB Gamepad",           // 2: Product
    "000001",                     // 3: Serial
};


// TinyUSB字符串描述符使用UTF-16。
static uint16_t desc_str[32];


uint16_t const *tud_descriptor_string_cb(uint8_t nIndex, uint16_t nLangID)
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

    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * nChrCount + 2);

    return desc_str;
}


// --------------------------------------------------------------------
// HID callbacks
// --------------------------------------------------------------------

// Host请求读取Feature/Input Report时会进入这里。
// 第一版暂时不需要处理。
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


// Host发送Output/Feature Report时会进入这里。
// 第一版也暂时不处理。
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
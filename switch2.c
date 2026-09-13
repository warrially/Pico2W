#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "usb_descriptors.h"

// --------------------------------------------------------------------
// 发送Switch 2手柄报告
// --------------------------------------------------------------------

static void sendGamepadReport(bool bPressed)
{
    if (!tud_hid_ready())
    {
        return;
    }

    TSwitch2Report report = {
        .nButtons = 0,

        // D-Pad居中
        .nHat = HAT_CENTER,

        // 左摇杆居中
        .nLX = 128,
        .nLY = 128,

        // 右摇杆居中
        .nRX = 128,
        .nRY = 128,

        .nReserved = 0,
    };

    // 测试：每秒切换一次A键状态
    if (bPressed)
    {
        report.nButtons |= BUTTON_A;
    }

    // 无Report ID
    tud_hid_report(0, &report, sizeof(report));
}

// --------------------------------------------------------------------
// main
// --------------------------------------------------------------------

int main(void)
{
    // 初始化TinyUSB
    tusb_init();

    absolute_time_t tmLastToggle = get_absolute_time();

    bool bPressed = false;

    while (true)
    {
        // TinyUSB任务必须持续执行
        tud_task();

        // 每1秒切换一次A键
        if (absolute_time_diff_us(tmLastToggle, get_absolute_time()) >= 1000000)
        {
            tmLastToggle = get_absolute_time();

            bPressed = !bPressed;

            sendGamepadReport(bPressed);
        }

        sleep_ms(1);
    }

    return 0;
}
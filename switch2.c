#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "usb_descriptors.h"

// --------------------------------------------------------------------
// Gamepad helper
// --------------------------------------------------------------------

static void sendGamepadReport(bool bPressed)
{
    if (!tud_hid_ready())
    {
        return;
    }

    TGamepadReport report = {
        .nButtons = 0,
        .nHat = 8,

        .nLX = 128,
        .nLY = 128,

        .nRX = 128,
        .nRY = 128,
    };

    if (bPressed)
    {
        report.nButtons |= 0x0001;
    }

    tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

// --------------------------------------------------------------------
// main
// --------------------------------------------------------------------

int main(void)
{
    // 初始化TinyUSB。
    tusb_init();

    absolute_time_t tmLastToggle = get_absolute_time();

    bool bPressed = false;

    while (true)
    {
        // TinyUSB任务必须持续执行。
        tud_task();

        // 每1秒切换Button 1。
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
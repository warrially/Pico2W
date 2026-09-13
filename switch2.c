// switch2.c

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "wifi.h"
#include "http_server.h"
#include "usb_descriptors.h"

#define WIFI_SSID     "DUODUO"
#define WIFI_PASSWORD "zanzan520"

static void sendGamepadReport(void)
{
    if (!tud_hid_ready())
    {
        return;
    }

    TSwitch2Report report = {
        .nButtons = httpServerGetButtons(),
        .nHat = httpServerGetHat(),

        .nLX = 128,
        .nLY = 128,

        .nRX = 128,
        .nRY = 128,

        .nReserved = 0,
    };

    tud_hid_report(0, &report, sizeof(report));
}

int main(void)
{
    tusb_init();

    if (!wifiInit(WIFI_SSID, WIFI_PASSWORD))
    {
        while (true)
        {
            tud_task();
            sleep_ms(1);
        }
    }

    if (!httpServerInit())
    {
        while (true)
        {
            tud_task();
            sleep_ms(1);
        }
    }

    while (true)
    {
        tud_task();

        httpServerTask();

        sendGamepadReport();

        sleep_ms(5);
    }

    return 0;
}
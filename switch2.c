#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "tusb.h"

#include "wifi.h"
#include "http_server.h"
#include "usb_descriptors.h"

#define WIFI_SSID     "DUODUO"
#define WIFI_PASSWORD "zanzan520"

#define LED_ON_TIME_MS 100

static uint16_t gnLastButtons = 0;
static uint8_t gnLastHat = HAT_CENTER;

static absolute_time_t gtmLEDOffTime;
static bool gbLEDOn = false;


// --------------------------------------------------------------------
// LED
// --------------------------------------------------------------------

static void ledFlash(void)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    gbLEDOn = true;

    gtmLEDOffTime = make_timeout_time_ms(LED_ON_TIME_MS);
}


static void ledTask(void)
{
    if (!gbLEDOn)
    {
        return;
    }

    if (absolute_time_diff_us(get_absolute_time(), gtmLEDOffTime) <= 0)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

        gbLEDOn = false;
    }
}


// --------------------------------------------------------------------
// Gamepad
// --------------------------------------------------------------------

static void sendGamepadReport(void)
{
    if (!tud_hid_ready())
    {
        return;
    }

    uint16_t nButtons = httpServerGetButtons();
    uint8_t nHat = httpServerGetHat();

    // 有新的按键按下时闪LED。
    uint16_t nNewButtons = nButtons & (uint16_t)~gnLastButtons;

    bool bNewHatInput =
        (nHat != HAT_CENTER) &&
        ((gnLastHat & 0x0F) == HAT_CENTER || (nHat & 0x0F) != (gnLastHat & 0x0F));

    // C键位于nHat的bit4。
    bool bNewCInput =
        ((nHat & BUTTON_C) != 0) &&
        ((gnLastHat & BUTTON_C) == 0);

    if (nNewButtons != 0 || bNewHatInput || bNewCInput)
    {
        ledFlash();
    }

    gnLastButtons = nButtons;
    gnLastHat = nHat;

    TSwitch2Report report = {
        .nButtons = nButtons,
        .nHat = nHat,

        .nLX = 128,
        .nLY = 128,

        .nRX = 128,
        .nRY = 128,

        .nReserved = 0,
    };

    tud_hid_report(0, &report, sizeof(report));
}


// --------------------------------------------------------------------
// main
// --------------------------------------------------------------------

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

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    if (!httpServerInit())
    {
        while (true)
        {
            tud_task();

            cyw43_arch_poll();

            ledTask();

            sleep_ms(1);
        }
    }

    while (true)
    {
        tud_task();

        cyw43_arch_poll();

        sendGamepadReport();

        ledTask();

        sleep_ms(1);
    }

    return 0;
}
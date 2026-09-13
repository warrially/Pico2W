// wifi.c

#include <stdio.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "lwip/netif.h"

#include "wifi.h"

static char gstrIP[32];

bool wifiInit(const char *strSSID, const char *strPassword)
{
    if (cyw43_arch_init() != 0)
    {
        return false;
    }

    cyw43_arch_enable_sta_mode();

    int nResult = cyw43_arch_wifi_connect_timeout_ms(
        strSSID,
        strPassword,
        CYW43_AUTH_WPA2_AES_PSK,
        30000
    );

    if (nResult != 0)
    {
        return false;
    }

    ip4_addr_t *pIP = netif_ip4_addr(netif_default);

    snprintf(
        gstrIP,
        sizeof(gstrIP),
        "%s",
        ip4addr_ntoa(pIP)
    );

    return true;
}

const char *wifiGetIP(void)
{
    return gstrIP;
}
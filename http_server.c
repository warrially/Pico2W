#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "http_server.h"
#include "usb_descriptors.h"

#define CLICK_TIME_MS 100
#define CLICK_TIME_US (CLICK_TIME_MS * 1000ULL)

static struct tcp_pcb *gpServer = NULL;


// --------------------------------------------------------------------
// 普通按钮
// --------------------------------------------------------------------

static volatile uint16_t gnButtons = 0;
static volatile uint8_t gnHat = HAT_CENTER;


// --------------------------------------------------------------------
// 左摇杆
// --------------------------------------------------------------------

static volatile uint8_t gnLX = 128;
static volatile uint8_t gnLY = 128;

static volatile bool gbStickClick = false;
static volatile uint64_t gnStickClickEndUS = 0;


// --------------------------------------------------------------------
// Button
// --------------------------------------------------------------------

static void setButton(uint16_t nButton, bool bPressed)
{
    if (bPressed)
    {
        gnButtons |= nButton;
    }
    else
    {
        gnButtons &= (uint16_t)~nButton;
    }
}


// --------------------------------------------------------------------
// Hat
// --------------------------------------------------------------------

static void setHat(uint8_t nHat)
{
    gnHat = nHat;
}


// --------------------------------------------------------------------
// Stick
// --------------------------------------------------------------------

static void setLeftStick(uint8_t nX, uint8_t nY)
{
    gnLX = nX;
    gnLY = nY;
}


static void centerLeftStick(void)
{
    gnLX = 128;
    gnLY = 128;

    gbStickClick = false;
    gnStickClickEndUS = 0;
}


static void clickLeftStick(uint8_t nX, uint8_t nY)
{
    gnLX = nX;
    gnLY = nY;

    gbStickClick = true;

    gnStickClickEndUS = time_us_64() + CLICK_TIME_US;
}


// --------------------------------------------------------------------
// URL参数
// --------------------------------------------------------------------

static int getQueryInt(const char *strPath, const char *strKey, int nDefault)
{
    const char *pQuery = strchr(strPath, '?');

    if (pQuery == NULL)
    {
        return nDefault;
    }

    pQuery++;

    char strSearch[32];

    snprintf(strSearch, sizeof(strSearch), "%s=", strKey);

    const char *pValue = strstr(pQuery, strSearch);

    if (pValue == NULL)
    {
        return nDefault;
    }

    pValue += strlen(strSearch);

    return atoi(pValue);
}


// --------------------------------------------------------------------
// 普通按键API
// --------------------------------------------------------------------

static bool handleButtonAPI(
    const char *strPath,
    const char *strKey,
    uint16_t nButton)
{
    char strDown[64];
    char strUp[64];

    snprintf(strDown, sizeof(strDown), "/api/%s/down", strKey);
    snprintf(strUp, sizeof(strUp), "/api/%s/up", strKey);

    if (strcmp(strPath, strDown) == 0)
    {
        setButton(nButton, true);

        return true;
    }

    if (strcmp(strPath, strUp) == 0)
    {
        setButton(nButton, false);

        return true;
    }

    return false;
}


// --------------------------------------------------------------------
// 方向键API
// --------------------------------------------------------------------

static bool handleHatAPI(
    const char *strPath,
    const char *strKey,
    uint8_t nHat)
{
    char strDown[64];
    char strUp[64];

    snprintf(strDown, sizeof(strDown), "/api/%s/down", strKey);
    snprintf(strUp, sizeof(strUp), "/api/%s/up", strKey);

    if (strcmp(strPath, strDown) == 0)
    {
        setHat(nHat);

        return true;
    }

    if (strcmp(strPath, strUp) == 0)
    {
        setHat(HAT_CENTER);

        return true;
    }

    return false;
}


// --------------------------------------------------------------------
// API
// --------------------------------------------------------------------

static void handleAPI(const char *strPath)
{
    // ------------------------------------------------------------
    // 左摇杆坐标
    // ------------------------------------------------------------

    if (strncmp(strPath, "/api/lstick/set?", 16) == 0)
    {
        int nX = getQueryInt(strPath, "x", 128);
        int nY = getQueryInt(strPath, "y", 128);

        if (nX < 0)
        {
            nX = 0;
        }

        if (nX > 255)
        {
            nX = 255;
        }

        if (nY < 0)
        {
            nY = 0;
        }

        if (nY > 255)
        {
            nY = 255;
        }

        setLeftStick(
            (uint8_t)nX,
            (uint8_t)nY
        );

        return;
    }


    // ------------------------------------------------------------
    // 左摇杆click
    // ------------------------------------------------------------

    if (strncmp(strPath, "/api/lstick/click?", 18) == 0)
    {
        int nX = getQueryInt(strPath, "x", 128);
        int nY = getQueryInt(strPath, "y", 128);

        if (nX < 0)
        {
            nX = 0;
        }

        if (nX > 255)
        {
            nX = 255;
        }

        if (nY < 0)
        {
            nY = 0;
        }

        if (nY > 255)
        {
            nY = 255;
        }

        clickLeftStick(
            (uint8_t)nX,
            (uint8_t)nY
        );

        return;
    }


    // ------------------------------------------------------------
    // 左摇杆回中心
    // ------------------------------------------------------------

    if (strcmp(strPath, "/api/lstick/center") == 0)
    {
        centerLeftStick();

        return;
    }


    // ------------------------------------------------------------
    // ABXY
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "a", BUTTON_A))
    {
        return;
    }

    if (handleButtonAPI(strPath, "b", BUTTON_B))
    {
        return;
    }

    if (handleButtonAPI(strPath, "x", BUTTON_X))
    {
        return;
    }

    if (handleButtonAPI(strPath, "y", BUTTON_Y))
    {
        return;
    }


    // ------------------------------------------------------------
    // L/R
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "l", BUTTON_L))
    {
        return;
    }

    if (handleButtonAPI(strPath, "r", BUTTON_R))
    {
        return;
    }

    if (handleButtonAPI(strPath, "zl", BUTTON_ZL))
    {
        return;
    }

    if (handleButtonAPI(strPath, "zr", BUTTON_ZR))
    {
        return;
    }


    // ------------------------------------------------------------
    // PLUS/MINUS
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "plus", BUTTON_PLUS))
    {
        return;
    }

    if (handleButtonAPI(strPath, "minus", BUTTON_MINUS))
    {
        return;
    }


    // ------------------------------------------------------------
    // HOME/CAPTURE
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "home", BUTTON_HOME))
    {
        return;
    }

    if (handleButtonAPI(strPath, "capture", BUTTON_CAPTURE))
    {
        return;
    }


    // ------------------------------------------------------------
    // 左摇杆按下
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "lstick", BUTTON_LSTICK))
    {
        return;
    }


    // ------------------------------------------------------------
    // 右摇杆按下
    // ------------------------------------------------------------

    if (handleButtonAPI(strPath, "rstick", BUTTON_RSTICK))
    {
        return;
    }


    // ------------------------------------------------------------
    // 十字键
    // ------------------------------------------------------------

    if (handleHatAPI(strPath, "up", HAT_UP))
    {
        return;
    }

    if (handleHatAPI(strPath, "down", HAT_DOWN))
    {
        return;
    }

    if (handleHatAPI(strPath, "left", HAT_LEFT))
    {
        return;
    }

    if (handleHatAPI(strPath, "right", HAT_RIGHT))
    {
        return;
    }
}


// --------------------------------------------------------------------
// HTTP响应
// --------------------------------------------------------------------

static void sendResponse(
    struct tcp_pcb *pPCB,
    const char *strContentType,
    const char *strBody)
{
    char strHeader[512];

    int nBodyLen = strlen(strBody);

    snprintf(
        strHeader,
        sizeof(strHeader),

        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",

        strContentType,
        nBodyLen
    );

    tcp_write(
        pPCB,
        strHeader,
        strlen(strHeader),
        TCP_WRITE_FLAG_COPY
    );

    tcp_write(
        pPCB,
        strBody,
        nBodyLen,
        TCP_WRITE_FLAG_COPY
    );

    tcp_output(pPCB);
}


// --------------------------------------------------------------------
// HTTP接收
// --------------------------------------------------------------------

static err_t httpRecvCallback(
    void *pArg,
    struct tcp_pcb *pPCB,
    struct pbuf *pBuffer,
    err_t err)
{
    (void)pArg;

    if (pBuffer == NULL)
    {
        tcp_close(pPCB);

        return ERR_OK;
    }

    if (err != ERR_OK)
    {
        pbuf_free(pBuffer);

        tcp_close(pPCB);

        return err;
    }

    char strRequest[1024];

    uint16_t nCopyLen = pBuffer->tot_len;

    if (nCopyLen >= sizeof(strRequest))
    {
        nCopyLen = sizeof(strRequest) - 1;
    }

    pbuf_copy_partial(
        pBuffer,
        strRequest,
        nCopyLen,
        0
    );

    strRequest[nCopyLen] = '\0';

    tcp_recved(
        pPCB,
        pBuffer->tot_len
    );

    pbuf_free(pBuffer);


    char strMethod[16];
    char strPath[256];

    memset(strMethod, 0, sizeof(strMethod));
    memset(strPath, 0, sizeof(strPath));

    sscanf(
        strRequest,
        "%15s %255s",
        strMethod,
        strPath
    );


    if (strcmp(strMethod, "GET") != 0)
    {
        sendResponse(
            pPCB,
            "text/plain",
            "Method Not Allowed"
        );

        tcp_close(pPCB);

        return ERR_OK;
    }


    if (strncmp(strPath, "/api/", 5) == 0)
    {
        handleAPI(strPath);

        sendResponse(
            pPCB,
            "application/json",
            "{\"code\":0}"
        );
    }
    else
    {
        sendResponse(
            pPCB,
            "text/plain",
            "Switch2 Controller API"
        );
    }


    tcp_close(pPCB);

    return ERR_OK;
}


// --------------------------------------------------------------------
// Accept
// --------------------------------------------------------------------

static err_t httpAcceptCallback(
    void *pArg,
    struct tcp_pcb *pClientPCB,
    err_t err)
{
    (void)pArg;

    if (err != ERR_OK || pClientPCB == NULL)
    {
        return ERR_VAL;
    }

    tcp_recv(
        pClientPCB,
        httpRecvCallback
    );

    return ERR_OK;
}


// --------------------------------------------------------------------
// Init
// --------------------------------------------------------------------

bool httpServerInit(void)
{
    gpServer = tcp_new_ip_type(
        IPADDR_TYPE_ANY
    );

    if (gpServer == NULL)
    {
        return false;
    }


    err_t err = tcp_bind(
        gpServer,
        IP_ANY_TYPE,
        80
    );

    if (err != ERR_OK)
    {
        tcp_close(gpServer);

        gpServer = NULL;

        return false;
    }


    gpServer = tcp_listen_with_backlog(
        gpServer,
        4
    );

    if (gpServer == NULL)
    {
        return false;
    }


    tcp_accept(
        gpServer,
        httpAcceptCallback
    );

    return true;
}


// --------------------------------------------------------------------
// Task
// --------------------------------------------------------------------

void httpServerTask(void)
{
    if (!gbStickClick)
    {
        return;
    }


    uint64_t nNowUS = time_us_64();


    if (nNowUS >= gnStickClickEndUS)
    {
        gnLX = 128;
        gnLY = 128;

        gbStickClick = false;

        gnStickClickEndUS = 0;
    }
}


// --------------------------------------------------------------------
// Getters
// --------------------------------------------------------------------

uint16_t httpServerGetButtons(void)
{
    return gnButtons;
}


uint8_t httpServerGetHat(void)
{
    return gnHat;
}


uint8_t httpServerGetLX(void)
{
    return gnLX;
}


uint8_t httpServerGetLY(void)
{
    return gnLY;
}
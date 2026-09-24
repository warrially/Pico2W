// http_server.c

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "http_server.h"
#include "usb_descriptors.h"

#define CLICK_TIME_MS          80
#define CLICK_TIME_US          (CLICK_TIME_MS * 1000ULL)

#define RELEASE_TIME_MS        50
#define RELEASE_TIME_US        (RELEASE_TIME_MS * 1000ULL)

static struct tcp_pcb *gpServer = NULL;


// --------------------------------------------------------------------
// 持续按下状态
// --------------------------------------------------------------------

static volatile uint16_t gnButtons = 0;
static volatile uint8_t gnHat = HAT_CENTER;
static volatile bool gbCButton = false;


// --------------------------------------------------------------------
// Click状态
// --------------------------------------------------------------------

static volatile uint16_t gnClickButton = 0;
static volatile uint8_t gnClickHat = HAT_CENTER;
static volatile bool gbClickC = false;

static volatile uint64_t gnClickButtonEndUS = 0;
static volatile uint64_t gnClickHatEndUS = 0;
static volatile uint64_t gnClickCEndUS = 0;


// click结束后，强制保持释放一小段时间
static volatile uint64_t gnButtonReleaseEndUS = 0;
static volatile uint64_t gnHatReleaseEndUS = 0;
static volatile uint64_t gnCReleaseEndUS = 0;


// --------------------------------------------------------------------
// 左摇杆
// --------------------------------------------------------------------

static volatile uint8_t gnLX = 128;
static volatile uint8_t gnLY = 128;

static volatile bool gbLeftStickClick = false;

static volatile uint64_t gnLeftStickClickEndUS = 0;
static volatile uint64_t gnLeftStickReleaseEndUS = 0;


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


static void clickButton(uint16_t nButton)
{
    uint64_t nNowUS = time_us_64();

    // 清除这个按钮可能遗留的down状态
    gnButtons &= (uint16_t)~nButton;

    gnClickButton = nButton;

    gnClickButtonEndUS = nNowUS + CLICK_TIME_US;
    gnButtonReleaseEndUS = gnClickButtonEndUS + RELEASE_TIME_US;
}


// --------------------------------------------------------------------
// C Button
// --------------------------------------------------------------------

static void setCButton(bool bPressed)
{
    gbCButton = bPressed;
}


static void clickCButton(void)
{
    uint64_t nNowUS = time_us_64();

    gbCButton = false;

    gbClickC = true;

    gnClickCEndUS = nNowUS + CLICK_TIME_US;
    gnCReleaseEndUS = gnClickCEndUS + RELEASE_TIME_US;
}


// --------------------------------------------------------------------
// D-Pad
// --------------------------------------------------------------------

static void setHat(uint8_t nHat)
{
    gnHat = nHat;
}


static void clickHat(uint8_t nHat)
{
    uint64_t nNowUS = time_us_64();

    // 清除以前可能遗留的方向按下
    gnHat = HAT_CENTER;

    gnClickHat = nHat;

    gnClickHatEndUS = nNowUS + CLICK_TIME_US;
    gnHatReleaseEndUS = gnClickHatEndUS + RELEASE_TIME_US;
}


// --------------------------------------------------------------------
// Left Stick
// --------------------------------------------------------------------

static void setLeftStick(uint8_t nX, uint8_t nY)
{
    gnLX = nX;
    gnLY = nY;

    gbLeftStickClick = false;

    gnLeftStickClickEndUS = 0;
    gnLeftStickReleaseEndUS = 0;
}


static void centerLeftStick(void)
{
    gnLX = 128;
    gnLY = 128;

    gbLeftStickClick = false;

    gnLeftStickClickEndUS = 0;
    gnLeftStickReleaseEndUS = 0;
}


static void clickLeftStick(uint8_t nX, uint8_t nY)
{
    uint64_t nNowUS = time_us_64();

    gnLX = nX;
    gnLY = nY;

    gbLeftStickClick = true;

    gnLeftStickClickEndUS = nNowUS + CLICK_TIME_US;
    gnLeftStickReleaseEndUS = gnLeftStickClickEndUS + RELEASE_TIME_US;
}


// --------------------------------------------------------------------
// Query
// --------------------------------------------------------------------

static int getQueryInt(
    const char *strPath,
    const char *strKey,
    int nDefault)
{
    const char *pQuery = strchr(strPath, '?');

    if (pQuery == NULL)
    {
        return nDefault;
    }

    pQuery++;

    char strSearch[32];

    snprintf(
        strSearch,
        sizeof(strSearch),
        "%s=",
        strKey
    );

    const char *pValue = strstr(
        pQuery,
        strSearch
    );

    if (pValue == NULL)
    {
        return nDefault;
    }

    pValue += strlen(strSearch);

    return atoi(pValue);
}


static uint8_t getQueryUInt8(
    const char *strPath,
    const char *strKey,
    int nDefault)
{
    int nValue = getQueryInt(
        strPath,
        strKey,
        nDefault
    );

    if (nValue < 0)
    {
        nValue = 0;
    }

    if (nValue > 255)
    {
        nValue = 255;
    }

    return (uint8_t)nValue;
}


// --------------------------------------------------------------------
// Button API
// --------------------------------------------------------------------

static bool handleButtonAPI(
    const char *strPath,
    const char *strKey,
    uint16_t nButton)
{
    char strURL[64];

    snprintf(strURL, sizeof(strURL), "/api/%s/down", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        setButton(nButton, true);

        return true;
    }

    snprintf(strURL, sizeof(strURL), "/api/%s/up", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        setButton(nButton, false);

        if (gnClickButton == nButton)
        {
            gnClickButton = 0;
            gnClickButtonEndUS = 0;
            gnButtonReleaseEndUS = 0;
        }

        return true;
    }

    snprintf(strURL, sizeof(strURL), "/api/%s/click", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        clickButton(nButton);

        return true;
    }

    return false;
}


// --------------------------------------------------------------------
// Hat API
// --------------------------------------------------------------------

static bool handleHatAPI(
    const char *strPath,
    const char *strKey,
    uint8_t nHat)
{
    char strURL[64];

    snprintf(strURL, sizeof(strURL), "/api/%s/down", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        setHat(nHat);

        return true;
    }

    snprintf(strURL, sizeof(strURL), "/api/%s/up", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        setHat(HAT_CENTER);

        gnClickHat = HAT_CENTER;
        gnClickHatEndUS = 0;
        gnHatReleaseEndUS = 0;

        return true;
    }

    snprintf(strURL, sizeof(strURL), "/api/%s/click", strKey);

    if (strcmp(strPath, strURL) == 0)
    {
        clickHat(nHat);

        return true;
    }

    return false;
}


// --------------------------------------------------------------------
// API
// --------------------------------------------------------------------

static void handleAPI(const char *strPath)
{
    // ----------------------------------------------------------------
    // 左摇杆
    // ----------------------------------------------------------------

    if (strncmp(strPath, "/api/lstick/set?", 16) == 0)
    {
        uint8_t nX = getQueryUInt8(
            strPath,
            "x",
            128
        );

        uint8_t nY = getQueryUInt8(
            strPath,
            "y",
            128
        );

        setLeftStick(
            nX,
            nY
        );

        return;
    }


    if (strncmp(strPath, "/api/lstick/click?", 18) == 0)
    {
        uint8_t nX = getQueryUInt8(
            strPath,
            "x",
            128
        );

        uint8_t nY = getQueryUInt8(
            strPath,
            "y",
            128
        );

        clickLeftStick(
            nX,
            nY
        );

        return;
    }


    if (strcmp(strPath, "/api/lstick/center") == 0)
    {
        centerLeftStick();

        return;
    }


    // ----------------------------------------------------------------
    // 以下全部保持原来的
    // ----------------------------------------------------------------

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

    if (handleButtonAPI(strPath, "plus", BUTTON_PLUS))
    {
        return;
    }

    if (handleButtonAPI(strPath, "minus", BUTTON_MINUS))
    {
        return;
    }

    if (handleButtonAPI(strPath, "home", BUTTON_HOME))
    {
        return;
    }

    if (handleButtonAPI(strPath, "capture", BUTTON_CAPTURE))
    {
        return;
    }

    if (handleButtonAPI(strPath, "gl", BUTTON_GL))
    {
        return;
    }

    if (handleButtonAPI(strPath, "gr", BUTTON_GR))
    {
        return;
    }


    // 右摇杆按下 R3
    if (handleButtonAPI(strPath, "lstick", BUTTON_RSTICK))
    {
        return;
    }
    // 右摇杆按下 R3
    if (handleButtonAPI(strPath, "rstick", BUTTON_RSTICK))
    {
        return;
    }

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

    if (strcmp(strPath, "/api/c/down") == 0)
    {
        setCButton(true);

        return;
    }

    if (strcmp(strPath, "/api/c/up") == 0)
    {
        setCButton(false);

        gbClickC = false;
        gnClickCEndUS = 0;
        gnCReleaseEndUS = 0;

        return;
    }

    if (strcmp(strPath, "/api/c/click") == 0)
    {
        clickCButton();

        return;
    }
}


// --------------------------------------------------------------------
// HTTP Response
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
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
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

    if (nBodyLen > 0)
    {
        tcp_write(
            pPCB,
            strBody,
            nBodyLen,
            TCP_WRITE_FLAG_COPY
        );
    }

    tcp_output(pPCB);
}


// --------------------------------------------------------------------
// TCP Receive
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

    memset(
        strMethod,
        0,
        sizeof(strMethod)
    );

    memset(
        strPath,
        0,
        sizeof(strPath)
    );

    sscanf(
        strRequest,
        "%15s %255s",
        strMethod,
        strPath
    );

    if (strcmp(strMethod, "OPTIONS") == 0)
    {
        sendResponse(
            pPCB,
            "text/plain",
            ""
        );

        tcp_close(pPCB);

        return ERR_OK;
    }

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
// TCP Accept
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
// Public
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
        tcp_close(
            gpServer
        );

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


void httpServerTask(void)
{
    // 原来的click释放不依赖这里。
}


// --------------------------------------------------------------------
// 获取按钮状态
// --------------------------------------------------------------------

uint16_t httpServerGetButtons(void)
{
    uint64_t nNowUS = time_us_64();

    // click按下阶段
    if (
        gnClickButton != 0 &&
        nNowUS < gnClickButtonEndUS
    )
    {
        return gnButtons | gnClickButton;
    }

    // click按下已经结束
    if (
        gnClickButton != 0 &&
        nNowUS >= gnClickButtonEndUS
    )
    {
        gnClickButton = 0;
    }

    // 强制释放阶段
    if (nNowUS < gnButtonReleaseEndUS)
    {
        return gnButtons;
    }

    return gnButtons;
}


// --------------------------------------------------------------------
// 获取Hat状态
// --------------------------------------------------------------------

uint8_t httpServerGetHat(void)
{
    uint64_t nNowUS = time_us_64();

    uint8_t nResult = gnHat;

    // ------------------------------------------------------------
    // D-Pad click按下阶段
    // ------------------------------------------------------------

    if (
        gnClickHat != HAT_CENTER &&
        nNowUS < gnClickHatEndUS
    )
    {
        nResult = gnClickHat;
    }
    else if (
        gnClickHat != HAT_CENTER &&
        nNowUS >= gnClickHatEndUS
    )
    {
        // 到时间直接清除
        gnClickHat = HAT_CENTER;

        // 强制发CENTER
        nResult = HAT_CENTER;
    }

    // ------------------------------------------------------------
    // click结束后的强制CENTER阶段
    // ------------------------------------------------------------

    if (
        gnHatReleaseEndUS != 0 &&
        nNowUS >= gnClickHatEndUS &&
        nNowUS < gnHatReleaseEndUS
    )
    {
        nResult = HAT_CENTER;
    }

    // ------------------------------------------------------------
    // C
    // ------------------------------------------------------------

    if (
        gbClickC &&
        nNowUS < gnClickCEndUS
    )
    {
        nResult |= BUTTON_C;
    }
    else if (
        gbClickC &&
        nNowUS >= gnClickCEndUS
    )
    {
        gbClickC = false;
    }

    if (
        gnCReleaseEndUS != 0 &&
        nNowUS >= gnClickCEndUS &&
        nNowUS < gnCReleaseEndUS
    )
    {
        nResult &= (uint8_t)~BUTTON_C;
    }
    else if (gbCButton)
    {
        nResult |= BUTTON_C;
    }

    return nResult;
}


// --------------------------------------------------------------------
// 获取左摇杆X
// --------------------------------------------------------------------

uint8_t httpServerGetLX(void)
{
    uint64_t nNowUS = time_us_64();

    if (
        gbLeftStickClick &&
        nNowUS >= gnLeftStickClickEndUS
    )
    {
        gnLX = 128;
        gnLY = 128;

        gbLeftStickClick = false;
    }

    if (
        gnLeftStickReleaseEndUS != 0 &&
        nNowUS >= gnLeftStickClickEndUS &&
        nNowUS < gnLeftStickReleaseEndUS
    )
    {
        return 128;
    }

    return gnLX;
}


// --------------------------------------------------------------------
// 获取左摇杆Y
// --------------------------------------------------------------------

uint8_t httpServerGetLY(void)
{
    uint64_t nNowUS = time_us_64();

    if (
        gbLeftStickClick &&
        nNowUS >= gnLeftStickClickEndUS
    )
    {
        gnLX = 128;
        gnLY = 128;

        gbLeftStickClick = false;
    }

    if (
        gnLeftStickReleaseEndUS != 0 &&
        nNowUS >= gnLeftStickClickEndUS &&
        nNowUS < gnLeftStickReleaseEndUS
    )
    {
        return 128;
    }

    return gnLY;
}
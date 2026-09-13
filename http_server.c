#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#include "http_server.h"
#include "usb_descriptors.h"

#define CLICK_TIME_MS 20

static struct tcp_pcb *gpServer = NULL;

// 持续按键状态
static volatile uint16_t gnButtons = 0;
static volatile uint8_t gnHat = HAT_CENTER;

// 临时click状态
static volatile uint16_t gnClickButtons = 0;
static volatile uint8_t gnClickHat = HAT_CENTER;
static volatile bool gbClickC = false;

static absolute_time_t gtmButtonClickEnd;
static absolute_time_t gtmHatClickEnd;
static absolute_time_t gtmCClickEnd;


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
    gnClickButtons |= nButton;

    gtmButtonClickEnd = make_timeout_time_ms(CLICK_TIME_MS);
}


// --------------------------------------------------------------------
// C Button
// --------------------------------------------------------------------

static void setCButton(bool bPressed)
{
    if (bPressed)
    {
        gnHat |= BUTTON_C;
    }
    else
    {
        gnHat &= (uint8_t)~BUTTON_C;
    }
}


static void clickCButton(void)
{
    gbClickC = true;

    gtmCClickEnd = make_timeout_time_ms(CLICK_TIME_MS);
}


// --------------------------------------------------------------------
// D-Pad
// --------------------------------------------------------------------

static void setHat(uint8_t nHat)
{
    gnHat = (gnHat & 0xF0) | (nHat & 0x0F);
}


static void clickHat(uint8_t nHat)
{
    gnClickHat = nHat;

    gtmHatClickEnd = make_timeout_time_ms(CLICK_TIME_MS);
}


// --------------------------------------------------------------------
// API Button
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
// API Hat
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


void httpServerTask(void)
{
    absolute_time_t tmNow = get_absolute_time();

    if (gnClickButtons != 0)
    {
        if (absolute_time_diff_us(tmNow, gtmButtonClickEnd) <= 0)
        {
            gnClickButtons = 0;
        }
    }

    if (gnClickHat != HAT_CENTER)
    {
        if (absolute_time_diff_us(tmNow, gtmHatClickEnd) <= 0)
        {
            gnClickHat = HAT_CENTER;
        }
    }

    if (gbClickC)
    {
        if (absolute_time_diff_us(tmNow, gtmCClickEnd) <= 0)
        {
            gbClickC = false;
        }
    }
}


uint16_t httpServerGetButtons(void)
{
    return gnButtons | gnClickButtons;
}


uint8_t httpServerGetHat(void)
{
    uint8_t nHat = gnHat;

    if (gnClickHat != HAT_CENTER)
    {
        nHat = (nHat & 0xF0) | (gnClickHat & 0x0F);
    }

    if (gbClickC)
    {
        nHat |= BUTTON_C;
    }

    return nHat;
}
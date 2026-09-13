// http_server.c

#include <stdio.h>
#include <string.h>

#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#include "http_server.h"
#include "usb_descriptors.h"
#include "web_page.h"

static struct tcp_pcb *gpServer = NULL;

static volatile uint16_t gnButtons = 0;
static volatile uint8_t gnHat = HAT_CENTER;


// --------------------------------------------------------------------
// Controller state
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


static void setHat(uint8_t nHat)
{
    gnHat = (gnHat & 0xF0) | (nHat & 0x0F);
}


// --------------------------------------------------------------------
// API
// --------------------------------------------------------------------

static void handleAPI(const char *strPath)
{
    bool bPressed = strstr(strPath, "/down") != NULL;

    if (strncmp(strPath, "/api/a/", 7) == 0)
    {
        setButton(BUTTON_A, bPressed);
    }
    else if (strncmp(strPath, "/api/b/", 7) == 0)
    {
        setButton(BUTTON_B, bPressed);
    }
    else if (strncmp(strPath, "/api/x/", 7) == 0)
    {
        setButton(BUTTON_X, bPressed);
    }
    else if (strncmp(strPath, "/api/y/", 7) == 0)
    {
        setButton(BUTTON_Y, bPressed);
    }
    else if (strncmp(strPath, "/api/l/", 7) == 0)
    {
        setButton(BUTTON_L, bPressed);
    }
    else if (strncmp(strPath, "/api/r/", 7) == 0)
    {
        setButton(BUTTON_R, bPressed);
    }
    else if (strncmp(strPath, "/api/zl/", 8) == 0)
    {
        setButton(BUTTON_ZL, bPressed);
    }
    else if (strncmp(strPath, "/api/zr/", 8) == 0)
    {
        setButton(BUTTON_ZR, bPressed);
    }
    else if (strncmp(strPath, "/api/plus/", 10) == 0)
    {
        setButton(BUTTON_PLUS, bPressed);
    }
    else if (strncmp(strPath, "/api/minus/", 11) == 0)
    {
        setButton(BUTTON_MINUS, bPressed);
    }
    else if (strncmp(strPath, "/api/home/", 10) == 0)
    {
        setButton(BUTTON_HOME, bPressed);
    }
    else if (strncmp(strPath, "/api/capture/", 13) == 0)
    {
        setButton(BUTTON_CAPTURE, bPressed);
    }
    else if (strncmp(strPath, "/api/gl/", 8) == 0)
    {
        setButton(BUTTON_GL, bPressed);
    }
    else if (strncmp(strPath, "/api/gr/", 8) == 0)
    {
        setButton(BUTTON_GR, bPressed);
    }
    else if (strncmp(strPath, "/api/c/", 7) == 0)
    {
        setCButton(bPressed);
    }
    else if (strncmp(strPath, "/api/up/", 8) == 0)
    {
        setHat(bPressed ? HAT_UP : HAT_CENTER);
    }
    else if (strncmp(strPath, "/api/down/", 10) == 0)
    {
        setHat(bPressed ? HAT_DOWN : HAT_CENTER);
    }
    else if (strncmp(strPath, "/api/left/", 10) == 0)
    {
        setHat(bPressed ? HAT_LEFT : HAT_CENTER);
    }
    else if (strncmp(strPath, "/api/right/", 11) == 0)
    {
        setHat(bPressed ? HAT_RIGHT : HAT_CENTER);
    }
}


// --------------------------------------------------------------------
// HTTP response
// --------------------------------------------------------------------

static void sendResponse(struct tcp_pcb *pPCB, const char *strContentType, const char *strBody)
{
    char strHeader[256];

    int nBodyLen = strlen(strBody);

    snprintf(
        strHeader,
        sizeof(strHeader),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n",
        strContentType,
        nBodyLen
    );

    tcp_write(pPCB, strHeader, strlen(strHeader), TCP_WRITE_FLAG_COPY);
    tcp_write(pPCB, strBody, nBodyLen, TCP_WRITE_FLAG_COPY);
    tcp_output(pPCB);
}


// --------------------------------------------------------------------
// TCP callbacks
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

    pbuf_copy_partial(pBuffer, strRequest, nCopyLen, 0);

    strRequest[nCopyLen] = '\0';

    tcp_recved(pPCB, pBuffer->tot_len);

    pbuf_free(pBuffer);

    char strMethod[16];
    char strPath[256];

    memset(strMethod, 0, sizeof(strMethod));
    memset(strPath, 0, sizeof(strPath));

    sscanf(strRequest, "%15s %255s", strMethod, strPath);

    if (strcmp(strMethod, "GET") != 0)
    {
        sendResponse(pPCB, "text/plain", "Method Not Allowed");
        tcp_close(pPCB);
        return ERR_OK;
    }

    if (strncmp(strPath, "/api/", 5) == 0)
    {
        handleAPI(strPath);

        sendResponse(pPCB, "application/json", "{\"code\":0}");
    }
    else
    {
        sendResponse(pPCB, "text/html; charset=utf-8", gstrWebPage);
    }

    tcp_close(pPCB);

    return ERR_OK;
}


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

    tcp_recv(pClientPCB, httpRecvCallback);

    return ERR_OK;
}


// --------------------------------------------------------------------
// Public
// --------------------------------------------------------------------

bool httpServerInit(void)
{
    gpServer = tcp_new_ip_type(IPADDR_TYPE_ANY);

    if (gpServer == NULL)
    {
        return false;
    }

    err_t err = tcp_bind(gpServer, IP_ANY_TYPE, 80);

    if (err != ERR_OK)
    {
        tcp_close(gpServer);
        gpServer = NULL;

        return false;
    }

    gpServer = tcp_listen_with_backlog(gpServer, 4);

    if (gpServer == NULL)
    {
        return false;
    }

    tcp_accept(gpServer, httpAcceptCallback);

    return true;
}


void httpServerTask(void)
{
}


uint16_t httpServerGetButtons(void)
{
    return gnButtons;
}


uint8_t httpServerGetHat(void)
{
    return gnHat;
}
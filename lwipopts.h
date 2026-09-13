#ifndef _LWIPOPTS_H_
#define _LWIPOPTS_H_

#define NO_SYS                          1

#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

#define LWIP_IPV4                       1
#define LWIP_IPV6                       0

#define LWIP_TCP                        1
#define LWIP_UDP                        1
#define LWIP_DHCP                       1
#define LWIP_DNS                        1

#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        16000

#define MEMP_NUM_PBUF                   16
#define MEMP_NUM_TCP_PCB                8
#define MEMP_NUM_TCP_PCB_LISTEN         4
#define MEMP_NUM_TCP_SEG                32

#define PBUF_POOL_SIZE                  24
#define PBUF_POOL_BUFSIZE               1536

#define TCP_MSS                         1460
#define TCP_WND                         (8 * TCP_MSS)
#define TCP_SND_BUF                     (8 * TCP_MSS)
#define TCP_SND_QUEUELEN                16

#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1

#define LWIP_TCP_KEEPALIVE              1

#define LWIP_STATS                      0
#define LWIP_STATS_DISPLAY              0

#define LWIP_CHKSUM_ALGORITHM           3

#define CYW43_HOST_NAME                 "switch2"

#endif
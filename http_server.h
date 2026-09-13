// http_server.h

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include <stdbool.h>
#include <stdint.h>

bool httpServerInit(void);
void httpServerTask(void);

uint16_t httpServerGetButtons(void);
uint8_t httpServerGetHat(void);

#endif
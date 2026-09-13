// wifi.h

#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdbool.h>

bool wifiInit(const char *strSSID, const char *strPassword);
const char *wifiGetIP(void);

#endif
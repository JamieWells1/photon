#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_FAILED
} WifiState;

int wifi_join(const char *ssid, const char *password);
int wifi_join_async(const char *ssid, const char *password);
WifiState wifi_get_state(void);
void wifi_disconnect(void);

#endif  // WIFI_H

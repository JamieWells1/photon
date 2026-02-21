#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stdint.h>

int wifi_join(const char *ssid, const char *password);

#endif  // WIFI_H

#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Ensure WiFi is connected (lazy initialization)
 * Call this when you need WiFi. It handles everything.
 * @param ssid The network SSID
 * @param password The network password
 * @return 0 on success, non-zero on failure
 */
int wifi_ensure_connected(const char *ssid, const char *password);

#endif  // WIFI_H

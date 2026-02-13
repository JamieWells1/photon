#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the WiFi hardware (CYW43 chip)
 * @return 0 on success, -1 on failure
 */
int wifi_init(void);

/**
 * Connect to a WiFi network
 * @param ssid The network SSID
 * @param password The network password
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, 1 on failure
 */
int wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms);

/**
 * Get the current IP address
 * @param ip_out Buffer to store IP address (4 bytes)
 */
void wifi_get_ip(uint8_t *ip_out);

/**
 * Enable WiFi station mode
 */
void wifi_enable_sta_mode(void);

#endif  // WIFI_H

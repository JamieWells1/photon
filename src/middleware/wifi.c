/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <wifi.h>

#include <const.h>
#include <error.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <macro.h>

#include <stdio.h>
#include <string.h>

#include <dhcpserver.h>
#include <lwip/apps/mdns.h>
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <pico/cyw43_arch.h>
#include <pico/time.h>

static bool initialized = false;
static bool connected = false;
static bool ap_mode = false;
static bool mdns_initialized = false;
static dhcp_server_t dhcp_server;

static ReturnCode wifi_init(void)
{
    if (initialized) return RC_OK;

    if (cyw43_arch_init())
    {
        debug(ERROR, "Wi-Fi init failed");
        return RC_WIFI_INIT_FAILED;
    }

    debug(INFO, "Wi-Fi initialized");
    initialized = true;
    return RC_OK;
}

static void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    debug(INFO, "Wi-Fi station mode enabled");
}

static void wifi_display_status(Matrix* mtrx, int dot_counter)
{
    char status[256] = "WIFI";

    size_t len = strlen(status);
    int num_dots = (dot_counter % 3) + 1;
    for (int i = 0; i < num_dots && len + i + 1 < sizeof(status); i++)
    {
        status[len + i] = '.';
        status[len + i + 1] = '\0';
    }

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(status, &DEFAULT_COLOUR, &ICONS_ARR[WIFI], 0);
    matrix_show(mtrx);
}

ReturnCode wifi_connect(const char* ssid, const char* password, Matrix* mtrx)
{
    if (connected)
    {
        debug(DEBUG, "Already connected to WiFi");
        return RC_OK;
    }

    if (!initialized)
    {
        ReturnCode rc = wifi_init();
        if (RC_FAILED(rc)) return rc;
        wifi_enable_sta_mode();
    }

    debug(DEBUG, "Starting WiFi connection...");

    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    int dot_counter = 0;
    int attempts = 0;
    int display_frame_counter = 0;

    while (attempts < MAX_WIFI_CONNECTION_ATTEMPTS)
    {
        if (display_frame_counter % 4 == 0)
        {
            wifi_display_status(mtrx, dot_counter);
            dot_counter++;
        }
        display_frame_counter++;

        int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (link_status == CYW43_LINK_UP)
        {
            debug(INFO, "WiFi connected successfully");
            connected = true;

            // Get and log IP address
            const ip4_addr_t* ip = netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]);
            debug(INFO, "IP Address: %s", ip4addr_ntoa(ip));
            debug(INFO, "Gateway: %s",
                  ip4addr_ntoa(netif_ip4_gw(&cyw43_state.netif[CYW43_ITF_STA])));
            debug(INFO, "Netmask: %s",
                  ip4addr_ntoa(netif_ip4_netmask(&cyw43_state.netif[CYW43_ITF_STA])));
            debug(INFO, "To access web interface, navigate to CONFIG menu on device");

            return RC_OK;
        }
        else if (link_status == CYW43_LINK_BADAUTH)
        {
            debug(ERROR, "WiFi bad authentication (link_status=%d)", link_status);
            connected = false;
            return RC_WIFI_BAD_AUTH;
        }
        else if (link_status == CYW43_LINK_NONET)
        {
            debug(ERROR, "WiFi network not found (link_status=%d)", link_status);
            connected = false;
            return RC_WIFI_NO_NETWORK;
        }
        else if (link_status == CYW43_LINK_FAIL)
        {
            debug(ERROR, "WiFi connection failed (link_status=%d)", link_status);
            connected = false;
            return RC_WIFI_CONNECT_FAILED;
        }

        attempts++;
        sleep_ms(100);
    }

    debug(ERROR, "WiFi connection timeout");
    connected = false;
    return RC_WIFI_TIMEOUT;
}

void wifi_disconnect(void)
{
    if (!connected)
    {
        debug(DEBUG, "WiFi already disconnected");
        return;
    }

    cyw43_arch_deinit();
    connected = false;
    initialized = false;
    mdns_initialized = false;  // Reset mDNS state since we deinit the network stack
    debug(INFO, "WiFi disconnected");
}

ReturnCode wifi_init_ap(const char* ssid, const char* password, Matrix* mtrx)
{
    if (!initialized)
    {
        ReturnCode rc = wifi_init();
        if (RC_FAILED(rc)) return rc;
    }

    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_OPEN);
    ap_mode = true;

    debug(INFO, "WiFi AP mode enabled: %s", ssid);

    ip_addr_t gw, mask;
    IP4_ADDR(ip_2_ip4(&gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    dhcp_server_init(&dhcp_server, &gw, &mask);

    debug(INFO, "DHCP server started at 192.168.4.1");

    if (mtrx)
    {
        matrix_clear(mtrx);
        matrix_display_word_icon_pair("START...", &DEFAULT_COLOUR, NULL, 0);
        matrix_show(mtrx);
    }

    return RC_OK;
}

bool wifi_is_connected(void) { return connected; }

void wifi_start_mdns(void)
{
    if (!connected)
    {
        debug(WARN, "Cannot start mDNS: WiFi not connected");
        return;
    }

    if (mdns_initialized)
    {
        debug(DEBUG, "mDNS already initialized");
        return;
    }

    debug(INFO, "Starting mDNS responder...");

    // Give network stack extra time to settle
    sleep_ms(1000);

    // Initialize mDNS
    mdns_resp_init();
    mdns_initialized = true;

    // Add this interface to mDNS
    err_t err = mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], "delta");

    const ip4_addr_t* ip = netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]);

    if (err == ERR_OK)
    {
        debug(INFO, "mDNS responder started: delta.local resolves to %s", ip4addr_ntoa(ip));
    }
    else
    {
        debug(WARN, "mDNS failed to bind (error: %d). Use IP address: %s", err, ip4addr_ntoa(ip));
    }
}

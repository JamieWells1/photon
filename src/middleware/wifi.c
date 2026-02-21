#include <wifi.h>

#include <macro.h>

#include <stdio.h>

#include <pico/cyw43_arch.h>

static bool initialized = false;
static bool connected = false;
static WifiState current_state = WIFI_DISCONNECTED;

static int wifi_init(void)
{
    if (initialized)
    {
        return 0;
    }

    if (cyw43_arch_init())
    {
        debug("Wi-Fi init failed");
        return -1;
    }

    debug("Wi-Fi initialized");
    initialized = true;
    return 0;
}

static void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    debug("Wi-Fi station mode enabled");
}

static int wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    debug("Connecting to Wi-Fi...");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms))
    {
        debug("Failed to connect to Wi-Fi");
        connected = false;
        return 1;
    }

    debug("Connected to Wi-Fi");
    connected = true;
    return 0;
}

void wifi_loading_display()
{
    //
}

int wifi_join(const char *ssid, const char *password)
{
    if (connected)
    {
        current_state = WIFI_CONNECTED;
        return 0;
    }

    if (!initialized)
    {
        if (wifi_init() != 0)
        {
            current_state = WIFI_FAILED;
            return -1;
        }
        wifi_enable_sta_mode();
    }

    current_state = WIFI_CONNECTING;
    int result = wifi_connect(ssid, password, 30000);
    current_state = (result == 0) ? WIFI_CONNECTED : WIFI_FAILED;
    return result;
}

int wifi_join_async(const char *ssid, const char *password)
{
    if (connected)
    {
        current_state = WIFI_CONNECTED;
        return 0;
    }

    if (current_state == WIFI_CONNECTING)
    {
        int link_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);

        if (link_status == CYW43_LINK_UP)
        {
            debug("WiFi connected!");
            connected = true;
            current_state = WIFI_CONNECTED;
            return 0;
        }
        else if (link_status == CYW43_LINK_FAIL || link_status == CYW43_LINK_BADAUTH)
        {
            debug("WiFi connection failed");
            connected = false;
            current_state = WIFI_FAILED;
            return -1;
        }

        // Still connecting
        return 1;
    }

    if (!initialized)
    {
        if (wifi_init() != 0)
        {
            current_state = WIFI_FAILED;
            return -1;
        }
        wifi_enable_sta_mode();
    }

    debug("Starting async WiFi connection...");
    current_state = WIFI_CONNECTING;

    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    // Still connecting
    return 1;
}

WifiState wifi_get_state(void) { return current_state; }

void wifi_disconnect(void)
{
    if (!connected)
    {
        debug("WiFi already disconnected");
        return;
    }

    debug("Disconnecting WiFi to save power...");
    cyw43_arch_deinit();
    connected = false;
    initialized = false;
    debug("WiFi disconnected");
}

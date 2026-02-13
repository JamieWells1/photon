#include <wifi.h>

#include <stdio.h>

#include <pico/cyw43_arch.h>

// Private state
static bool initialized = false;
static bool connected = false;

// Private functions (static = not visible outside this file)
static int wifi_init(void)
{
    if (initialized)
    {
        return 0;
    }

    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    printf("Wi-Fi initialized\n");
    initialized = true;
    return 0;
}

static void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    printf("Wi-Fi station mode enabled\n");
}

static int wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms))
    {
        printf("Failed to connect to Wi-Fi\n");
        connected = false;
        return 1;
    }

    printf("Connected to Wi-Fi\n");
    connected = true;
    return 0;
}

// Public function - the only one users should call
int wifi_ensure_connected(const char *ssid, const char *password)
{
    if (connected)
    {
        return 0;  // Already connected
    }

    if (!initialized)
    {
        if (wifi_init() != 0)
        {
            return -1;
        }
        wifi_enable_sta_mode();
    }

    return wifi_connect(ssid, password, 30000);
}

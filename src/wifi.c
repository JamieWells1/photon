#include <wifi.h>
#include "pico/cyw43_arch.h"
#include <stdio.h>

int wifi_init(void)
{
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    printf("Wi-Fi initialized\n");
    return 0;
}

void wifi_enable_sta_mode(void)
{
    cyw43_arch_enable_sta_mode();
    printf("Wi-Fi station mode enabled\n");
}

int wifi_connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    printf("Connecting to Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password,
                                           CYW43_AUTH_WPA2_AES_PSK,
                                           timeout_ms)) {
        printf("Failed to connect to Wi-Fi\n");
        return 1;
    }

    printf("Connected to Wi-Fi\n");
    return 0;
}

void wifi_get_ip(uint8_t *ip_out)
{
    uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    for (int i = 0; i < 4; i++) {
        ip_out[i] = ip_address[i];
    }
}

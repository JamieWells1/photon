#include <stdio.h>
#include <pico/stdlib.h>
#include <wifi.h>
#include <led.h>

// Configuration
#define WIFI_SSID "Your Wi-Fi SSID"
#define WIFI_PASSWORD "Your Wi-Fi Password"
#define WIFI_TIMEOUT_MS 30000
#define LED_PIN 6
#define LED_FREQ_HZ 3

int main()
{
    // Initialize standard I/O
    stdio_init_all();

    // Initialize WiFi hardware
    if (wifi_init() != 0) {
        return -1;
    }

    // Initialize LED system
    led_init();

    // Start blinking LED
#ifdef PICO_DEFAULT_LED_PIN
    printf("Using PICO_DEFAULT_LED_PIN: %d\n", PICO_DEFAULT_LED_PIN);
    led_blink_start(PICO_DEFAULT_LED_PIN, LED_FREQ_HZ);
#else
    printf("PICO_DEFAULT_LED_PIN not defined, using pin %d\n", LED_PIN);
    led_blink_start(LED_PIN, LED_FREQ_HZ);
#endif

    // Enable WiFi station mode
    wifi_enable_sta_mode();

    // Connect to WiFi
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS) != 0) {
        return 1;
    }

    // Get and display IP address
    uint8_t ip[4];
    wifi_get_ip(ip);
    printf("IP address %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

    // Main loop
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }

    return 0;
}

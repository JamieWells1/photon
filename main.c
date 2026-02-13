#include <led.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <wifi.h>

// Configuration
#define WIFI_SSID "Your Wi-Fi SSID"
#define WIFI_PASSWORD "Your Wi-Fi Password"
#define WIFI_TIMEOUT_MS 30000
#define LED_PIN 6   // GPIO pin connected to WS2812 data line
#define NUM_LEDS 1  // Number of LEDs in your strip

int main()
{
    // Initialize standard I/O
    stdio_init_all();

    printf("Starting WS2812 demo...\n");

    // Initialize WS2812 LEDs
    led_init(LED_PIN, NUM_LEDS);

    // Light up the first pixel - RED
    printf("Setting pixel 0 to RED\n");
    led_set_pixel(0, 255, 0, 0);  // Red
    led_show();

    sleep_ms(2000);

    // Change to GREEN
    printf("Setting pixel 0 to GREEN\n");
    led_set_pixel(0, 0, 255, 0);  // Green
    led_show();

    sleep_ms(2000);

    // Change to BLUE
    printf("Setting pixel 0 to BLUE\n");
    led_set_pixel(0, 0, 0, 255);  // Blue
    led_show();

    sleep_ms(2000);

    // Initialize WiFi hardware
    if (wifi_init() != 0)
    {
        return -1;
    }

    // Enable WiFi station mode
    wifi_enable_sta_mode();

    // Connect to WiFi
    if (wifi_connect(WIFI_SSID, WIFI_PASSWORD, WIFI_TIMEOUT_MS) != 0)
    {
        return 1;
    }

    // Get and display IP address
    uint8_t ip[4];
    wifi_get_ip(ip);
    printf("IP address %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

    // Main loop - cycle through colors
    uint8_t hue = 0;
    while (true)
    {
        // Simple rainbow effect
        if (hue < 85)
        {
            led_set_pixel(0, 255 - hue * 3, hue * 3, 0);
        }
        else if (hue < 170)
        {
            led_set_pixel(0, 0, 255 - (hue - 85) * 3, (hue - 85) * 3);
        }
        else
        {
            led_set_pixel(0, (hue - 170) * 3, 0, 255 - (hue - 170) * 3);
        }
        led_show();

        hue++;
        sleep_ms(20);
    }

    return 0;
}

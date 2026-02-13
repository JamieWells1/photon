#include <led.h>
#include <wifi.h>

#include <stdio.h>

#include <pico/stdlib.h>

// Configuration
#define WIFI_SSID "Your Wi-Fi SSID"
#define WIFI_PASSWORD "Your Wi-Fi Password"
#define LED_PIN 6
#define NUM_LEDS 1

int main()
{
    stdio_init_all();
    printf("Starting...\n");

    // Initialize LEDs - instant!
    led_init(LED_PIN, NUM_LEDS);

    // Demo: Light up pixel
    led_set_pixel(0, 255, 0, 0);  // Red
    led_show();

    // Game runs immediately - NO WiFi delay!
    printf("Playing Tetris...\n");

    // Only connect when user requests online feature
    // For example, when they press a button:
    sleep_ms(3000);  // Simulate playing for 3 seconds

    printf("User wants leaderboard, connecting to WiFi...\n");
    if (wifi_ensure_connected(WIFI_SSID, WIFI_PASSWORD) == 0)
    {
        printf("WiFi ready! Show leaderboard\n");
    }

    // Main game loop
    uint8_t hue = 0;
    while (true)
    {
        // Rainbow animation
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
}

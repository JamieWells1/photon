#ifndef LED_H
#define LED_H

#include <stdint.h>

#include <pico/types.h>

/**
 * Initialize the WS2812 LED system
 * @param pin GPIO pin connected to WS2812 data line
 * @param num_leds Number of LEDs in the strip
 */
void led_init(uint pin, uint num_leds);

/**
 * Set a single pixel color (RGB)
 * @param index Pixel index (0-based)
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 */
void led_set_pixel(uint index, uint8_t r, uint8_t g, uint8_t b);

/**
 * Set a pixel using a 32-bit color value
 * @param index Pixel index (0-based)
 * @param color RGB color as 0xRRGGBB
 */
void led_set_pixel_u32(uint index, uint32_t color);

/**
 * Update the LED strip with current pixel data
 */
void led_show(void);

/**
 * Clear all LEDs (turn them off)
 */
void led_clear(void);

/**
 * Create a 32-bit RGB color value
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 * @return 32-bit color value
 */
static inline uint32_t led_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

#endif  // LED_H

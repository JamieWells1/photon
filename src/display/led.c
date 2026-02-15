#include <led.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/pio.h>
#include <pico/stdlib.h>
#include <ws2812.pio.h>

static PIO pio = NULL;
static uint sm = 0;
static uint32_t *pixel_buffer = NULL;
static uint num_pixels = 0;

void led_init(uint pin, uint num_leds)
{
    pio = pio0;
    sm = 0;
    num_pixels = num_leds;

    pixel_buffer = calloc(num_leds, sizeof(uint32_t));
    if (!pixel_buffer)
    {
        printf("Failed to allocate LED buffer\n");
        return;
    }

    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000, false);

    printf("WS2812 initialized on pin %d with %d LEDs\n", pin, num_leds);
}

void led_set_pixel(uint index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= num_pixels || !pixel_buffer)
    {
        return;
    }

    // WS2812 expects GRB format
    pixel_buffer[index] = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

void led_set_pixel_u32(uint index, uint32_t color)
{
    if (index >= num_pixels || !pixel_buffer)
    {
        return;
    }

    // Convert RGB to GRB
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    pixel_buffer[index] = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

void led_show(void)
{
    if (!pixel_buffer || !pio)
    {
        return;
    }

    for (uint i = 0; i < num_pixels; i++)
    {
        pio_sm_put_blocking(pio, sm, pixel_buffer[i] << 8);
    }
}

void led_clear(void)
{
    if (!pixel_buffer)
    {
        return;
    }

    memset(pixel_buffer, 0, num_pixels * sizeof(uint32_t));
    led_show();
}

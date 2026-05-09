/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <power.h>

#include <const.h>
#include <macro.h>
#include <matrix.h>

#include <stdio.h>

#include <glyphs.h>
#include <hardware/adc.h>
#include <pico/time.h>

// Based on WS2812B datasheet: ~18mA per R/G/B channel at 100%, ~1mA controller overhead
#define LED_CURRENT_MA_PER_CHANNEL 18  // mA per color channel at 100% brightness
#define LED_IDLE_CURRENT_MA 1          // mA per LED when off (controller chip)
#define PICO_IDLE_CURRENT_MA 100       // Pico + WiFi baseline current

static uint16_t power_estimate_current_draw_ma(LogLevel log_level, uint8_t brightness_pct)
{
    uint32_t total_brightness = 0;
    uint16_t active_pixels = 0;

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        Pixel pixel;
        if (matrix_get_pixel_state(i, &pixel))
        {
            total_brightness += pixel.col.r;
            total_brightness += pixel.col.g;
            total_brightness += pixel.col.b;
            active_pixels++;
        }
    }

    // Each channel: 0-255, scale by global brightness
    // Formula: (sum_of_all_channels / 255) * brightness_pct * mA_per_channel
    uint32_t led_active_current =
        (total_brightness * brightness_pct * LED_CURRENT_MA_PER_CHANNEL) / (255 * 100);

    // Add 1mA per LED for controller chip overhead (even when off)
    uint32_t led_idle_current = NUM_PIXELS * LED_IDLE_CURRENT_MA;

    print(log_level, "Active pixels: %d/%d (%.1f%%)", active_pixels, NUM_PIXELS,
          (active_pixels * 100.0f) / NUM_PIXELS);
    print(log_level, "Total brightness sum: %u", total_brightness);

    return (uint16_t)(PICO_IDLE_CURRENT_MA + led_active_current + led_idle_current);
}

void power_log_stats(LogLevel log_level, uint8_t brightness_pct)
{
    // Note: VSYS voltage reading disabled - conflicts with WiFi on Pico W/Pico 2 W
    // Assume nominal 5V for power calculation
    const float nominal_voltage = 5.0f;

    uint16_t estimated_ma = power_estimate_current_draw_ma(log_level, brightness_pct);
    float power_watts = (estimated_ma * nominal_voltage) / 1000.0f;

    print(log_level, "Draw: %dmA | Power: %.2fW @ 5V", estimated_ma, power_watts);

    if (estimated_ma > 2000)
    {
        print(WARN, "⚠️  Estimated current (%dmA) exceeds USB safe limit (2000mA)", estimated_ma);
    }
}

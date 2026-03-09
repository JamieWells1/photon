/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <const.h>
#include <crypto.h>
#include <dns_server.h>
#include <error.h>
#include <flash_memory.h>
#include <global.h>
#include <http_server.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <menu_main.h>
#include <power.h>
#include <wifi.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <ws2812.pio.h>

void debug_inputs(Button* btns, Rotator* rtr)
{
    if (input_btn_pressed(&btns[0]))
    {
        debug(DEBUG, "Left button pressed");
    }
    if (input_btn_pressed(&btns[1]))
    {
        debug(DEBUG, "Right button pressed");
    }
    if (input_btn_released(&btns[0]))
    {
        debug(DEBUG, "Left button released");
    }
    if (input_btn_released(&btns[1]))
    {
        debug(DEBUG, "Right button released");
    }
    if (rtr->rotation_count > 0)
    {
        debug(DEBUG, "Rotator clockwise (count: %d)", rtr->rotation_count);
    }
    if (rtr->rotation_count < 0)
    {
        debug(DEBUG, "Rotator anti-clockwise (count: %d)", rtr->rotation_count);
    }
    if (input_rtr_pressed(rtr))
    {
        debug(DEBUG, "Rotator pressed");
    }
    if (input_rtr_released(rtr))
    {
        debug(DEBUG, "Rotator released");
    }
}

Button* main_init_buttons(void)
{
    static Button buttons[2] = {{.pin = BUTTON_LEFT_PIN}, {.pin = BUTTON_RIGHT_PIN}};

    gpio_init(buttons[0].pin);
    gpio_set_dir(buttons[0].pin, GPIO_IN);
    gpio_pull_up(buttons[0].pin);

    gpio_init(buttons[1].pin);
    gpio_set_dir(buttons[1].pin, GPIO_IN);
    gpio_pull_up(buttons[1].pin);

    buttons[0].current_state = gpio_get(buttons[0].pin);
    buttons[0].last_state = buttons[0].current_state;

    buttons[1].current_state = gpio_get(buttons[1].pin);
    buttons[1].last_state = buttons[1].current_state;

    debug(INFO, "✓ GPIO pins for buttons initialised");

    return buttons;
}

Rotator* main_init_rotator(void)
{
    static Rotator rotator = {
        .pin_clk = ROTATOR_CLK_PIN, .pin_dt = ROTATOR_DT_PIN, .pin_sw = ROTATOR_SW_PIN};

    gpio_init(ROTATOR_CLK_PIN);
    gpio_set_dir(ROTATOR_CLK_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_CLK_PIN);

    gpio_init(ROTATOR_DT_PIN);
    gpio_set_dir(ROTATOR_DT_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_DT_PIN);

    gpio_init(ROTATOR_SW_PIN);
    gpio_set_dir(ROTATOR_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_SW_PIN);

    rotator.current_clk_state = gpio_get(ROTATOR_CLK_PIN);
    rotator.last_clk_state = rotator.current_clk_state;

    rotator.current_dt_state = gpio_get(ROTATOR_DT_PIN);

    rotator.current_sw_state = gpio_get(ROTATOR_SW_PIN);
    rotator.last_sw_state = rotator.current_sw_state;

    rotator.state = (rotator.current_clk_state << 1) | rotator.current_dt_state;
    rotator.rotation_count = 0;
    rotator.last_change_time = time_us_32();

    debug(INFO, "✓ GPIO pins for rotary encoder initialised");

    return &rotator;
}

Matrix* main_init_matrix(void)
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, MATRIX_DIN_PIN, MATRIX_DATA_FREQ, false);
    debug(INFO, "✓ WS2812 program initialised");

    static Matrix matrix = {.pin = MATRIX_DIN_PIN, .pio = pio0, .sm = 0};
    return &matrix;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(2000);

    int usb_wait_count = 0;
    while (!stdio_usb_connected() && usb_wait_count < 50)
    {
        sleep_ms(100);
        usb_wait_count++;
    }
    sleep_ms(100);

    debug(INFO, "Debug level: %u", LOG_LEVEL);

    Button* buttons = main_init_buttons();
    Rotator* rotator = main_init_rotator();
    Matrix* mtrx = main_init_matrix();

    matrix_clear(mtrx);
    matrix_show(mtrx);

    bool usb_host = stdio_usb_connected();
    uint8_t max_brightness = usb_host ? MAX_BRIGHTNESS_USB : MAX_BRIGHTNESS_MAINS;
    matrix_set_max_brightness(max_brightness);
    debug(INFO, "Power source: %s (max brightness: %u%%)", usb_host ? "USB" : "mains",
          max_brightness);

    debug(INFO, "✓✓✓ Main init complete. Starting main execution loop.");

    device_config_t config;
    bool wifi_ready = false;
    bool config_found = flash_memory_read(&config);
    matrix_set_brightness(config.brightness);
    crypto_set_enabled_mask(config.crypto);

    if (config_found)
    {
        debug(INFO, "Flash config found, attempting to connect to WiFi");
        ReturnCode rc = wifi_connect(config.ssid, config.password, mtrx);
        if (RC_SUCCESS(rc))
        {
            debug(INFO, "WiFi connected successfully");
            debug(INFO, "To access web interface:");
            debug(INFO, "  1. Use rotary encoder to navigate to CONFIG menu");
            debug(INFO, "  2. Press button to enter CONFIG mode");
            debug(INFO, "  3. Access http://photon.local in browser");
            wifi_ready = true;
        }
        else
        {
            debug(WARN, "WiFi connection failed: %s", rc_to_string(rc));
            matrix_clear(mtrx);
            matrix_display_word_icon_pair("FAIL", &RED, NULL, 0);
            matrix_show(mtrx);
            sleep_ms(2000);
            flash_memory_erase();
        }
    }
    else
    {
        debug(INFO, "No flash config found");
    }

    if (!wifi_ready)
    {
        debug(INFO, "No valid WiFi config, entering setup mode");

        wifi_init_ap("Photon-Setup", NULL, mtrx);
        sleep_ms(500);

        dns_server_start();
        http_server_start();

        for (int i = 0; i < 100; i++)
        {
            cyw43_arch_poll();
            sleep_ms(10);
        }

        debug(INFO, "Captive portal active, waiting for credentials");

        bool blink_state = true;
        int tick = 0;

        while (!http_server_credentials_saved())
        {
            if (tick % 50 == 0)
            {
                matrix_clear(mtrx);
                if (blink_state)
                {
                    matrix_display_word_icon_pair("ENTER", &ORANGE, &ICONS_ARR[WIFI], 0);
                }
                else
                {
                    matrix_display_word_icon_pair("WIFI", &ORANGE, &ICONS_ARR[WIFI], 0);
                }
                matrix_show(mtrx);
                blink_state = !blink_state;
            }

            cyw43_arch_poll();
            sleep_ms(10);
            tick++;
        }

        debug(INFO, "Credentials saved");
        matrix_clear(mtrx);
        matrix_show(mtrx);
        sleep_ms(200);

        matrix_display_word_icon_pair("WAIT", &ORANGE, &ICONS_ARR[WIFI], 0);
        matrix_show(mtrx);
        sleep_ms(1000);

        debug(INFO, "Rebooting");
        matrix_clear(mtrx);
        matrix_show(mtrx);
        sleep_ms(100);

        matrix_display_word_icon_pair("SAVED", &GREEN, NULL, 0);
        matrix_show(mtrx);
        sleep_ms(2000);

        watchdog_enable(1, 1);
        while (1);
    }

    bool menu_active = false;
    static uint32_t power_log_tick = 0;
    while (true)
    {
        input_update(buttons, rotator);
        if (LOG_LEVEL >= DEBUG)
        {
            debug_inputs(buttons, rotator);
        }

        if (!menu_active)
        {
            matrix_display_start_screen(mtrx);

            bool all_released = false;
            int wait_count = 0;
            int max_wait = 500;

            while (!all_released && wait_count < max_wait)
            {
                input_update(buttons, rotator);
                all_released =
                    (buttons[0].current_state == true && buttons[1].current_state == true &&
                     rotator->current_sw_state == true);

                sleep_ms(10);
                wait_count++;
            }

            if (wait_count >= max_wait)
            {
                debug(WARN, "Button release timeout, proceeding anyway");
            }

            input_update(buttons, rotator);
            menu_active = true;
        }

        if (menu_active)
        {
            menu_main_start(buttons, rotator, mtrx);
        }

        // Log power stats every 30 seconds
        if (power_log_tick % 3000 == 0)
        {
            power_log_stats(TRACE, config.brightness);
        }
        power_log_tick++;

        sleep_ms(10);
    }

    // Never reached
    return 0;
}

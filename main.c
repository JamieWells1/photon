#include <const.h>
#include <dns_server.h>
#include <flash_config.h>
#include <global.h>
#include <graphics.h>
#include <http_server.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <menu.h>
#include <secrets_config.h>
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
    if (rtr->direction == 1)
    {
        debug(DEBUG, "Rotator clockwise");
    }
    if (rtr->direction == -1)
    {
        debug(DEBUG, "Rotator anti-clockwise");
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
    rotator.direction = 0;
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
    debug(INFO, "✓✓✓ Main init complete. Starting main execution loop.");

    wifi_config_t config;
    bool wifi_ready = false;

    if (flash_config_exists() && flash_config_read(&config))
    {
        debug(INFO, "Flash config found, attempting to connect to WiFi");
        if (wifi_connect(config.ssid, config.password, mtrx) == 0)
        {
            debug(INFO, "WiFi connected successfully");
            wifi_ready = true;
        }
        else
        {
            debug(WARN, "WiFi connection failed, entering setup mode");
            matrix_clear(mtrx);
            matrix_display_word_icon_pair("FAIL", &RED, NULL, 0);
            matrix_show(mtrx);
            sleep_ms(2000);
            flash_config_erase();
        }
    }

    if (!wifi_ready)
    {
        debug(INFO, "No valid WiFi config, entering setup mode");
        matrix_clear(mtrx);
        matrix_display_word_icon_pair("SETUP", &BLUE, NULL, 0);
        matrix_show(mtrx);

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

        while (!http_server_credentials_saved())
        {
            cyw43_arch_poll();
            sleep_ms(10);
        }

        debug(INFO, "Credentials saved, rebooting");
        matrix_clear(mtrx);
        matrix_display_word_icon_pair("SAVED", &GREEN, NULL, 0);
        matrix_show(mtrx);
        sleep_ms(2000);

        watchdog_reboot(0, 0, 0);
    }

    debug(INFO, "PRESS ANY BUTTON TO START");

    bool menu_active = false;

    while (true)
    {
        input_update(buttons, rotator);
        if (LOG_LEVEL >= DEBUG)
        {
            debug_inputs(buttons, rotator);
        }

        if (input_any_btn_pressed(buttons, rotator) && !menu_active)
        {
            graphics_display_start_screen(mtrx);

            bool all_released = false;
            while (!all_released)
            {
                input_update(buttons, rotator);
                all_released =
                    (buttons[0].current_state == true && buttons[1].current_state == true &&
                     rotator->current_sw_state == true);
                sleep_ms(10);
            }
            input_update(buttons, rotator);

            menu_active = true;
        }

        if (menu_active)
        {
            menu_start(buttons, rotator, mtrx);
        }

        sleep_ms(10);
    }

    // Never reached
    return 0;
}

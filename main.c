#include <const.h>
#include <input.h>
#include <matrix.h>
#include <print.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <ws2812.pio.h>

Button* main_init_buttons()
{
    static Button buttons[2] = {{.pin = BUTTON_LEFT_PIN, .last_state = true},
                                {.pin = BUTTON_RIGHT_PIN, .last_state = true}};

    gpio_init(buttons[0].pin);
    gpio_set_dir(buttons[0].pin, GPIO_IN);
    gpio_pull_up(buttons[0].pin);

    gpio_init(buttons[1].pin);
    gpio_set_dir(buttons[1].pin, GPIO_IN);
    gpio_pull_up(buttons[1].pin);

    debug("✓ GPIO pins for buttons initialised");

    return buttons;
}

Rotator* main_init_rotator()
{
    static Rotator rotator = {.pin_clk = ROTATOR_CLK_PIN,
                              .pin_dt = ROTATOR_DT_PIN,
                              .pin_sw = ROTATOR_SW_PIN,
                              .last_clk_state = true,
                              .last_sw_state = true};

    gpio_init(ROTATOR_CLK_PIN);
    gpio_set_dir(ROTATOR_CLK_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_CLK_PIN);

    gpio_init(ROTATOR_DT_PIN);
    gpio_set_dir(ROTATOR_DT_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_DT_PIN);

    gpio_init(ROTATOR_SW_PIN);
    gpio_set_dir(ROTATOR_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_SW_PIN);

    debug("✓ GPIO pins for rotary encoder initialised");

    return &rotator;
}

Matrix* main_init_matrix()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, MATRIX_MATRIX_PIN, MATRIX_DATA_FREQ, false);
    debug("✓ WS2812 program initialised");

    static Matrix matrix = {.pin = MATRIX_MATRIX_PIN, .pio = pio0, .sm = 0};
    return &matrix;
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    // Time buffer for screening into program to see initial debug logs
    sleep_ms(2000);

    if (DEBUG_LOGS)
    {
        printf("[INFO]: DEBUG MODE ENABMATRIX\n");
    }
    else
    {
        printf("[INFO]: DEBUG MODE DISABMATRIX\n");
    }

    Button* buttons = main_init_buttons();
    Button* button_left = &buttons[0];
    Button* button_right = &buttons[1];
    Rotator* rotator = main_init_rotator();
    Matrix* matrix = main_init_matrix();

    // Initialize random seed and clear matrix
    srand(get_absolute_time());
    matrix_clear();
    matrix_show(matrix);

    debug("✓✓✓ Main init complete. Starting main execution loop.");

    while (true)
    {
        if (input_btn_pressed(button_right))
        {
            debug("RIGHT PRESSED");

            // Light up random pixel with random color
            int x = rand() % MATRIX_WIDTH;
            int y = rand() % MATRIX_HEIGHT;
            uint8_t r = rand() % 256;
            uint8_t g = rand() % 256;
            uint8_t b = rand() % 256;

            RGB colour = {.r = r, .g = g, .b = b};

            matrix_set_pixel(x, y, colour);
            matrix_show(matrix);
            sleep_ms(1000);

            matrix_clear();
            matrix_show(matrix);
        }
        if (input_btn_released(button_right))
        {
            debug("RIGHT RELEASED");
        }

        if (input_btn_pressed(button_left))
        {
            debug("LEFT PRESSED");
        }
        if (input_btn_released(button_left))
        {
            debug("LEFT RELEASED");
        }

        sleep_ms(10);
    }

    return 0;
}

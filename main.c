#include <const.h>
#include <input.h>
#include <matrix.h>
#include <print.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <ws2812.pio.h>

void debug_inputs(Button* btns, Rotator* rtr)
{
    if (input_btn_pressed(&btns[0]))
    {
        debug("Left button pressed");
    }
    if (input_btn_pressed(&btns[1]))
    {
        debug("Right button pressed");
    }
    if (input_btn_released(&btns[0]))
    {
        debug("Left button released");
    }
    if (input_btn_released(&btns[1]))
    {
        debug("Right button released");
    }
    if (input_rtr_cw(rtr))
    {
        debug("Rotator clockwise");
    }
    if (input_rtr_anti_cw(rtr))
    {
        debug("Rotator anti-clockwise");
    }
    if (input_rtr_pressed(rtr))
    {
        debug("Rotator pressed");
    }
    if (input_rtr_released(rtr))
    {
        debug("Rotator released");
    }
}

Button* main_init_buttons()
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

    debug("✓ GPIO pins for buttons initialised");

    return buttons;
}

Rotator* main_init_rotator()
{
    static Rotator rotator = {
        .pin_clk = ROTATOR_CLK_PIN, .pin_dt = ROTATOR_DT_PIN, .pin_sw = ROTATOR_SW_PIN};

    // Initialize GPIO pins
    gpio_init(ROTATOR_CLK_PIN);
    gpio_set_dir(ROTATOR_CLK_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_CLK_PIN);

    gpio_init(ROTATOR_DT_PIN);
    gpio_set_dir(ROTATOR_DT_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_DT_PIN);

    gpio_init(ROTATOR_SW_PIN);
    gpio_set_dir(ROTATOR_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_SW_PIN);

    // Read actual GPIO states and initialize both current and last
    rotator.current_clk_state = gpio_get(ROTATOR_CLK_PIN);
    rotator.last_clk_state = rotator.current_clk_state;

    rotator.current_dt_state = gpio_get(ROTATOR_DT_PIN);

    rotator.current_sw_state = gpio_get(ROTATOR_SW_PIN);
    rotator.last_sw_state = rotator.current_sw_state;

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
    sleep_ms(100);

    if (DEBUG_MODE)
    {
        printf("[INFO]: Debug mode ENABLED. Debug logs will appear.\n");
    }
    else
    {
        printf("[INFO]: Debug mode DISABLED. Debug logs will not appear.\n");
    }
    // Get random seed
    srand(get_absolute_time());

    Button* buttons = main_init_buttons();
    Button* button_left = &buttons[0];
    Button* button_right = &buttons[1];
    Rotator* rotator = main_init_rotator();
    Matrix* matrix = main_init_matrix();

    matrix_clear(matrix);

    debug("✓✓✓ Main init complete. Starting main execution loop.");

    while (true)
    {
        input_update_inputs(buttons, rotator);
        if (DEBUG_MODE)
        {
            debug_inputs(buttons, rotator);
        }

        if (input_btn_pressed(button_right))
        {
            // Light up random pixel with random color
            int x = rand() % MATRIX_WIDTH;
            int y = rand() % MATRIX_HEIGHT;
            uint8_t r = rand() % 256;
            uint8_t g = rand() % 256;
            uint8_t b = rand() % 256;

            RGB colour = {.r = r, .g = g, .b = b};

            matrix_set_pixel(x, y, &colour);
            matrix_show(matrix);
        }
        if (input_btn_released(button_right))
        {
            matrix_clear(matrix);
        }

        sleep_ms(10);
    }

    return 0;
}

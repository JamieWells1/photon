#include <const.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <menu.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <pico/stdlib.h>
#include <ws2812.pio.h>

// Define DEBUG_LEVEL here (declared as extern in const.h)
const DebugLevel DEBUG_LEVEL = TRACE;

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

    ws2812_program_init(pio, sm, offset, MATRIX_DIN_PIN, MATRIX_DATA_FREQ, false);
    debug("✓ WS2812 program initialised");

    static Matrix matrix = {.pin = MATRIX_DIN_PIN, .pio = pio0, .sm = 0};
    MATRIX_ORIENTATION = HORIZONTAL;
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

    if (DEBUG_LEVEL == TRACE)
    {
        printf("[INFO]: Debug level TRACE. Trace logs will appear.\n");
    }
    else if (DEBUG_LEVEL == INPUTS)
    {
        printf("[INFO]: Debug level INPUTS. Only debug logs for hardware inputs will appear.\n");
    }
    else
    {
        printf("[INFO]: Debug level INFO. No debug logs will appear.\n");
    }

    Button* buttons = main_init_buttons();
    Rotator* rotator = main_init_rotator();
    Matrix* mtrx = main_init_matrix();

    matrix_clear(mtrx);
    debug("✓✓✓ Main init complete. Starting main execution loop.");
    debug("PRESS ANY BUTTON TO START");

    bool menu_active = false;

    while (true)
    {
        input_update(buttons, rotator);
        if (DEBUG_LEVEL >= INPUTS)
        {
            debug_inputs(buttons, rotator);
        }

        if (input_any_btn_pressed(buttons, rotator) && !menu_active)
        {
            graphics_display_start_screen(mtrx);
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

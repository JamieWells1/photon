#include <const.h>
#include <print.h>

#include <stdint.h>
#include <stdio.h>

#include <pico/stdlib.h>
#include <ws2812.pio.h>

#define BUTTON_LEFT_PIN 9   // GP9
#define BUTTON_RIGHT_PIN 5  // GP5

#define ROTATOR_CLK_PIN 18  // GP18
#define ROTATOR_DT_PIN 19   // GP19
#define ROTATOR_SW_PIN 20   // GP20

#define LED_MATRIX_PIN 2  // GP2
#define LED_DATA_FREQ 800000

typedef struct
{
    uint8_t pin;
    bool last_state;
} Button;

typedef struct
{
    uint8_t pin_clk;
    uint8_t pin_dt;
    uint8_t pin_sw;
    bool last_clk_state;
    bool last_sw_state;
} Rotator;

typedef struct
{
    uint8_t pin;
    PIO pio;
    uint8_t sm;
} LED;

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

LED* main_init_led()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, LED_DATA_FREQ, false);
    debug("✓ WS2812 program initialised");

    static LED led = {.pin = LED_MATRIX_PIN, .pio = pio0, .sm = 0};
    return &led;
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    // Time to screen into program to see debug logs
    sleep_ms(2000);

    if (DEBUG_LOGS)
    {
        printf("[INFO]: DEBUG MODE ENABLED\n");
    }
    else
    {
        printf("[INFO]: DEBUG MODE DISABLED\n");
    }

    Button* buttons = main_init_buttons();
    Button* button_left = &buttons[0];
    Button* button_right = &buttons[1];
    Rotator* rotator = main_init_rotator();
    LED* led = main_init_led();
    
    debug("✓✓✓ Main init complete. Starting main execution loop.");

    while (true)
    {
        bool current_state_right = gpio_get(BUTTON_RIGHT_PIN);
        bool current_state_left = gpio_get(BUTTON_LEFT_PIN);

        if (button_right->last_state == true && current_state_right == false)
        {
            debug("RIGHT PRESSED");
        }
        if (button_right->last_state == false && current_state_right == true)
        {
            debug("RIGHT RELEASED");
        }

        if (button_left->last_state == true && current_state_left == false)
        {
            debug("LEFT PRESSED");
        }
        if (button_left->last_state == false && current_state_left == true)
        {
            debug("LEFT RELEASED");
        }

        button_right->last_state = current_state_right;
        button_left->last_state = current_state_left;

        // Test rotary encoder rotation (Functionality 1: direction detection)
        bool current_clk = gpio_get(ROTATOR_CLK_PIN);
        if (current_clk != rotator->last_clk_state && current_clk == false)
        {
            bool current_dt = gpio_get(ROTATOR_DT_PIN);
            if (current_dt != current_clk)
            {
                debug("Encoder CLOCKWISE");
            }
            else
            {
                debug("Encoder COUNTER-CLOCKWISE");
            }
        }
        rotator->last_clk_state = current_clk;

        bool current_sw = gpio_get(ROTATOR_SW_PIN);
        if (rotator->last_sw_state == true && current_sw == false)
        {
            debug("ENCODER BUTTON PRESSED");
        }
        if (rotator->last_sw_state == false && current_sw == true)
        {
            debug("ENCODER BUTTON released");
        }
        rotator->last_sw_state = current_sw;

        sleep_ms(10);
    }

    return 0;
}

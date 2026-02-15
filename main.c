#include <stdint.h>
#include <stdio.h>

#include <pico/stdlib.h>
#include <ws2812.pio.h>

#define BUTTON_LEFT_PIN 9   // GP9
#define BUTTON_RIGHT_PIN 5  // GP5

#define ROTATOR_CLK_PIN 11  // GP11
#define ROTATOR_DT_PIN 12   // GP12
#define ROTATOR_SW_PIN 13   // GP13

#define LED_MATRIX_PIN 2  // GP2

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
    bool last_state;
} Rotator;

typedef struct
{
    uint8_t pin;
    PIO pio;
    uint8_t sm;
} LED;

Button* init_buttons()
{
    static Button buttons[2] = {{.pin = BUTTON_LEFT_PIN, .last_state = true},
                                {.pin = BUTTON_RIGHT_PIN, .last_state = true}};

    gpio_init(buttons[0].pin);
    gpio_set_dir(buttons[0].pin, GPIO_IN);
    gpio_pull_up(buttons[0].pin);

    gpio_init(buttons[1].pin);
    gpio_set_dir(buttons[1].pin, GPIO_IN);
    gpio_pull_up(buttons[1].pin);

    return buttons;
}

Rotator* init_rotator()
{
    static Rotator rotator = {.pin_clk = ROTATOR_CLK_PIN,
                              .pin_dt = ROTATOR_DT_PIN,
                              .pin_sw = ROTATOR_SW_PIN,
                              .last_state = true};

    gpio_init(ROTATOR_CLK_PIN);
    gpio_set_dir(ROTATOR_CLK_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_CLK_PIN);

    gpio_init(ROTATOR_DT_PIN);
    gpio_set_dir(ROTATOR_DT_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_DT_PIN);

    gpio_init(ROTATOR_SW_PIN);
    gpio_set_dir(ROTATOR_SW_PIN, GPIO_IN);
    gpio_pull_up(ROTATOR_SW_PIN);

    return &rotator;
}

LED* init_led()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, false);

    static LED led = {.pin = LED_MATRIX_PIN, .pio = pio0, .sm = 0};
    return &led;
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    Button* buttons = init_buttons();
    Button* button_left = &buttons[0];
    Button* button_right = &buttons[1];

    Rotator* rotator = init_rotator();

    LED* led = init_led();

    while (true)
    {
        bool current_state_right = gpio_get(BUTTON_RIGHT_PIN);
        bool current_state_left = gpio_get(BUTTON_LEFT_PIN);

        if (button_right->last_state == true && current_state_right == false)
        {
            printf("RIGHT PRESSED!");
        }
        if (button_right->last_state == false && current_state_right == true)
        {
            printf("RIGHT released\n\n");
        }

        if (button_left->last_state == true && current_state_left == false)
        {
            printf("LEFT PRESSED!");
        }
        if (button_left->last_state == false && current_state_left == true)
        {
            printf("LEFT released\n\n");
        }

        button_right->last_state = current_state_right;
        button_left->last_state = current_state_left;
        sleep_ms(10);
    }
}

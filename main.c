#include <pico/stdlib.h>
#include <stdio.h>

#define BUTTON_PIN_RIGHT 5  // GP5
#define BUTTON_PIN_LEFT 9   // GP9

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("\n=== Two Button Test ===\n");
    printf("RIGHT: GP5 | LEFT: GP9\n\n");

    gpio_init(BUTTON_PIN_RIGHT);
    gpio_set_dir(BUTTON_PIN_RIGHT, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_RIGHT);

    gpio_init(BUTTON_PIN_LEFT);
    gpio_set_dir(BUTTON_PIN_LEFT, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_LEFT);

    bool last_state_right = true;
    bool last_state_left = true;

    while (true)
    {
        bool current_state_right = gpio_get(BUTTON_PIN_RIGHT);
        bool current_state_left = gpio_get(BUTTON_PIN_LEFT);

        if (last_state_right == true && current_state_right == false)
        {
            printf("RIGHT PRESSED!");
        }
        if (last_state_right == false && current_state_right == true)
        {
            printf("RIGHT released\n\n");
        }

        if (last_state_left == true && current_state_left == false)
        {
            printf("LEFT PRESSED!");
        }
        if (last_state_left == false && current_state_left == true)
        {
            printf("LEFT released\n\n");
        }

        last_state_right = current_state_right;
        last_state_left = current_state_left;
        sleep_ms(10);
    }
}

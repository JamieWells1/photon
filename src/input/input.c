#include <const.h>
#include <input.h>
#include <print.h>

#include <ws2812.pio.h>

bool input_button_pressed(Button* btn)
{
    bool current_state = gpio_get(btn->pin);
    if (btn->last_state == true && current_state == false)
    {
        return true;
    }
    return false;
}

bool input_button_released(Button* btn)
{
    bool current_state = gpio_get(btn->pin);
    if (btn->last_state == false && current_state == true)
    {
        return true;
    }
    return false;
}

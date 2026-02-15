#include <const.h>
#include <input.h>
#include <print.h>

#include <ws2812.pio.h>

bool input_button_pressed(Button* btn)
{
    bool current_state = gpio_get(btn->pin);
    bool pressed = (btn->last_state == true && current_state == false);
    btn->last_state = current_state;

    if (pressed)
    {
        debug("Button pressed");
        return true;
    }
    return false;
}

bool input_button_released(Button* btn)
{
    bool current_state = gpio_get(btn->pin);
    bool released = (btn->last_state == false && current_state == true);
    btn->last_state = current_state;

    if (released)
    {
        debug("Button released");
        return true;
    }
    return false;
}

bool input_rotator_cw(Rotator* rtr)
{
    bool current_clk = gpio_get(rtr->pin_clk);
    bool is_cw = false;

    if (current_clk != rtr->last_clk_state && current_clk == false)
    {
        bool current_dt = gpio_get(rtr->pin_dt);
        if (current_dt != current_clk)
        {
            debug("Encoder clockwise");
            is_cw = true;
        }
    }

    rtr->last_clk_state = current_clk;
    return is_cw;
}

bool input_rotator_anti_cw(Rotator* rtr)
{
    bool current_clk = gpio_get(rtr->pin_clk);
    bool is_anti_cw = false;

    if (current_clk != rtr->last_clk_state && current_clk == false)
    {
        bool current_dt = gpio_get(rtr->pin_dt);
        if (current_dt == current_clk)
        {
            debug("Encoder anti-clockwise");
            is_anti_cw = true;
        }
    }

    rtr->last_clk_state = current_clk;
    return is_anti_cw;
}

bool input_rotator_pressed(Rotator* rtr)
{
    bool current_sw = gpio_get(rtr->pin_sw);
    bool pressed = (rtr->last_sw_state == true && current_sw == false);
    rtr->last_sw_state = current_sw;

    if (pressed)
    {
        debug("Encoder pressed");
        return true;
    }
    return false;
}

bool input_rotator_released(Rotator* rtr)
{
    bool current_sw = gpio_get(rtr->pin_sw);
    bool released = (rtr->last_sw_state == false && current_sw == true);
    rtr->last_sw_state = current_sw;

    if (released)
    {
        debug("Encoder released");
        return true;
    }
    return false;
}

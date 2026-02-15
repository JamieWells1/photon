#include <const.h>
#include <hardware/gpio.h>
#include <input.h>
#include <print.h>

#include <stdint.h>
#include <ws2812.pio.h>

void input_update_inputs(Button* btns, Rotator* rtr)
{
    // Update states of left and right buttons
    input_btn_update(&btns[0]);
    input_btn_update(&btns[1]);

    // Update states of rotary encoder
    input_rtr_update(rtr);
}

void input_rtr_update(Rotator* rtr)
{
    rtr->last_clk_state = rtr->current_clk_state;
    rtr->last_sw_state = rtr->current_sw_state;

    rtr->current_clk_state = gpio_get(rtr->pin_clk);
    rtr->current_dt_state = gpio_get(rtr->pin_dt);
    rtr->current_sw_state = gpio_get(rtr->pin_sw);
}

void input_btn_update(Button* btn)
{
    btn->last_state = btn->current_state;
    btn->current_state = gpio_get(btn->pin);
}

bool input_btn_pressed(Button* btn)
{
    return btn->last_state == true && btn->current_state == false;
}

bool input_btn_released(Button* btn)
{
    return btn->last_state == false && btn->current_state == true;
}

bool input_rtr_cw(Rotator* rtr)
{
    // Check if CLK changed from HIGH to LOW
    if (rtr->last_clk_state == true && rtr->current_clk_state == false)
    {
        // If DT is different from CLK, it's clockwise
        if (rtr->current_dt_state != rtr->current_clk_state)
        {
            return true;
        }
    }
    return false;
}

bool input_rtr_anti_cw(Rotator* rtr)
{
    // Check if CLK changed from HIGH to LOW
    if (rtr->last_clk_state == true && rtr->current_clk_state == false)
    {
        // If DT is same as CLK, it's anti-clockwise
        if (rtr->current_dt_state == rtr->current_clk_state)
        {
            return true;
        }
    }
    return false;
}

bool input_rtr_pressed(Rotator* rtr)
{
    return rtr->last_sw_state == true && rtr->current_sw_state == false;
}

bool input_rtr_released(Rotator* rtr)
{
    return rtr->last_sw_state == false && rtr->current_sw_state == true;
}

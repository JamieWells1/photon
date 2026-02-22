#include <input.h>

#include <const.h>

#include <stdint.h>

#include <hardware/gpio.h>
#include <pico/time.h>
#include <ws2812.pio.h>

#define DEBOUNCE_TIME_US 500

void input_update(Button* btns, Rotator* rtr)
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

    uint8_t new_state = (rtr->current_clk_state << 1) | rtr->current_dt_state;

    uint32_t now = time_us_32();
    if (new_state != rtr->state && (now - rtr->last_change_time) > DEBOUNCE_TIME_US)
    {
        // Valid state transition table for quadrature encoder
        // [old_state][new_state] -> direction (1=CW, -1=CCW, 0=invalid)
        static const int8_t transition_table[4][4] = {
            {0, -1, 1, 0},  // From state 0 (00)
            {1, 0, 0, -1},  // From state 1 (01)
            {-1, 0, 0, 1},  // From state 2 (10)
            {0, 1, -1, 0}   // From state 3 (11)
        };

        int8_t dir = transition_table[rtr->state][new_state];

        // Only update direction when reaching rest position (state 3)
        // This gives us one trigger per detent instead of four
        if (dir != 0 && new_state == 3)
        {
            rtr->direction = dir;
            rtr->last_change_time = now;
        }

        rtr->state = new_state;
    }
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
    if (rtr->direction == 1)
    {
        rtr->direction = 0;
        return true;
    }
    return false;
}

bool input_rtr_anti_cw(Rotator* rtr)
{
    if (rtr->direction == -1)
    {
        rtr->direction = 0;
        return true;
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

bool input_any_btn_pressed(Button* btns, Rotator* rtr)
{
    if (input_btn_pressed(&btns[0]) || input_btn_pressed(&btns[1]) || input_rtr_pressed(rtr))
    {
        return true;
    }
    return false;
}

/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <input.h>

#include <const.h>

#include <stdint.h>

#include <hardware/gpio.h>
#include <pico/time.h>
#include <ws2812.pio.h>

#define DEBOUNCE_TIME_US 200
#define LONG_PRESS_TIME_MS 500

void input_update(Button* btns, Rotator* rtr)
{
    // Update states of left and right buttons
    input_btn_update(&btns[0]);
    input_btn_update(&btns[1]);

    // Update rotary encoder state
    input_rtr_update(rtr);
}

void input_rtr_update(Rotator* rtr)
{
    rtr->last_clk_state = rtr->current_clk_state;
    rtr->last_sw_state = rtr->current_sw_state;

    rtr->current_clk_state = gpio_get(rtr->pin_clk);
    rtr->current_dt_state = gpio_get(rtr->pin_dt);
    rtr->current_sw_state = gpio_get(rtr->pin_sw);

    if (input_rtr_pressed(rtr))
    {
        rtr->press_start_time = time_us_32() / 1000;
        rtr->long_press_triggered = false;
    }

    if (input_rtr_released(rtr))
    {
        rtr->long_press_triggered = false;
    }

    uint8_t new_state = (rtr->current_clk_state << 1) | rtr->current_dt_state;

    uint32_t now = time_us_32();
    if (new_state != rtr->state && (now - rtr->last_change_time) > DEBOUNCE_TIME_US)
    {
        static const int8_t transition_table[4][4] = {
            {0, -1, 1, 0},   // From state 0 (00)
            {1, 0, 0, -1},   // From state 1 (01)
            {-1, 0, 0, 1},   // From state 2 (10)
            {0, 1, -1, 0}    // From state 3 (11)
        };

        int8_t dir = transition_table[rtr->state][new_state];

        if (dir != 0 && new_state == 3)
        {
            rtr->rotation_count += dir;
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
    if (rtr->rotation_count > 0)
    {
        rtr->rotation_count--;
        return true;
    }
    return false;
}

bool input_rtr_anti_cw(Rotator* rtr)
{
    if (rtr->rotation_count < 0)
    {
        rtr->rotation_count++;
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

bool input_rtr_long_pressed(Rotator* rtr)
{
    if (rtr->current_sw_state == false && !rtr->long_press_triggered)
    {
        uint32_t now_ms = time_us_32() / 1000;
        uint32_t held_time = now_ms - rtr->press_start_time;

        if (held_time >= LONG_PRESS_TIME_MS)
        {
            rtr->long_press_triggered = true;
            return true;
        }
    }
    return false;
}

bool input_any_btn_pressed(Button* btns, Rotator* rtr)
{
    if (input_btn_pressed(&btns[0]) || input_btn_pressed(&btns[1]) || input_rtr_pressed(rtr))
    {
        return true;
    }
    return false;
}

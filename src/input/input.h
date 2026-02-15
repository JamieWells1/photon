#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t pin;
    bool current_state;
    bool last_state;
} Button;

typedef struct
{
    uint8_t pin_clk;
    uint8_t pin_dt;
    uint8_t pin_sw;
    bool current_clk_state;
    bool current_dt_state;
    bool current_sw_state;
    bool last_clk_state;
    bool last_sw_state;
} Rotator;

void input_update(Button* btns, Rotator* rtr);

void input_btn_update(Button* btn);
void input_rtr_update(Rotator* rtr);

bool input_btn_pressed(Button* btn);
bool input_btn_released(Button* btn);

bool input_rtr_cw(Rotator* rtr);
bool input_rtr_anti_cw(Rotator* rtr);
bool input_rtr_pressed(Rotator* rtr);
bool input_rtr_released(Rotator* rtr);

#endif  // INPUT_H

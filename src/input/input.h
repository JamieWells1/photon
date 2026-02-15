#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

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

bool input_btn_pressed(Button* btn);
bool input_btn_released(Button* btn);

#endif  // INPUT_H

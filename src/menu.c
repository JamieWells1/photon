#include <menu.h>

#include <const.h>
#include <graphics.h>
#include <input.h>

#include <stdlib.h>

#include <pico/time.h>
#include "displays.h"

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // Get random seed
    srand(get_absolute_time());

    Button* btn_left = &btns[0];
    Button* btn_right = &btns[1];

    if (input_btn_pressed(btn_right))
    {
        displays_word("HELLO MUM", 1, 1, &DEFAULT_COLOUR);

        matrix_show(mtrx);
        sleep_ms(3000);
        matrix_clear(mtrx);
    }

    sleep_ms(10);
}

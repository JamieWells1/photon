#include <menu.h>
#include "common/const.h"

void menu_start(Button* btn_left, Button* btn_right, Rotator* rtr, Matrix* matrix)
{
    while (true)
    {
        input_update(buttons, rotator);
        if (DEBUG_LEVEL >= INPUTS)
        {
            debug_inputs(buttons, rotator);
        }

        if (input_btn_pressed(button_right))
        {
            // Light up random pixel with random color
            int x = rand() % MATRIX_WIDTH;
            int y = rand() % MATRIX_HEIGHT;
            uint8_t r = rand() % 256;
            uint8_t g = rand() % 256;
            uint8_t b = rand() % 256;

            RGB colour = {.r = r, .g = g, .b = b};

            matrix_set_pixel(x, y, &colour);
            matrix_show(matrix);
        }
        if (input_btn_released(button_right))
        {
            matrix_clear(matrix);
        }

        sleep_ms(10);
    }
}

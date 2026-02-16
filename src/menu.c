#include <const.h>
#include <input.h>
#include <menu.h>
#include <print.h>
#include <graphics.h>

#include <stdlib.h>

#include <pico/time.h>

void menu_start(Button* btns, Rotator* rtr, Matrix* matrix)
{
    graphics_display_start_screen(matrix);
    graphics_display_main_menu(matrix);

    Button* btn_left = &btns[0];
    Button* btn_right = &btns[1];

    if (input_btn_pressed(btn_right))
    {
        int x = rand() % MATRIX_WIDTH;
        int y = rand() % MATRIX_HEIGHT;
        uint8_t r = rand() % 256;
        uint8_t g = rand() % 256;
        uint8_t b = rand() % 256;

        RGB colour = {.r = r, .g = g, .b = b};

        matrix_set_pixel(x, y, &colour);
        matrix_show(matrix);
    }

    if (input_btn_released(btn_right))
    {
        matrix_clear(matrix);
    }

    sleep_ms(10);
}

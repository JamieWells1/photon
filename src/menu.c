#include <menu.h>

#include <const.h>
#include <graphics.h>
#include <input.h>

#include <stdlib.h>

#include <pico/time.h>

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    graphics_display_start_screen(mtrx);
    graphics_display_main_menu(mtrx);

    Button* btn_left = &btns[0];
    Button* btn_right = &btns[1];

    if (input_btn_pressed(btn_right))
    {
        int x = rand() % MATRIX_WIDTH;
        int y = rand() % MATRIX_HEIGHT;
        uint8_t r = rand() % 256;
        uint8_t g = rand() % 256;
        uint8_t b = rand() % 256;

        RGB col = {.r = r, .g = g, .b = b};
        Pixel pxl = {x, y, col};

        matrix_set_pixel(&pxl);
        matrix_show(mtrx);
    }

    if (input_btn_released(btn_right))
    {
        matrix_clear(mtrx);
    }

    sleep_ms(10);
}

#include <displays.h>
#include <graphics.h>
#include <pico/time.h>
#include <stdint.h>
#include "matrix.h"

void graphics_display_start_screen(Matrix* mtrx)
{
    

    matrix_show(mtrx);
    sleep_ms(2000);
    matrix_clear(mtrx);
}

void graphics_display_main_menu(Matrix* mtrx)
{
    // TODO
}

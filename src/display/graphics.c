#include <graphics.h>

#include <displays.h>
#include <matrix.h>
#include <const.h>

#include <stdint.h>

#include <pico/time.h>

void graphics_display_start_screen(Matrix* mtrx)
{
    displays_word(PROJECT_NAME, 1, 1, &DEFAULT_COLOUR);

    matrix_show(mtrx);
    sleep_ms(2000);
    matrix_clear(mtrx);
}

void graphics_display_main_menu(Matrix* mtrx)
{
    // TODO
}

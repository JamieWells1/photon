#include <graphics.h>

#include <displays.h>
#include <matrix.h>
#include <const.h>

#include <stdint.h>

#include <pico/time.h>

void graphics_display_start_screen(Matrix* mtrx)
{
    displays_word(PROJECT_NAME, 1, 1, &LOGO_COLOUR);
    displays_icon(PHOTON, 26, 1, &LOGO_COLOUR);
    draw_horiz_line(1, 7, 30, &LOGO_COLOUR);

    matrix_show(mtrx);
    sleep_ms(3000);
    matrix_clear(mtrx);
}

void graphics_display_main_menu(Matrix* mtrx)
{
    // TODO
}

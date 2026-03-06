#include <graphics.h>

#include <const.h>
#include <glyphs.h>
#include <matrix.h>

#include <stdint.h>

#include <pico/time.h>

void graphics_display_start_screen(Matrix* mtrx)
{
    matrix_clear(mtrx);

    matrix_display_word_icon_pair(PROJECT_NAME, &LOGO_COLOUR, &ICONS_ARR[PHOTON], 0);

    matrix_show(mtrx);
    sleep_ms(3000);
    matrix_clear(mtrx);
    matrix_show(mtrx);
}

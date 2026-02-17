#include <displays.h>
#include <graphics.h>
#include <pico/time.h>
#include <stdint.h>
#include "matrix.h"

void graphics_display_start_screen(Matrix* mtrx)
{
    size_t logo_pixels = sizeof(PHOTON_LOGO) / sizeof(PHOTON_LOGO[0]);

    for (size_t i = 0; i < logo_pixels; i++)
    {
        matrix_set_pixel(&PHOTON_LOGO[i]);
    }

    matrix_show(mtrx);
    sleep_ms(2000);
    matrix_clear(mtrx);
}

void graphics_display_main_menu(Matrix* mtrx)
{
    // TODO
}

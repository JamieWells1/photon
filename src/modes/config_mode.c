#include <config_mode.h>

#include <const.h>
#include <glyphs.h>
#include <http_server.h>
#include <macro.h>
#include <matrix.h>

#include <stdio.h>

static bool server_started = false;
static int scroll_offset = 0;

void config_display(Matrix* mtrx)
{
    if (!server_started)
    {
        http_server_start();
        server_started = true;
        debug(INFO, "Config server started. Access at: http://photon.local");
    }

    const char* url_text = "  http://photon.local";
    int total_width = matrix_calculate_word_width_with_space(url_text) + 4;

    matrix_clear(mtrx);

    // Draw the text twice for seamless scrolling loop
    for (int wrap = 0; wrap < 2; wrap++)
    {
        int text_x = (wrap * total_width) - scroll_offset;
        int text_y = 1;
        matrix_display_word(url_text, text_x, text_y, &BLUE);
    }

    matrix_show(mtrx);

    scroll_offset++;
    if (scroll_offset >= total_width)
    {
        scroll_offset = 0;
    }
}

void config_cleanup(void)
{
    server_started = false;
    scroll_offset = 0;
}

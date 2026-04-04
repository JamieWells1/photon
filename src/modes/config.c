/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <config.h>

#include <const.h>
#include <error.h>
#include <flash_memory.h>
#include <glyphs.h>
#include <http_server.h>
#include <macro.h>
#include <matrix.h>
#include <stock.h>
#include <wifi.h>

#include <stdio.h>

#include <hardware/watchdog.h>
#include <pico/cyw43_arch.h>

static int scroll_offset = 0;

void config_display(Matrix* mtrx)
{
    const char* url_text = "    http://delta.local";
    int total_width = matrix_calculate_word_width_with_space(url_text) + 4;

    matrix_clear(mtrx);

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

void config_cleanup(void) { scroll_offset = 0; }

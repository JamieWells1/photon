#ifndef MATRIX_H
#define MATRIX_H

#include <displays.h>

#include <stdint.h>
#include <ws2812.pio.h>

typedef struct
{
    uint8_t pin;
    PIO pio;
    uint8_t sm;
} Matrix;

void matrix_set_pixel(Pixel* pxl);
void matrix_clear_pixel(int x, int y);

void matrix_show(Matrix* mtrx);
void matrix_clear(Matrix* mtrx);

void matrix_display_letter(const char c, int x, int y, const RGB* col);
void matrix_display_icon(const Glyph* icon_type, int x, int y, const RGB* col);
void matrix_display_word(const char* word, int x, int y, const RGB* col);
void matrix_display_word_icon_pair(const char* word, const RGB* word_col, const Glyph* icon,
                                   int offset_x);

void matrix_draw_horiz_line(int x, int y, int length, const RGB* col);
void matrix_clear_horiz_line(int x, int y, int length);

void matrix_draw_vert_line(int x, int y, int length, const RGB* col);

#endif  // MATRIX_H

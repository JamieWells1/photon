#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <ws2812.pio.h>

typedef struct
{
    uint8_t pin;
    PIO pio;
    uint8_t sm;
} Matrix;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct
{
    uint8_t x;
    uint8_t y;
    RGB col;
} Pixel;

void matrix_set_pixel(Pixel* pxl);
void matrix_clear_pixel(int x, int y);

void matrix_show(Matrix* mtrx);
void matrix_clear(Matrix* mtrx);

#endif  // MATRIX_H

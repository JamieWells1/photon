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

void matrix_set_pixel(int x, int y, RGB* colour);
void matrix_clear_pixel(int x, int y);

void matrix_show(Matrix* matrix);
void matrix_clear(Matrix* matrix);

#endif  // MATRIX_H

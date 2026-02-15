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

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

int matrix_xy_to_index(int x, int y);

uint32_t matrix_rgb_to_grb(RGB colour);

// Set a specific pixel color
void matrix_set_pixel(int x, int y, RGB colour);

void matrix_clear();

void matrix_show(Matrix* matrix);

#endif  // MATRIX_H

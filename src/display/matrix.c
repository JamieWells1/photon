#include <const.h>
#include <matrix.h>

static uint32_t pixels[NUM_PIXELS];

int matrix_xy_to_index(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * MATRIX_WIDTH + x;
    }
    else
    {
        return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
    }
}

uint32_t matrix_rgb_to_grb(RGB col) { return (col.g << 16) | (col.r << 8) | col.b; }

void matrix_set_pixel(int x, int y, RGB col)
{
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
    {
        int index = matrix_xy_to_index(x, y);
        pixels[index] = matrix_rgb_to_grb(col);
    }
}

void matrix_clear()
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pixels[i] = 0;
    }
}

void matrix_show(Matrix* matrix)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(matrix->pio, matrix->sm, pixels[i] << 8u);
    }
}

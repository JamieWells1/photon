#include <const.h>
#include <matrix.h>
#include <print.h>
#include <stdio.h>

static uint32_t pixels[NUM_PIXELS];

static int matrix_xy_to_index(int x, int y)
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

static void matrix_set_brightness(RGB* col)
{
    col->r *= BRIGHTNESS_COEFFICIENT;
    col->g *= BRIGHTNESS_COEFFICIENT;
    col->b *= BRIGHTNESS_COEFFICIENT;
}

static uint32_t matrix_rgb_to_grb(RGB* col) { return (col->g << 16) | (col->r << 8) | col->b; }

// Set a pixel's colour state on the matrix
void matrix_set_pixel(int x, int y, RGB* col)
{
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT)
    {
        RGB dimmed = *col;
        matrix_set_brightness(&dimmed);
        int index = matrix_xy_to_index(x, y);
        pixels[index] = matrix_rgb_to_grb(&dimmed);

        char debug_msg[100];
        snprintf(debug_msg, sizeof(debug_msg), "Colour (%u, %u, %u) set at pixel (%d, %d)", col->r,
                 col->g, col->b, x, y);
        debug(debug_msg);
    }
}

void matrix_clear_pixel(int x, int y)
{
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0)
    {
        int index = matrix_xy_to_index(x, y);
        pixels[index] = 0;
    }
}

// Clear the matrix of its current state
void matrix_clear(Matrix* matrix)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pixels[i] = 0;
    }
    matrix_show(matrix);
    debug("Matrix cleared");
}

// Show the matrix current state
void matrix_show(Matrix* matrix)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(matrix->pio, matrix->sm, pixels[i] << 8u);
    }
    debug("Matrix showing new state");
}

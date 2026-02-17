#include <const.h>
#include <global.h>
#include <matrix.h>
#include <print.h>

#include <stdio.h>

static uint32_t pixels[NUM_PIXELS];

static int matrix_xy_to_index_horizontal(int input_x, int input_y)
{
    // Matrix is column-serpentine, starting bottom-right (i=0)
    // Map (0,0) to top-left: x=0 should be leftmost column, y=0 should be top

    int col = MATRIX_WIDTH - 1 - input_x;
    int row = input_y;

    if (col % 2 == 0)
    {
        return col * MATRIX_HEIGHT + (MATRIX_HEIGHT - 1 - row);
    }
    else
    {
        return col * MATRIX_HEIGHT + row;
    }
}

static int matrix_xy_to_index_vertical(int input_x, int input_y)
{
    // TODO: when turned 90 degrees clockwise
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

        int index = 0;
        if (MATRIX_ORIENTATION == HORIZONTAL)
        {
            int index = matrix_xy_to_index_horizontal(x, y);
        }
        else
        {
            int index = matrix_xy_to_index_vertical(x, y);
        }

        pixels[index] = matrix_rgb_to_grb(&dimmed);

        char debug_msg[100];
        snprintf(debug_msg, sizeof(debug_msg),
                 "Colour (%u, %u, %u) set at pixel (%d, %d) with index %d", col->r, col->g, col->b,
                 x, y, index);
        debug(debug_msg);
    }
}

void matrix_clear_pixel(int x, int y)
{
    if (x >= 0 && x < MATRIX_WIDTH && y >= 0)
    {
        int index;
        if (MATRIX_ORIENTATION == HORIZONTAL)
        {
            int index = matrix_xy_to_index_horizontal(x, y);
        }
        else
        {
            int index = matrix_xy_to_index_vertical(x, y);
        }
        pixels[index] = 0;
    }
}

// Clear the matrix of its current state
void matrix_clear(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pixels[i] = 0;
    }
    matrix_show(mtrx);
    debug("matrix_clear()");
}

// Show the matrix current state
void matrix_show(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(mtrx->pio, mtrx->sm, pixels[i] << 8u);
    }
    debug("matrix_show()");
}

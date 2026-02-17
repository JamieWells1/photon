#include <const.h>
#include <global.h>
#include <macro.h>
#include <matrix.h>

#include <stdio.h>

static uint32_t pixels[NUM_PIXELS];

static const int matrix_xy_to_index_horizontal(int input_x, int input_y)
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

static const int matrix_xy_to_index_vertical(int input_x, int input_y)
{
    // TODO: when turned 90 degrees clockwise
    return 0;
}

static void matrix_set_brightness(RGB* col)
{
    col->r *= BRIGHTNESS_COEFFICIENT;
    col->g *= BRIGHTNESS_COEFFICIENT;
    col->b *= BRIGHTNESS_COEFFICIENT;
}

static const uint32_t matrix_rgb_to_grb(RGB* col) { return (col->g << 16) | (col->r << 8) | col->b; }

// Set a pixel's colour state on the matrix
void matrix_set_pixel(Pixel* pxl)
{
    if (pxl->x >= 0 && pxl->x < MATRIX_WIDTH && pxl->y >= 0 && pxl->y < MATRIX_HEIGHT)
    {
        // Copy RGB object to modify when dimming colour values
        RGB dimmed = pxl->col;
        matrix_set_brightness(&dimmed);

        int index = 0;
        if (MATRIX_ORIENTATION == HORIZONTAL)
        {
            int index = matrix_xy_to_index_horizontal(pxl->x, pxl->y);
        }
        else
        {
            int index = matrix_xy_to_index_vertical(pxl->x, pxl->y);
        }

        pixels[index] = matrix_rgb_to_grb(&dimmed);

        debug("Colour (%u, %u, %u) set at pixel (%d, %d) with index %d", pxl->col.r, pxl->col.g, pxl->col.b,
              pxl->x, pxl->y, index);
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

#include <matrix.h>

#include <const.h>
#include <global.h>
#include <macro.h>

#include <ctype.h>
#include <string.h>

#include <displays.h>
#include <stdio.h>

// ==========================
// STATIC
// ==========================

static uint32_t g_pixels[NUM_PIXELS];

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
    return 0;
}

static void matrix_set_brightness(RGB* col)
{
    col->r *= BRIGHTNESS_COEFFICIENT;
    col->g *= BRIGHTNESS_COEFFICIENT;
    col->b *= BRIGHTNESS_COEFFICIENT;
}

static uint32_t matrix_rgb_to_grb(RGB* col) { return (col->g << 16) | (col->r << 8) | col->b; }

static const Glyph* matrix_letter_in_pxls(char ch)
{
    ch = toupper(ch);

    // Check for letters A-Z
    if (ch >= 'A' && ch <= 'Z')
    {
        return &ALPHABET[ch - 'A'];
    }

    // Check for punctuation
    switch (ch)
    {
        case '!':
            return &PUNCTUATION[0];
        case '?':
            return &PUNCTUATION[1];
        case '.':
            return &PUNCTUATION[2];
        case ',':
            return &PUNCTUATION[3];
        case '-':
            return &PUNCTUATION[4];
        default:
            return NULL;
    }
}

// ==========================
// PUBLIC
// ==========================

// Set a pixel's colour state on the matrix
void matrix_set_pixel(Pixel* pxl)
{
    if (IN_BOUNDS(pxl->x, pxl->y))
    {
        // Copy RGB object to modify when dimming colour values
        RGB dimmed = pxl->col;
        matrix_set_brightness(&dimmed);

        int index = 0;
        if (MATRIX_ORIENTATION == HORIZONTAL)
        {
            index = matrix_xy_to_index_horizontal(pxl->x, pxl->y);
        }
        else
        {
            index = matrix_xy_to_index_vertical(pxl->x, pxl->y);
        }

        g_pixels[index] = matrix_rgb_to_grb(&dimmed);
    }
}

void matrix_clear_pixel(int x, int y)
{
    if (IN_BOUNDS(x, y))
    {
        int index;
        if (MATRIX_ORIENTATION == HORIZONTAL)
        {
            index = matrix_xy_to_index_horizontal(x, y);
        }
        else
        {
            index = matrix_xy_to_index_vertical(x, y);
        }
        g_pixels[index] = 0;
    }
}

// Clear the matrix of its current state
void matrix_clear(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        g_pixels[i] = 0;
    }
    matrix_show(mtrx);
    debug("matrix_clear()");
}

// Show the matrix current state
void matrix_show(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(mtrx->pio, mtrx->sm, g_pixels[i] << 8u);
    }
    debug("matrix_show()");
}

// Only writes letter to state, not matrix
void matrix_display_letter(const char c, int x, int y, const RGB* col)
{
    const Glyph* letter = matrix_letter_in_pxls(c);
    if (!letter)
    {
        debug("Cannot display invalid character: '%c'", c);
        return;
    }

    for (size_t i = 0; i < letter->pxl_count; i++)
    {
        int transformed_x = letter->pixels[i].x + x;
        int transformed_y = letter->pixels[i].y + y;

        if (!IN_BOUNDS(transformed_x, transformed_y)) continue;

        Pixel pxl = {transformed_x, transformed_y, *col};
        matrix_set_pixel(&pxl);
        debug("Character '%c' set on matrix, position (%d, %d)", c, transformed_x, transformed_y);
    }
}

void matrix_display_icon(const Glyph* icon, int x, int y, const RGB* col)
{
    for (size_t i = 0; i < icon->pxl_count; i++)
    {
        int transformed_x = icon->pixels[i].x + x;
        int transformed_y = icon->pixels[i].y + y;

        if (!IN_BOUNDS(transformed_x, transformed_y)) continue;

        // Use icon's own color if no override provided
        const RGB* pixel_col = col ? col : &icon->pixels[i].col;

        Pixel pxl = {transformed_x, transformed_y, *pixel_col};
        matrix_set_pixel(&pxl);
    }
}

// Only writes word to state, not matrix
void matrix_display_word(const char* word, int x, int y, const RGB* col)
{
    int current_x = x;

    for (size_t i = 0; i < strlen(word); i++)
    {
        char ch = word[i];
        const Glyph* letter = matrix_letter_in_pxls(ch);

        if (!letter)
        {
            current_x += 2;
            continue;
        }

        matrix_display_letter(ch, current_x, y, col);

        current_x += letter->width;
        if (i < strlen(word) - 1)
        {
            current_x++;
        }
    }
}

void matrix_draw_horiz_line(int x, int y, int length, const RGB* col)
{
    int current_x = x;
    for (int i = 0; i < length; i++)
    {
        Pixel pxl = {current_x, y, *col};
        matrix_set_pixel(&pxl);
        current_x++;
    }
    debug("matrix_draw_horiz_line() of length %d at (%d, %d)", length, x, y);
}

void matrix_clear_horiz_line(int x, int y, int length)
{
    int current_x = x;
    for (int i = 0; i < length; i++)
    {
        matrix_clear_pixel(current_x, y);
        current_x++;
    }
    debug("matrix_clear_horiz_line() of length %d at (%d, %d)", length, x, y);
}

void matrix_draw_vert_line(int x, int y, int length, const RGB* col)
{
    int current_y = y;
    for (int i = 0; i < length; i++)
    {
        Pixel pxl = {x, current_y, *col};
        matrix_set_pixel(&pxl);
        current_y++;
    }
}

void matrix_display_word_icon_pair(const char* word, const RGB* word_col, const Glyph* icon,
                                   int offset_x)
{
    matrix_display_word(word, 1 + offset_x, 1, word_col);

    // Icon may be NULL
    if (strlen(word) <= 6 && icon)
    {
        // Start the icon at the end of the screen
        int icon_x = MATRIX_WIDTH - icon->width - 1 + offset_x;
        matrix_display_icon(icon, icon_x, 1, NULL);
    }
}

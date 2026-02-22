#include <matrix.h>

#include <const.h>
#include <global.h>
#include <macro.h>

#include <ctype.h>
#include <pico/time.h>
#include <stdbool.h>
#include <string.h>

#include <glyphs.h>
#include <stdio.h>

// ==========================
// STATIC
// ==========================

static uint32_t g_pixels[NUM_PIXELS];

static PixelState g_pixel_states[NUM_PIXELS];

static int matrix_xy_to_index(int input_x, int input_y)
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

static void matrix_set_brightness(RGB* col)
{
    col->r *= BRIGHTNESS_COEFFICIENT;
    col->g *= BRIGHTNESS_COEFFICIENT;
    col->b *= BRIGHTNESS_COEFFICIENT;
}

static uint32_t matrix_rgb_to_grb(RGB* col) { return (col->g << 16) | (col->r << 8) | col->b; }

static const Glyph* matrix_char_in_pxls(char ch)
{
    ch = toupper(ch);

    // Check for letters A-Z
    if (ch >= 'A' && ch <= 'Z')
    {
        return &ALPHABET[ch - 'A'];
    }

    // Check for digits 0-9
    if (ch >= '0' && ch <= '9')
    {
        return &NUMBERS[ch - '0'];
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
        case (char)0xB0:  // UTF-8 degree symbol ° (second byte)
            return &PUNCTUATION[5];
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
        index = matrix_xy_to_index(pxl->x, pxl->y);

        g_pixels[index] = matrix_rgb_to_grb(&dimmed);

        g_pixel_states[index].pixel = *pxl;
        g_pixel_states[index].is_set = true;
    }
}

void matrix_clear_pixel(int x, int y)
{
    if (IN_BOUNDS(x, y))
    {
        int index = matrix_xy_to_index(x, y);
        g_pixels[index] = 0;

        g_pixel_states[index].is_set = false;
    }
}

bool matrix_get_pixel_state(int index, Pixel* out_pixel)
{
    if (g_pixel_states[index].is_set)
    {
        *out_pixel = g_pixel_states[index].pixel;
        return true;
    }

    return false;
}

void matrix_save_state(PixelState* destination)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        destination[i] = g_pixel_states[i];
    }
}

void matrix_render_raw(const PixelState* source, int offset_x, int offset_y)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (source[i].is_set)
        {
            Pixel shifted_pxl = source[i].pixel;
            shifted_pxl.x += offset_x;
            shifted_pxl.y += offset_y;

            if (IN_BOUNDS(shifted_pxl.x, shifted_pxl.y))
            {
                matrix_set_pixel(&shifted_pxl);
                sleep_us(10);  // Prevents display corruption during rendering (heisenbug)
            }
        }
    }
}

void matrix_shift(Matrix* mtrx, int offset_x, int offset_y)
{
    PixelState temp_states[NUM_PIXELS];
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        temp_states[i] = g_pixel_states[i];
    }
    matrix_clear(mtrx);

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (temp_states[i].is_set)
        {
            Pixel shifted_pxl = temp_states[i].pixel;
            shifted_pxl.x += offset_x;
            shifted_pxl.y += offset_y;

            matrix_set_pixel(&shifted_pxl);
        }
    }
}

// Show the matrix current state
void matrix_show(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pio_sm_put_blocking(mtrx->pio, mtrx->sm, g_pixels[i] << 8u);
    }
}

// Clear the matrix of its current state
void matrix_clear(Matrix* mtrx)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        g_pixels[i] = 0;
        g_pixel_states[i].is_set = false;
    }
}

// Only writes letter to state, not matrix
void matrix_display_char(const char c, int x, int y, const RGB* col)
{
    const Glyph* ch = matrix_char_in_pxls(c);
    if (!ch)
    {
        debug("Cannot display invalid character: '%c'", c);
        return;
    }

    for (size_t i = 0; i < ch->pxl_count; i++)
    {
        int transformed_x = ch->pixels[i].x + x;
        int transformed_y = ch->pixels[i].y + y;

        if (!IN_BOUNDS(transformed_x, transformed_y)) continue;

        Pixel pxl = {transformed_x, transformed_y, *col};
        matrix_set_pixel(&pxl);
        sleep_us(10);  // Prevents display corruption during character rendering (heisenbug)
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
    size_t word_len = strlen(word);

    for (size_t i = 0; i < word_len; i++)
    {
        char ch = word[i];

        // Handle UTF-8 degree symbol (0xC2 0xB0) - skip first byte
        if ((unsigned char)ch == 0xC2 && i + 1 < word_len && (unsigned char)word[i + 1] == 0xB0)
        {
            continue;
        }

        const Glyph* chars = matrix_char_in_pxls(ch);

        if (!chars)
        {
            current_x += 2;
            continue;
        }

        matrix_display_char(ch, current_x, y, col);

        current_x += chars->width;
        if (i < word_len - 1)
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
}

void matrix_clear_horiz_line(int x, int y, int length)
{
    int current_x = x;
    for (int i = 0; i < length; i++)
    {
        matrix_clear_pixel(current_x, y);
        current_x++;
    }
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

int matrix_calculate_word_width_with_space(const char* word)
{
    int total_width = 0;
    for (size_t i = 0; word[i] != '\0'; i++)
    {
        // Handle UTF-8 degree symbol (0xC2 0xB0) - skip first byte
        if ((unsigned char)word[i] == 0xC2 && word[i + 1] != '\0' &&
            (unsigned char)word[i + 1] == 0xB0)
        {
            continue;
        }

        const Glyph* letter = matrix_char_in_pxls(word[i]);
        if (letter)
        {
            total_width += letter->width + 1;
        }
        else
        {
            total_width += 2;
        }
    }
    return total_width;
}

void matrix_display_word_icon_pair(const char* word, const RGB* word_col, const Glyph* icon,
                                   int offset_x)
{
    matrix_display_word(word, 1 + offset_x, 1, word_col);

    // Only show icon if word width <= 25 pixels
    int word_width = matrix_calculate_word_width_with_space(word);
    if (word_width <= 25 && icon)
    {
        // Start the icon at the end of the screen
        int icon_x = MATRIX_WIDTH - icon->width - 1 + offset_x;
        matrix_display_icon(icon, icon_x, 1, NULL);
    }
}

#include <displays.h>

#include <macro.h>
#include <matrix.h>
#include <stdio.h>

#include <ctype.h>
#include <string.h>

static const Glyph* displays_letter_in_pxls(char ch)
{
    ch = toupper(ch);

    if (ch < 'A' || ch > 'Z')
    {
        return NULL;
    }

    return &ALPHABET[ch - 'A'];
}

// Only writes letter to state, not matrix
void displays_letter(const char c, int x, int y, const RGB* col)
{
    const Glyph* letter = displays_letter_in_pxls(c);
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
    }
}

// Only writes word to state, not matrix
void displays_word(const char* word, int x, int y, const RGB* col)
{
    int current_x = x;

    for (size_t i = 0; i < strlen(word); i++)
    {
        char ch = word[i];
        const Glyph* letter = displays_letter_in_pxls(ch);

        // Determine where to write next letter, accounting for 1px gap
        int letter_width = letter->width;
        if (i > 0 && i != strlen(word) - 1)
        {
            // Gap in between each letter exluding first and last
            letter_width++;
        }
        current_x += letter_width;

        displays_letter(ch, current_x, y, col);
    }
}

void draw_horiz_line(int x, int y, int length, const RGB* col)
{
    int current_x = x;
    for (int i = 0; i < length; i++)
    {
        Pixel pxl = {current_x, y, *col};
        matrix_set_pixel(&pxl);
        current_x++;
    }
}

void draw_vert_line(int x, int y, int length, const RGB* col)
{
    int current_y = y;
    for (int i = 0; i < length; i++)
    {
        Pixel pxl = {x, current_y, *col};
        matrix_set_pixel(&pxl);
        current_y++;
    }
}

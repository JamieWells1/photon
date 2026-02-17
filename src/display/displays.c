#include <ctype.h>
#include <displays.h>
#include <macro.h>
#include <stdio.h>

static const Letter* displays_letter_in_pxls(char ch)
{
    ch = toupper(ch);

    if (ch < 'A' || ch > 'Z')
    {
        return NULL;
    }

    return &ALPHABET[ch - 'A'];
}

// Only writes letter to state, not matrix
void write_letter(const char c, int x, int y, RGB* col)
{
    const Letter* letter = displays_letter_in_pxls(c);
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
void write_word(const char* word, int x, int y, RGB* col)
{
    //
}

void draw_horiz_line(int x, int y, int length);
void draw_vert_line(int x, int y, int length);

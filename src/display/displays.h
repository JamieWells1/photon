// Pre-built display functions for the matrix

#ifndef DISPLAYS_H
#define DISPLAYS_H

#include <graphics.h>
#include <macro.h>

// ==========================
// TYPEDEFS
// ==========================
typedef struct
{
    const Pixel* pixels;
    size_t pxl_count;
    int width;
    int height;
} Glyph;

// When adding to this enum, also add the icon to the end of the ICONS array
typedef enum {
    PHOTON = 0,
    ICON_COUNT
} IconType;

// ==========================
// CONST VALUES
// ==========================

static const RGB DEFAULT_COLOUR = {255, 255, 255};
static const RGB LOGO_COLOUR = {100, 200, 250};

// ==========================
// FUNCTIONS
// ==========================

void displays_letter(const char c, int x, int y, const RGB* col);
void displays_icon(IconType icon_type, int x, int y, const RGB* col);
void displays_word(const char* word, int x, int y, const RGB* col);

void draw_horiz_line(int x, int y, int length, const RGB* col);
void draw_vert_line(int x, int y, int length, const RGB* col);

// ==========================
// LETTERS
// ==========================

static const Pixel LETTER_A[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_B[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR},
};

static const Pixel LETTER_C[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_D[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR},
};

static const Pixel LETTER_E[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_F[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
};

static const Pixel LETTER_G[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_H[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_I[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR},
    {1, 1, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_J[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR},
};

static const Pixel LETTER_K[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_L[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_M[] = {
    // 5x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR},
    {4, 0, DEFAULT_COLOUR}, {4, 1, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR}, {4, 3, DEFAULT_COLOUR},
    {4, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_N[] = {
    // 4x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {3, 0, DEFAULT_COLOUR},
    {3, 1, DEFAULT_COLOUR}, {3, 2, DEFAULT_COLOUR}, {3, 3, DEFAULT_COLOUR}, {3, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_O[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_P[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR},
};

static const Pixel LETTER_Q[] = {
    // 4x5
    {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR},
    {3, 2, DEFAULT_COLOUR}, {3, 3, DEFAULT_COLOUR}, {3, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_R[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_S[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_T[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {1, 0, DEFAULT_COLOUR}, {1, 1, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR},
    {1, 3, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
};

static const Pixel LETTER_U[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_V[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR},
};

static const Pixel LETTER_W[] = {
    // 5x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR}, {3, 3, DEFAULT_COLOUR},
    {4, 0, DEFAULT_COLOUR}, {4, 1, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR}, {4, 3, DEFAULT_COLOUR},
    {4, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_X[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

static const Pixel LETTER_Y[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 1, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR},
};

static const Pixel LETTER_Z[] = {
    // 3x5
    {0, 0, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
};

// ==========================
// ALPHABET LOOKUP TABLE
// ==========================

static const Glyph ALPHABET[26] = {
    {LETTER_A, ARRAY_SIZE(LETTER_A), 3, 5},  // A
    {LETTER_B, ARRAY_SIZE(LETTER_B), 3, 5},  // B
    {LETTER_C, ARRAY_SIZE(LETTER_C), 3, 5},  // C
    {LETTER_D, ARRAY_SIZE(LETTER_D), 3, 5},  // D
    {LETTER_E, ARRAY_SIZE(LETTER_E), 3, 5},  // E
    {LETTER_F, ARRAY_SIZE(LETTER_F), 3, 5},  // F
    {LETTER_G, ARRAY_SIZE(LETTER_G), 3, 5},  // G
    {LETTER_H, ARRAY_SIZE(LETTER_H), 3, 5},  // H
    {LETTER_I, ARRAY_SIZE(LETTER_I), 3, 5},  // I
    {LETTER_J, ARRAY_SIZE(LETTER_J), 3, 5},  // J
    {LETTER_K, ARRAY_SIZE(LETTER_K), 3, 5},  // K
    {LETTER_L, ARRAY_SIZE(LETTER_L), 3, 5},  // L
    {LETTER_M, ARRAY_SIZE(LETTER_M), 5, 5},  // M
    {LETTER_N, ARRAY_SIZE(LETTER_N), 4, 5},  // N
    {LETTER_O, ARRAY_SIZE(LETTER_O), 3, 5},  // O
    {LETTER_P, ARRAY_SIZE(LETTER_P), 3, 5},  // P
    {LETTER_Q, ARRAY_SIZE(LETTER_Q), 4, 5},  // Q
    {LETTER_R, ARRAY_SIZE(LETTER_R), 3, 5},  // R
    {LETTER_S, ARRAY_SIZE(LETTER_S), 3, 5},  // S
    {LETTER_T, ARRAY_SIZE(LETTER_T), 3, 5},  // T
    {LETTER_U, ARRAY_SIZE(LETTER_U), 3, 5},  // U
    {LETTER_V, ARRAY_SIZE(LETTER_V), 3, 5},  // V
    {LETTER_W, ARRAY_SIZE(LETTER_W), 5, 5},  // W
    {LETTER_X, ARRAY_SIZE(LETTER_X), 3, 5},  // X
    {LETTER_Y, ARRAY_SIZE(LETTER_Y), 3, 5},  // Y
    {LETTER_Z, ARRAY_SIZE(LETTER_Z), 3, 5},  // Z
};

// ==========================
// ICONS
// ==========================

static const Pixel ICON_PHOTON[] = {
    // Photon icon
    {0, 2, DEFAULT_COLOUR}, {0, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR}, {3, 0, DEFAULT_COLOUR}, {3, 2, DEFAULT_COLOUR},
    {4, 0, DEFAULT_COLOUR}, {4, 1, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR},
};

// ==========================
// ICON LOOKUP TABLE
// ==========================

static const Glyph ICONS[1] = {
    {ICON_PHOTON, ARRAY_SIZE(ICON_PHOTON), 5, 5},  // PHOTON ICON
};

#endif  // DISPLAYS_H

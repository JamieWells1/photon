// Pre-built display functions for the matrix

#ifndef DISPLAYS_H
#define DISPLAYS_H

#include <graphics.h>

// Const values
static const RGB DEFAULT_COLOUR = {255, 255, 255};
static const RGB LOGO_COLOUR = {100, 200, 250};

// Functions
void write_letter(const char c, int x, int y);
void write_word(const char* word, int x, int y);

void draw_horiz_line(int x, int y, int length);
void draw_vert_line(int x, int y, int length);

// Letters
static Pixel ALPHABET[] = {
    // A (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // B (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},

    // C (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // D (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},

    // E (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // F (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},

    // G (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // H (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // I (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 1, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // J (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},

    // K (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // L (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // M (5x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {3, 1, DEFAULT_COLOUR},
    {4, 0, DEFAULT_COLOUR},
    {4, 1, DEFAULT_COLOUR},
    {4, 2, DEFAULT_COLOUR},
    {4, 3, DEFAULT_COLOUR},
    {4, 4, DEFAULT_COLOUR},

    // N (4x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {3, 0, DEFAULT_COLOUR},
    {3, 1, DEFAULT_COLOUR},
    {3, 2, DEFAULT_COLOUR},
    {3, 3, DEFAULT_COLOUR},
    {3, 4, DEFAULT_COLOUR},

    // O (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // P (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},

    // Q (4x5)
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {3, 1, DEFAULT_COLOUR},
    {3, 2, DEFAULT_COLOUR},
    {3, 3, DEFAULT_COLOUR},
    {3, 4, DEFAULT_COLOUR},

    // R (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // S (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // T (3x5)
    {0, 0, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 1, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},

    // U (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // V (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},

    // W (5x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 2, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 3, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR},
    {3, 3, DEFAULT_COLOUR},
    {4, 0, DEFAULT_COLOUR},
    {4, 1, DEFAULT_COLOUR},
    {4, 2, DEFAULT_COLOUR},
    {4, 3, DEFAULT_COLOUR},
    {4, 4, DEFAULT_COLOUR},

    // X (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},

    // Y (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 1, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},

    // Z (3x5)
    {0, 0, DEFAULT_COLOUR},
    {0, 3, DEFAULT_COLOUR},
    {0, 4, DEFAULT_COLOUR},
    {1, 0, DEFAULT_COLOUR},
    {1, 2, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR},
    {2, 4, DEFAULT_COLOUR},
};

#endif  // DISPLAYS_H

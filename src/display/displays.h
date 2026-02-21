// Pre-built display functions for the matrix

#ifndef DISPLAYS_H
#define DISPLAYS_H

#include <macro.h>

#include <stddef.h>
#include <stdint.h>

// ==========================
// TYPEDEFS
// ==========================

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

typedef struct
{
    uint8_t x;
    uint8_t y;
    RGB col;
} Pixel;

typedef struct
{
    const Pixel* pixels;
    size_t pxl_count;
    int width;
    int height;
} Glyph;

// When adding to this enum, also add the icon to the end of the ICONS array
typedef enum
{
    PHOTON = 0,
    TICKERS = 1,
    TKR_UP = 2,
    TKR_DOWN = 3,
    GAMES = 4,
    TETRIS = 5,
    WEATHER = 6,
    SUNNY = 7,
    SUNNY_CLOUDY = 8,
    CLOUDY = 9,
    RAIN = 10,
    LIGHTNING = 11,
    SNOW = 12,
    ICON_COUNT
} IconType;

// ==========================
// COLOURS
// ==========================

static const RGB DEFAULT_COLOUR = {255, 255, 255};
static const RGB LOGO_COLOUR = {100, 200, 250};

static const RGB GREEN = {50, 200, 50};
static const RGB RED = {200, 50, 50};
static const RGB YELLOW = {250, 200, 50};
static const RGB LIGHT_BLUE = {150, 250, 250};

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

// LOGO

static const Pixel ICON_PHOTON[15] = {
    // Photon icon
    {0, 2, LOGO_COLOUR}, {0, 3, LOGO_COLOUR}, {0, 4, LOGO_COLOUR}, {1, 2, LOGO_COLOUR},
    {1, 4, LOGO_COLOUR}, {2, 0, LOGO_COLOUR}, {2, 1, LOGO_COLOUR}, {2, 2, LOGO_COLOUR},
    {2, 3, LOGO_COLOUR}, {2, 4, LOGO_COLOUR}, {3, 0, LOGO_COLOUR}, {3, 2, LOGO_COLOUR},
    {4, 0, LOGO_COLOUR}, {4, 1, LOGO_COLOUR}, {4, 2, LOGO_COLOUR},
};

// TICKERS

static const Pixel ICON_TICKERS[7] = {
    {0, 4, GREEN}, {0, 3, GREEN}, {1, 2, RED},   {2, 3, RED},
    {3, 2, GREEN}, {4, 1, GREEN}, {4, 0, GREEN},
};

static const Pixel ICON_TKR_UP[9] = {
    {0, 3, GREEN}, {1, 2, GREEN}, {1, 3, GREEN}, {2, 1, GREEN}, {2, 2, GREEN},
    {2, 3, GREEN}, {3, 2, GREEN}, {3, 3, GREEN}, {4, 3, GREEN},
};

static const Pixel ICON_TKR_DOWN[9] = {
    {0, 3, RED}, {1, 2, RED}, {1, 3, RED}, {2, 1, RED}, {2, 2, RED},
    {2, 3, RED}, {3, 2, RED}, {3, 3, RED}, {4, 3, RED},
};

// GAMES

static const Pixel ICON_GAMES[15] = {
    {0, 1, LIGHT_BLUE}, {0, 2, LIGHT_BLUE}, {0, 3, LIGHT_BLUE}, {0, 4, LIGHT_BLUE},
    {1, 0, LIGHT_BLUE}, {1, 1, GREEN},      {1, 2, LIGHT_BLUE}, {2, 1, LIGHT_BLUE},
    {3, 0, LIGHT_BLUE}, {3, 1, RED},        {3, 2, LIGHT_BLUE}, {4, 1, LIGHT_BLUE},
    {4, 2, LIGHT_BLUE}, {4, 3, LIGHT_BLUE}, {4, 4, LIGHT_BLUE},
};

static const Pixel ICON_TETRIS[17] = {
    {3, 0, LIGHT_BLUE}, {4, 0, LIGHT_BLUE}, {3, 1, LIGHT_BLUE}, {4, 1, LIGHT_BLUE}, {0, 1, YELLOW},
    {1, 1, YELLOW},     {0, 2, YELLOW},     {0, 3, GREEN},      {0, 4, GREEN},      {1, 2, YELLOW},
    {3, 2, RED},        {4, 2, RED},        {2, 3, RED},        {3, 3, RED},        {1, 4, GREEN},
    {2, 4, GREEN},      {3, 4, GREEN},
};

// WEATHER

static const Pixel ICON_SUNNY[17] = {
    {0, 0, YELLOW}, {0, 2, YELLOW}, {0, 4, YELLOW}, {2, 0, YELLOW}, {4, 0, YELLOW}, {4, 2, YELLOW},
    {4, 4, YELLOW}, {2, 4, YELLOW}, {1, 1, YELLOW}, {1, 2, YELLOW}, {1, 3, YELLOW}, {2, 1, YELLOW},
    {2, 2, YELLOW}, {2, 3, YELLOW}, {3, 1, YELLOW}, {3, 2, YELLOW}, {3, 3, YELLOW},
};

static const Pixel ICON_SUNNY_CLOUDY[18] = {
    {0, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR},
    {1, 4, DEFAULT_COLOUR}, {2, 0, YELLOW},         {2, 1, YELLOW},         {2, 2, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR}, {3, 0, YELLOW},         {3, 1, YELLOW},
    {3, 2, YELLOW},         {3, 3, DEFAULT_COLOUR}, {3, 4, DEFAULT_COLOUR}, {4, 0, YELLOW},
    {4, 1, YELLOW},         {4, 2, YELLOW},
};

static const Pixel ICON_CLOUDY[12] = {
    {0, 4, DEFAULT_COLOUR}, {1, 1, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR}, {1, 4, DEFAULT_COLOUR},
    {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 3, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
    {3, 0, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR}, {3, 4, DEFAULT_COLOUR}, {4, 1, DEFAULT_COLOUR},
};

static const Pixel ICON_RAIN[15] = {
    {0, 2, DEFAULT_COLOUR}, {0, 4, LIGHT_BLUE},     {1, 1, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR},
    {1, 3, LIGHT_BLUE},     {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR},
    {3, 5, LIGHT_BLUE},     {3, 0, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR}, {3, 2, DEFAULT_COLOUR},
    {4, 3, LIGHT_BLUE},     {4, 1, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR},
};

static const Pixel ICON_LIGHTNING[15] = {
    {0, 2, DEFAULT_COLOUR}, {0, 4, YELLOW},         {1, 1, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR},
    {1, 3, YELLOW},         {2, 0, DEFAULT_COLOUR}, {2, 1, DEFAULT_COLOUR}, {2, 2, DEFAULT_COLOUR},
    {2, 5, YELLOW},         {3, 0, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR}, {3, 2, DEFAULT_COLOUR},
    {3, 3, YELLOW},         {4, 1, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR},
};

static const Pixel ICON_SNOW[17] = {
    {0, 0, DEFAULT_COLOUR}, {2, 0, DEFAULT_COLOUR}, {4, 0, DEFAULT_COLOUR}, {1, 1, DEFAULT_COLOUR},
    {2, 1, DEFAULT_COLOUR}, {3, 1, DEFAULT_COLOUR}, {0, 2, DEFAULT_COLOUR}, {1, 2, DEFAULT_COLOUR},
    {2, 2, DEFAULT_COLOUR}, {3, 2, DEFAULT_COLOUR}, {4, 2, DEFAULT_COLOUR}, {1, 3, DEFAULT_COLOUR},
    {2, 3, DEFAULT_COLOUR}, {3, 3, DEFAULT_COLOUR}, {0, 4, DEFAULT_COLOUR}, {2, 4, DEFAULT_COLOUR},
    {4, 4, DEFAULT_COLOUR},
};

// ==========================
// ICON LOOKUP TABLE
// ==========================

static const Glyph ICONS_ARR[ICON_COUNT] = {
    {ICON_PHOTON, ARRAY_SIZE(ICON_PHOTON), 5, 5},              // 0: PHOTON
    {ICON_TICKERS, ARRAY_SIZE(ICON_TICKERS), 5, 5},            // 1: TICKERS
    {ICON_TKR_UP, ARRAY_SIZE(ICON_TKR_UP), 5, 5},              // 2: TKR_UP
    {ICON_TKR_DOWN, ARRAY_SIZE(ICON_TKR_DOWN), 5, 5},          // 3: TKR_DOWN
    {ICON_GAMES, ARRAY_SIZE(ICON_GAMES), 5, 5},                // 4: GAMES
    {ICON_TETRIS, ARRAY_SIZE(ICON_TETRIS), 5, 5},              // 5: TETRIS
    {ICON_SUNNY_CLOUDY, ARRAY_SIZE(ICON_SUNNY_CLOUDY), 5, 5},  // 6: WEATHER
    {ICON_SUNNY, ARRAY_SIZE(ICON_SUNNY), 5, 5},                // 7: SUNNY
    {ICON_SUNNY_CLOUDY, ARRAY_SIZE(ICON_SUNNY_CLOUDY), 5, 5},  // 8: SUNNY_CLOUDY
    {ICON_CLOUDY, ARRAY_SIZE(ICON_CLOUDY), 5, 5},              // 9: CLOUDY
    {ICON_RAIN, ARRAY_SIZE(ICON_RAIN), 5, 5},                  // 10: RAIN
    {ICON_LIGHTNING, ARRAY_SIZE(ICON_LIGHTNING), 5, 5},        // 11: LIGHTNING
    {ICON_SNOW, ARRAY_SIZE(ICON_SNOW), 5, 5},                  // 12: SNOW
};

#endif  // DISPLAYS_H

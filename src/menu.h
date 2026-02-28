#ifndef MENU_H
#define MENU_H

#include <glyphs.h>
#include <input.h>
#include <matrix.h>

// Main modes
typedef enum
{
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
} Mode;

#define TKR_START 0
#define TKR_COUNT 3
#define GAME_START (TKR_START + TKR_COUNT)
#define GAME_COUNT 1
#define TEMP_START (GAME_START + GAME_COUNT)
#define TEMP_COUNT 2

// Structs
typedef struct
{
    Mode mode;
    const char* text;
    const Glyph* icon;
} Menu;

static const Menu MENUS[] = {
    {MENU_TICKERS, "STOCKS", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

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
    MENU_RESET = 3,
} Mode;

#define TKR_START 0
#define TKR_END 3
#define GAME_START (TKR_START + TKR_END)
#define GAME_END 1
#define TEMP_START (GAME_START + GAME_END)
#define TEMP_END 2
#define RESET_START (TEMP_START + TEMP_END)
#define RESET_END 1

// Structs
typedef struct
{
    Mode mode;
    const char* text;
    const Glyph* icon;
} Menu;

static const Menu MENUS[] = {
    {MENU_TICKERS, "CRYPTO", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
    {MENU_RESET, "RESET", &ICONS_ARR[POWER]},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

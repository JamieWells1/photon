#ifndef MENU_H
#define MENU_H

#include <glyphs.h>
#include <input.h>
#include <matrix.h>

// Main modes
typedef enum
{
    MENU_CRYPTO = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
    MENU_CONFIG = 3,
    MENU_RESET = 4,
} Mode;

#define TKR_START 0
#define TKR_END 3
#define GAME_START (TKR_START + TKR_END)
#define GAME_END 1
#define TEMP_START (GAME_START + GAME_END)
#define TEMP_END 2
#define CONFIG_START (TEMP_START + TEMP_END)
#define CONFIG_END 1
#define RESET_START (CONFIG_START + CONFIG_END)
#define RESET_END 1

// Structs
typedef struct
{
    Mode mode;
    const char* text;
    const Glyph* icon;
} Menu;

static const Menu MENUS[] = {
    {MENU_CRYPTO, "CRYPTO", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
    {MENU_CONFIG, "CONFIG", &ICONS_ARR[DATA]},
    {MENU_RESET, "RESET", &ICONS_ARR[POWER]},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

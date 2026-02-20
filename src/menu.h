#ifndef MENU_H
#define MENU_H

#include <displays.h>
#include <input.h>
#include <matrix.h>

// Main modes
typedef enum
{
    MENU_TICKERS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
} MainMode;

// Sub-modes
typedef enum
{
    // Ticker submodes
    TKR_BTC = 0,
    TKR_XAU,
    TKR_XAG,

    // Game submodes
    GAME_TETRIS,
    GAME_SNAKE,
    GAME_PONG,

    // Temperature submodes
    TEMP_CURRENT,
    TEMP_HOURLY,
} SubMode;

#define TKR_START 0
#define TKR_COUNT 3
#define GAME_START (TKR_START + TKR_COUNT)
#define GAME_COUNT 3
#define TEMP_START (GAME_START + GAME_COUNT)
#define TEMP_COUNT 2

// Structs
typedef struct
{
    const MainMode mode;
    const char* text;
    const Glyph* icon;
} MenuMode;

typedef struct
{
    const SubMode mode;
    const char* text;
    const Glyph* icon;
} SubMenuMode;

// Menu state struct
typedef struct
{
    MainMode main_mode;
    SubMode sub_mode;
} MenuState;

static const MenuMode MENU_MODES[3] = {
    {MENU_TICKERS, "TICKERS", &ICONS_ARR[TICKERS]},
    {MENU_GAMES, "GAMES", &ICONS_ARR[GAMES]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
};

static const SubMenuMode SUB_MENU_MODES[6] = {
    // Icons may be null if they are not needed.
    {TKR_BTC, "BTC", NULL},      {TKR_XAU, "XAU", NULL},
    {TKR_XAG, "XAG", NULL},      {GAME_TETRIS, "TETRIS", &ICONS_ARR[TETRIS]},
    {TEMP_CURRENT, "NOW", NULL}, {TEMP_HOURLY, "00", NULL},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

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
    const IconType icon;
} MenuMode;

typedef struct
{
    const SubMode mode;
    const char* text;
} SubMenuMode;

// Menu state struct
typedef struct
{
    MainMode main_mode;
    SubMode sub_mode;
} MenuState;

static const MenuMode MENU_MODES[3] = {
    {MENU_TICKERS, "TICKERS", TICKERS},
    {MENU_GAMES, "GAMES", GAMES},
    {MENU_WEATHER, "TEMP", WEATHER},
};

static const SubMenuMode SUB_MENU_MODES[6] = {
    {TKR_BTC, "BTC"},        {TKR_XAU, "XAU"},      {TKR_XAG, "XAG"},
    {GAME_TETRIS, "TETRIS"}, {TEMP_CURRENT, "NOW"}, {TEMP_HOURLY, "00"},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

#ifndef MENU_H
#define MENU_H

#include <displays.h>
#include <input.h>
#include <matrix.h>

// Main modes
typedef enum
{
    MENU_STOCKS = 0,
    MENU_GAMES = 1,
    MENU_WEATHER = 2,
} MainMode;

// Sub-modes
typedef enum
{
    TKR_BTC = 0,
    TKR_XAU = 1,
    TKR_XAG = 2,
    GAME_TETRIS = 3,
    TEMP_CURRENT = 4,
    TEMP_HOURLY = 5,
} SubMode;

// Menu state struct
typedef struct
{
    MainMode main_mode;
    SubMode sub_mode;
} MenuState;

// Structs
typedef struct
{
    const MainMode mode;
    const char* text;
    const IconType icon;
} MenuMode;

typedef struct
{
    const SubMode sub_mode;
    const char* text;
} SubMenuMode;

static const MenuMode MENU_MODES[3] = {
    {MENU_STOCKS, "STOCKS", STOCKS},
    {MENU_GAMES, "GAMES", GAMES},
    {MENU_WEATHER, "TEMP", WEATHER},
};

static const SubMenuMode SUB_MENU_MODES[6] = {
    {TKR_BTC, "BTC"},        {TKR_XAU, "XAU"},      {TKR_XAG, "XAG"},
    {GAME_TETRIS, "TETRIS"}, {TEMP_CURRENT, "NOW"}, {TEMP_HOURLY, "00"},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

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
} ModeType;

// Games sub-modes
typedef enum
{
    GAME_TETRIS
} GameMode;

// Stocks sub-modes
typedef enum
{
    STOCK_BTC,
    STOCK_NVDA,
    STOCK_AAPL
} StockMode;

// Menu state struct
typedef struct
{
    ModeType main_mode;
    GameMode sub_game_mode;
    StockMode sub_stock_mode;
} MenuState;

// Structs
typedef struct
{
    const ModeType mode;
    const char* name;
    const IconType icon;
} MenuMode;

static const MenuMode MENU_MODES[3] = {
    {MENU_STOCKS, "STOCKS", STOCKS},
    {MENU_GAMES, "GAMES", GAMES},
    {MENU_WEATHER, "WEATHER", WEATHER},
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

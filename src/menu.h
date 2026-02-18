#ifndef MENU_H
#define MENU_H

#include <input.h>
#include <matrix.h>
#include <displays.h>

// Main modes
typedef enum
{
    MENU_GAMES,
    MENU_STOCKS,
    MENU_WEATHER
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

static const MenuMode MENU_MODES[] = {
    {MENU_STOCKS, "STOCKS", STOCKS}
};

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

#ifndef MENU_H
#define MENU_H

#include <input.h>
#include <matrix.h>

// Main modes
typedef enum
{
    MENU_GAMES,
    MENU_STOCKS,
    MENU_WEATHER
} MenuMode;

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
    MenuMode mode;
    GameMode game_mode;
    StockMode stock_mode;
} MenuState;

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx);

#endif  // MENU_H

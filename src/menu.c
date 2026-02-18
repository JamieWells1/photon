#include <menu.h>

#include <const.h>
#include <graphics.h>
#include <input.h>
#include <global.h>

#include <stdlib.h>
#include <pico/time.h>

void menu_set_initial_display()
{
    MENU_STATE.main_mode = MENU_STOCKS;
    MENU_STATE.sub_game_mode = GAME_TETRIS;
    MENU_STATE.sub_stock_mode = STOCK_BTC;

    
}

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    menu_set_initial_display();
}

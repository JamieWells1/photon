/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#include <menu_markets.h>

#include <const.h>
#include <crypto.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <menu_common.h>
#include <menu_main.h>
#include <portfolio.h>
#include <stock.h>

#include <glyphs.h>
#include <pico/time.h>
#include <stdio.h>

typedef enum
{
    SUBMENU_INVEST = 0,
    SUBMENU_STOCKS = 1,
    SUBMENU_CRYPTO = 2,
} MarketsSubmenu;

typedef struct
{
    MarketsSubmenu submenu;
    const char* text;
    const Glyph* icon;
} MarketsMenu;

#define INVEST_START 0
#define INVEST_END 1
#define STK_START (INVEST_START + INVEST_END)
#define STK_END 1
#define CRYPTO_START (STK_START + STK_END)
#define CRYPTO_END 3

static const MarketsMenu MARKETS_MENUS[] = {
    {SUBMENU_INVEST, "INVEST", &ICONS_ARR[INVEST]},
    {SUBMENU_STOCKS, "STOCKS", &ICONS_ARR[STOCKS_CANDLE]},
    {SUBMENU_CRYPTO, "CRYPTO", &ICONS_ARR[CRYPTO]},
};

static bool g_markets_display_initialized = false;
static MarketsSubmenu g_markets_selection = SUBMENU_INVEST;
static bool g_in_markets_submenu = false;
static Mode g_markets_current_mode = INVEST_START;

static int g_invest_offset = 0;
static int g_stock_offset = 0;
static int g_crypto_offset = 0;

static bool g_markets_menu_ready = false;
static bool g_invest_submenu_ready = false;
static bool g_stocks_submenu_ready = false;
static bool g_crypto_submenu_ready = false;

static void menu_markets_set_initial_display(Matrix* mtrx)
{
    matrix_clear(mtrx);
    MarketsMenu currentMode = MARKETS_MENUS[g_markets_selection];
    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_markets_display_initialized = true;
}

static void menu_markets_slide_right(Matrix* mtrx)
{
    int index_next = (g_markets_selection + 1) % ARRAY_SIZE(MARKETS_MENUS);
    MarketsMenu nextMode = MARKETS_MENUS[index_next];

    debug(INFO, "Markets menu: %s -> %s", MARKETS_MENUS[g_markets_selection].text, nextMode.text);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_right(mtrx, current_state, next_state);

    g_markets_selection = index_next;
    g_in_markets_submenu = false;
    g_markets_display_initialized = true;
    g_markets_menu_ready = false;
    g_invest_submenu_ready = false;
    g_stocks_submenu_ready = false;
    g_crypto_submenu_ready = false;
}

static void menu_markets_slide_left(Matrix* mtrx)
{
    int index_previous;
    if (g_markets_selection == 0)
    {
        index_previous = ARRAY_SIZE(MARKETS_MENUS) - 1;
    }
    else
    {
        index_previous = (g_markets_selection - 1);
    }
    MarketsMenu nextMode = MARKETS_MENUS[index_previous];

    debug(INFO, "Markets menu: %s -> %s", MARKETS_MENUS[g_markets_selection].text, nextMode.text);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_left(mtrx, current_state, next_state);

    g_markets_selection = index_previous;
    g_in_markets_submenu = false;
    g_markets_display_initialized = true;
    g_markets_menu_ready = false;
    g_invest_submenu_ready = false;
    g_stocks_submenu_ready = false;
    g_crypto_submenu_ready = false;
}

static void menu_markets_enter_mode(MarketsMenu submenu, Matrix* mtrx)
{
    Mode target_menu;

    if (submenu.submenu == SUBMENU_INVEST)
    {
        target_menu = INVEST_START;
    }
    else if (submenu.submenu == SUBMENU_STOCKS)
    {
        target_menu = STK_START;
    }
    else if (submenu.submenu == SUBMENU_CRYPTO)
    {
        target_menu = CRYPTO_START;
    }
    else
    {
        return;
    }

    if (!g_in_markets_submenu || g_markets_current_mode != target_menu)
    {
        debug(TRACE, "Entering markets mode: target=%d", target_menu);
        g_in_markets_submenu = true;
        g_markets_current_mode = target_menu;
    }

    matrix_clear(mtrx);

    if (g_markets_current_mode == INVEST_START)
    {
        portfolio_display(mtrx, &g_invest_offset);
    }
    else if (g_markets_current_mode == STK_START)
    {
        stock_display(mtrx, &g_stock_offset);
    }
    else if (g_markets_current_mode == CRYPTO_START)
    {
        crypto_display(mtrx, &g_crypto_offset);
    }
}

static void menu_markets_invest_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_invest_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_invest_submenu_ready = true;
            debug(DEBUG, "All buttons released, invest submenu ready for input");
        }
        return;
    }

    if (input_rtr_long_pressed(rtr))
    {
        menu_main_exit_submenu(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        portfolio_next_metric(mtrx);
    }

    if (input_rtr_anti_cw(rtr))
    {
        portfolio_prev_metric(mtrx);
    }
}

static void menu_markets_stocks_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_stocks_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_stocks_submenu_ready = true;
            debug(DEBUG, "All buttons released, stocks submenu ready for input");
        }
        return;
    }

    if (input_rtr_long_pressed(rtr))
    {
        menu_main_exit_submenu(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        g_stock_offset += 1;
    }
}

static void menu_markets_crypto_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_crypto_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_crypto_submenu_ready = true;
            debug(DEBUG, "All buttons released, crypto submenu ready for input");
        }
        return;
    }

    if (input_rtr_long_pressed(rtr))
    {
        menu_main_exit_submenu(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        g_crypto_offset += 1;
    }
}

void menu_markets_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_in_markets_submenu)
    {
        if (!g_markets_menu_ready)
        {
            bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                                 rtr->current_sw_state == true);

            if (all_released)
            {
                g_markets_menu_ready = true;
                debug(DEBUG, "All buttons released, markets menu ready for input");
            }
            return;
        }

        bool cw = input_rtr_cw(rtr);
        bool acw = input_rtr_anti_cw(rtr);

        if (cw)
        {
            menu_markets_slide_right(mtrx);
        }

        if (acw)
        {
            menu_markets_slide_left(mtrx);
        }

        if (input_rtr_long_pressed(rtr))
        {
            menu_main_exit_submenu(mtrx);
        }
    }

    if (input_rtr_pressed(rtr) && !g_in_markets_submenu && g_markets_menu_ready)
    {
        debug(DEBUG, "Button pressed in markets menu, entering mode=%d", g_markets_selection);
        menu_markets_enter_mode(MARKETS_MENUS[g_markets_selection], mtrx);
    }

    if (g_in_markets_submenu)
    {
        if (g_markets_current_mode == INVEST_START)
        {
            menu_markets_invest_detect_inputs(btns, rtr, mtrx);
        }
        else if (g_markets_current_mode == STK_START)
        {
            menu_markets_stocks_detect_inputs(btns, rtr, mtrx);
        }
        else if (g_markets_current_mode == CRYPTO_START)
        {
            menu_markets_crypto_detect_inputs(btns, rtr, mtrx);
        }
    }
}

void menu_markets_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_markets_display_initialized)
    {
        menu_markets_set_initial_display(mtrx);
        matrix_show(mtrx);
    }

    if (g_in_markets_submenu)
    {
        menu_markets_enter_mode(MARKETS_MENUS[g_markets_selection], mtrx);
    }
}

void menu_markets_cleanup(void)
{
    portfolio_cleanup();
    stock_cleanup();
    crypto_cleanup();

    g_markets_display_initialized = false;
    g_in_markets_submenu = false;
    g_markets_selection = SUBMENU_INVEST;
    g_invest_offset = 0;
    g_stock_offset = 0;
    g_crypto_offset = 0;
    g_markets_menu_ready = false;
    g_invest_submenu_ready = false;
    g_stocks_submenu_ready = false;
    g_crypto_submenu_ready = false;
}

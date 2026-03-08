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
    {SUBMENU_INVEST, "INVEST", &ICONS_ARR[STOCKS]},
    {SUBMENU_STOCKS, "STOCKS", &ICONS_ARR[STOCKS]},
    {SUBMENU_CRYPTO, "CRYPTO", &ICONS_ARR[CRYPTO]},
};

static bool g_markets_display_initialized = false;
static MarketsSubmenu g_markets_selection = SUBMENU_INVEST;
static bool g_in_markets_submenu = false;
static Mode g_markets_current_mode = INVEST_START;

static int g_invest_offset = 0;
static int g_stock_offset = 0;
static int g_crypto_offset = 0;

static void menu_markets_set_initial_display(void)
{
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
    if (input_rtr_long_pressed(rtr))
    {
        menu_main_exit_submenu(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        g_invest_offset += 1;
    }
}

static void menu_markets_stocks_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
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

    if (input_any_btn_pressed(btns, rtr) && !g_in_markets_submenu)
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
        menu_markets_set_initial_display();
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
}

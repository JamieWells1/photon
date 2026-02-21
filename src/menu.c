#include <menu.h>

#include <const.h>
#include <game.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <stdio.h>
#include <ticker.h>
#include <weather.h>

#include <displays.h>
#include <pico/time.h>
#include <stdlib.h>

static bool g_display_initialized = false;
static int g_tick = 0;
static bool g_underscore_on = true;
static bool g_in_submenu = false;
static SubMode g_current_submenu = TKR_BTC;

static void menu_set_underscore(bool underscore_on)
{
    int uscr_starting_x = 1;
    int uscr_starting_y = 7;
    int uscr_length = 30;

    if (underscore_on)
    {
        matrix_draw_horiz_line(uscr_starting_x, uscr_starting_y, uscr_length, &DEFAULT_COLOUR);
    }
    else
    {
        matrix_clear_horiz_line(uscr_starting_x, uscr_starting_y, uscr_length);
    }
}
static void menu_set_initial_display()
{
    // MenuState already initialised in global.c
    Menu currentMode = MENUS[MENU_STATE.main_mode];

    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_display_initialized = true;
}

static void reset_states(Matrix* mtrx)
{
    g_display_initialized = false;
    g_underscore_on = true;
    g_tick = 0;
    g_in_submenu = false;

    matrix_clear(mtrx);
    menu_set_initial_display();
    menu_set_underscore(true);
    matrix_show(mtrx);
}

static void slide_menu(Matrix* mtrx, Menu currentMode, Menu nextMode, int direction)
{
    int sliding_coefficient = -(MATRIX_WIDTH / 2);
    for (int offset = 0; offset <= MATRIX_WIDTH; offset++)
    {
        matrix_clear(mtrx);

        // Move current out
        matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon,
                                      direction * offset);
        // Move next in
        matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon,
                                      direction * offset - (direction * MATRIX_WIDTH));

        matrix_show(mtrx);
        // Quadratic smoothing for sliding menu
        sleep_ms((sliding_coefficient * sliding_coefficient) / 10);
        sliding_coefficient++;
    }
}

// From clockwise turn
static void slide_main_menu_right(Matrix* mtrx)
{
    menu_set_underscore(true);

    Menu currentMode = MENUS[MENU_STATE.main_mode];
    // Last element -> wrap to first element
    int index_next = (MENU_STATE.main_mode + 1) % ARRAY_SIZE(MENUS);
    Menu nextMode = MENUS[index_next];

    slide_menu(mtrx, currentMode, nextMode, -1);
    MENU_STATE.main_mode = index_next;
    reset_states(mtrx);
}

// From anti-clockwise turn
static void slide_main_menu_left(Matrix* mtrx)
{
    menu_set_underscore(true);

    Menu currentMode = MENUS[MENU_STATE.main_mode];

    int index_previous;
    if (MENU_STATE.main_mode == 0)
    {
        index_previous = ARRAY_SIZE(MENUS) - 1;
    }
    else
    {
        index_previous = (MENU_STATE.main_mode - 1);
    }
    Menu nextMode = MENUS[index_previous];

    slide_menu(mtrx, currentMode, nextMode, 1);
    MENU_STATE.main_mode = index_previous;
    reset_states(mtrx);
}

static void menu_enter_sub_menu(MenuState menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
    matrix_clear(mtrx);
    SubMode target_submenu;

    if (menu_state.main_mode == MENU_TICKERS)
    {
        target_submenu = TKR_START;
    }
    else if (menu_state.main_mode == MENU_GAMES)
    {
        target_submenu = GAME_START;
    }
    else if (menu_state.main_mode == MENU_WEATHER)
    {
        target_submenu = TEMP_START;
    }
    else
    {
        return;
    }

    // Only initialise if entering a new submenu or first time
    if (!g_in_submenu || g_current_submenu != target_submenu)
    {
        g_in_submenu = true;
        g_current_submenu = target_submenu;
        g_display_initialized = false;

        menu_state.sub_mode = target_submenu;

        if (menu_state.main_mode == MENU_TICKERS)
        {
            ticker_display(SUB_MENUS[menu_state.sub_mode]);
        }
        else if (menu_state.main_mode == MENU_GAMES)
        {
            game_display(SUB_MENUS[menu_state.sub_mode]);
        }
        else if (menu_state.main_mode == MENU_WEATHER)
        {
            weather_display(SUB_MENUS[menu_state.sub_mode]);
        }

        g_display_initialized = true;
    }

    Button* btn_left = &btns[0];
    Button* btn_right = &btns[1];

    if (input_btn_pressed(btn_left))
    {
        // Back to main menu
        reset_states(mtrx);
    }

    if (input_btn_pressed(btn_right))
    {
        // TODO: Go into mode (if enabled)
        // Stocks: false
        // Games: true
        // Weather: false
    }

    if (input_rtr_cw(rtr))
    {
        // TODO: Next sub-mode
    }

    matrix_show(mtrx);
}

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // Reset to avoid integer overflow for long running idle programs
    if (g_tick >= 100000) g_tick = 0;
    g_tick++;

    if (!g_display_initialized)
    {
        menu_set_initial_display();
        matrix_show(mtrx);
    }

    if (g_tick % 50 == 0)
    {
        menu_set_underscore(g_underscore_on);
        g_underscore_on = !g_underscore_on;
        matrix_show(mtrx);
    }

    if (!g_in_submenu && input_rtr_cw(rtr))
    {
        slide_main_menu_right(mtrx);
    }

    if (!g_in_submenu && input_rtr_anti_cw(rtr))
    {
        slide_main_menu_left(mtrx);
    }

    if (input_any_btn_pressed(btns, rtr))
    {
        debug("BUTTON PRESSED! ENTERING SUBMENU");
        if (!g_in_submenu)
        {
            menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
        }
        else
        {
            menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
        }
    }
}

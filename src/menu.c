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

#include <glyphs.h>
#include <pico/time.h>
#include <stdlib.h>

static bool g_display_initialized = false;
static int g_tick = 0;
static bool g_underscore_on = true;
static bool g_in_submenu = false;
static Mode g_current_mode = MENU_TICKERS;

static int g_weather_hour_offset = 0;

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
static void menu_set_initial_display(void)
{
    // MenuState already initialised in global.c
    Menu currentMode = MENUS[MENU_STATE.mode];

    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_display_initialized = true;
}

static void menu_reset_states(Matrix* mtrx)
{
    ticker_cleanup();
    weather_cleanup();

    g_display_initialized = false;
    g_underscore_on = true;
    g_tick = 0;
    g_in_submenu = false;
    g_weather_hour_offset = 0;  // Reset to current hour

    matrix_clear(mtrx);
    menu_set_initial_display();
    menu_set_underscore(true);
    matrix_show(mtrx);
}

// From clockwise turn
static void menu_main_slide_right(Matrix* mtrx)
{
    menu_set_underscore(true);

    // Last element -> wrap to first element
    int index_next = (MENU_STATE.mode + 1) % ARRAY_SIZE(MENUS);
    Menu nextMode = MENUS[index_next];

    // Save current state
    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    // Render next state
    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    // Animate slide
    matrix_slide_right(mtrx, current_state, next_state);

    MENU_STATE.mode = index_next;
    menu_reset_states(mtrx);
}

// From anti-clockwise turn
static void menu_main_slide_left(Matrix* mtrx)
{
    menu_set_underscore(true);

    int index_previous;
    if (MENU_STATE.mode == 0)
    {
        index_previous = ARRAY_SIZE(MENUS) - 1;
    }
    else
    {
        index_previous = (MENU_STATE.mode - 1);
    }
    Menu nextMode = MENUS[index_previous];

    // Save current state
    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    // Render next state
    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    // Animate slide
    matrix_slide_left(mtrx, current_state, next_state);

    MENU_STATE.mode = index_previous;
    menu_reset_states(mtrx);
}

static void menu_enter_sub_menu(Menu menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
    Mode target_menu;

    if (menu_state.mode == MENU_TICKERS)
    {
        target_menu = TKR_START;
    }
    else if (menu_state.mode == MENU_GAMES)
    {
        target_menu = GAME_START;
    }
    else if (menu_state.mode == MENU_WEATHER)
    {
        target_menu = TEMP_START;
    }
    else
    {
        return;
    }

    // Only initialise if entering a new submenu or first time
    if (!g_in_submenu || g_current_mode != target_menu)
    {
        g_in_submenu = true;
        g_current_mode = target_menu;
        g_display_initialized = false;

        menu_state.mode = target_menu;

        g_display_initialized = true;
    }

    if (menu_state.mode == MENU_TICKERS)
    {
        ticker_display(mtrx);
    }
    else if (menu_state.mode == MENU_GAMES)
    {
        game_display(mtrx);
    }
    else if (menu_state.mode == MENU_WEATHER)
    {
        weather_display(mtrx, &g_weather_hour_offset);
    }
}

static void menu_sub_menu_stocks_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // TODO
}

static void menu_sub_menu_games_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // TODO
}

static void menu_sub_menu_weather_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_btn_pressed(&btns[0]))
    {
        menu_reset_states(mtrx);
    }

    int max_offset = weather_hours_fetched() - weather_current_hour_index() - 1;
    if (input_rtr_cw(rtr) && g_weather_hour_offset < max_offset)
    {
        // Save current display state
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset++;

        weather_display(mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        matrix_slide_right(mtrx, current_state, next_state);
    }

    if (input_rtr_anti_cw(rtr) && g_weather_hour_offset > 0)
    {
        // Save current display state
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset--;
        if (g_weather_hour_offset < 0) g_weather_hour_offset = 0;

        weather_display(mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        matrix_slide_left(mtrx, current_state, next_state);
    }
}

static void menu_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_in_submenu)
    {
        bool cw = input_rtr_cw(rtr);
        bool acw = input_rtr_anti_cw(rtr);

        if (cw || acw)
        {
            debug(TRACE, "menu_detect_inputs: g_in_submenu=%d, cw=%d, acw=%d\n", g_in_submenu, cw,
                  acw);
        }

        if (cw)
        {
            debug(TRACE, "Calling menu_main_slide_right\n");
            menu_main_slide_right(mtrx);
        }

        if (acw)
        {
            debug(TRACE, "Calling menu_main_slide_left\n");
            menu_main_slide_left(mtrx);
        }
    }

    if (input_any_btn_pressed(btns, rtr) && !g_in_submenu)
    {
        menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    if (g_in_submenu)
    {
        if (MENU_STATE.mode == MENU_TICKERS)
        {
            menu_sub_menu_stocks_detect_inputs(btns, rtr, mtrx);
        }
        if (MENU_STATE.mode == MENU_GAMES)
        {
            menu_sub_menu_games_detect_inputs(btns, rtr, mtrx);
        }
        if (MENU_STATE.mode == MENU_WEATHER)
        {
            menu_sub_menu_weather_detect_inputs(btns, rtr, mtrx);
        }
    }
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

    if (g_tick % 50 == 0 && !g_in_submenu)
    {
        menu_set_underscore(g_underscore_on);
        g_underscore_on = !g_underscore_on;
        matrix_show(mtrx);
    }

    if (g_in_submenu && g_tick % DISPLAY_UPDATE_INTERVAL_TICKS_MS == 0)
    {
        menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    menu_detect_inputs(btns, rtr, mtrx);
}

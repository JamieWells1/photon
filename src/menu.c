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
static SubMode g_current_submenu = TKR_BTC;

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
static void menu_set_initial_display()
{
    // MenuState already initialised in global.c
    Menu currentMode = MENUS[MENU_STATE.main_mode];

    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_display_initialized = true;
}

static void menu_reset_states(Matrix* mtrx)
{
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

static void menu_slide_from_mode(Matrix* mtrx, Menu currentMode, Menu nextMode, int direction)
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

static void menu_slide_from_state(Matrix* mtrx, PixelState* current_state, PixelState* next_state,
                                  int direction)
{
    int sliding_coefficient = -(MATRIX_WIDTH / 2);
    for (int offset = 0; offset <= MATRIX_WIDTH; offset++)
    {
        matrix_clear(mtrx);

        // Render current state sliding out
        matrix_render_raw(current_state, direction * offset, 0);

        // Render next state sliding in
        matrix_render_raw(next_state, direction * offset - (direction * MATRIX_WIDTH), 0);

        matrix_show(mtrx);
        // Quadratic smoothing for sliding menu
        sleep_ms((sliding_coefficient * sliding_coefficient) / 10);
        sliding_coefficient++;
    }
}

// From clockwise turn
static void menu_main_slide_right(Matrix* mtrx)
{
    menu_set_underscore(true);

    Menu currentMode = MENUS[MENU_STATE.main_mode];
    // Last element -> wrap to first element
    int index_next = (MENU_STATE.main_mode + 1) % ARRAY_SIZE(MENUS);
    Menu nextMode = MENUS[index_next];

    menu_slide_from_mode(mtrx, currentMode, nextMode, -1);
    MENU_STATE.main_mode = index_next;
    menu_reset_states(mtrx);
}

// From anti-clockwise turn
static void menu_main_slide_left(Matrix* mtrx)
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

    menu_slide_from_mode(mtrx, currentMode, nextMode, 1);
    MENU_STATE.main_mode = index_previous;
    menu_reset_states(mtrx);
}

static void menu_enter_sub_menu(MenuState menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
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

        g_display_initialized = true;
    }

    if (menu_state.main_mode == MENU_TICKERS)
    {
        ticker_display(SUB_MENUS[menu_state.sub_mode], mtrx);
    }
    else if (menu_state.main_mode == MENU_GAMES)
    {
        game_display(SUB_MENUS[menu_state.sub_mode], mtrx);
    }
    else if (menu_state.main_mode == MENU_WEATHER)
    {
        weather_display(SUB_MENUS[menu_state.sub_mode].mode, mtrx, &g_weather_hour_offset);
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
        MENU_STATE.sub_mode = TEMP_HOURLY;

        // Save current display state
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset++;

        weather_display(MENU_STATE.sub_mode, mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        menu_slide_from_state(mtrx, current_state, next_state, -1);
    }

    if (input_rtr_anti_cw(rtr) && g_weather_hour_offset > 0)
    {
        MENU_STATE.sub_mode = TEMP_HOURLY;

        // Save current display state
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset--;
        if (g_weather_hour_offset < 0) g_weather_hour_offset = 0;

        weather_display(MENU_STATE.sub_mode, mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        menu_slide_from_state(mtrx, current_state, next_state, 1);
    }
}

static void menu_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_in_submenu && input_rtr_cw(rtr))
    {
        menu_main_slide_right(mtrx);
    }

    if (!g_in_submenu && input_rtr_anti_cw(rtr))
    {
        menu_main_slide_left(mtrx);
    }

    if (input_any_btn_pressed(btns, rtr) && !g_in_submenu)
    {
        menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    if (g_in_submenu)
    {
        if (MENU_STATE.main_mode == MENU_TICKERS)
        {
            menu_sub_menu_stocks_detect_inputs(btns, rtr, mtrx);
        }
        if (MENU_STATE.main_mode == MENU_GAMES)
        {
            menu_sub_menu_games_detect_inputs(btns, rtr, mtrx);
        }
        if (MENU_STATE.main_mode == MENU_WEATHER)
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

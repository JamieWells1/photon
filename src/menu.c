#include <menu.h>

#include <const.h>
#include <game.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <matrix.h>
#include <ticker.h>
#include <weather.h>

#include <displays.h>
#include <pico/time.h>
#include <stdlib.h>

static bool g_display_initialized = false;
static int g_tick = 0;
static bool g_underscore_on = true;

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
    MenuMode currentMode = MENU_MODES[MENU_STATE.main_mode];

    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_display_initialized = true;
}

static void reset_states(Matrix* mtrx)
{
    g_display_initialized = false;
    g_underscore_on = true;
    g_tick = 0;

    matrix_clear(mtrx);
    menu_set_initial_display();
    menu_set_underscore(true);
    matrix_show(mtrx);
}

static void slide_menu(Matrix* mtrx, MenuMode currentMode, MenuMode nextMode, int direction)
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

    MenuMode currentMode = MENU_MODES[MENU_STATE.main_mode];
    // Last element -> wrap to first element
    int index_next = (MENU_STATE.main_mode + 1) % ARRAY_SIZE(MENU_MODES);
    MenuMode nextMode = MENU_MODES[index_next];

    slide_menu(mtrx, currentMode, nextMode, -1);
    MENU_STATE.main_mode = index_next;
    reset_states(mtrx);
}

// From anti-clockwise turn
static void slide_main_menu_left(Matrix* mtrx)
{
    menu_set_underscore(true);

    MenuMode currentMode = MENU_MODES[MENU_STATE.main_mode];

    int index_previous;
    if (MENU_STATE.main_mode == 0)
    {
        index_previous = ARRAY_SIZE(MENU_MODES) - 1;
    }
    else
    {
        index_previous = (MENU_STATE.main_mode - 1);
    }
    MenuMode nextMode = MENU_MODES[index_previous];

    slide_menu(mtrx, currentMode, nextMode, 1);
    MENU_STATE.main_mode = index_previous;
    reset_states(mtrx);
}

static void menu_enter_sub_menu(MenuState menu_state)
{
    if (menu_state.main_mode == MENU_TICKERS)
    {
        menu_state.sub_mode = TKR_START;
        ticker_display(SUB_MENU_MODES[menu_state.sub_mode]);
    }

    else if (menu_state.main_mode == MENU_GAMES)
    {
        menu_state.sub_mode = GAME_START;
        game_display(SUB_MENU_MODES[menu_state.sub_mode]);
    }

    else if (menu_state.main_mode == MENU_WEATHER)
    {
        menu_state.sub_mode = TEMP_START;
        temp_display(SUB_MENU_MODES[menu_state.sub_mode]);
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

    if (g_tick % 60 == 0)
    {
        menu_set_underscore(g_underscore_on);
        g_underscore_on = !g_underscore_on;
        matrix_show(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        slide_main_menu_right(mtrx);
    }

    if (input_rtr_anti_cw(rtr))
    {
        slide_main_menu_left(mtrx);
    }

    if (input_any_btn_pressed(btns, rtr))
    {
        menu_enter_sub_menu(MENU_STATE);
        matrix_show(mtrx);
    }
}

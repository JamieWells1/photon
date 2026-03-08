#include <menu_main.h>

#include <config.h>
#include <const.h>
#include <flash_memory.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <menu_common.h>
#include <menu_markets.h>
#include <reset.h>
#include <stdio.h>
#include <weather.h>

#include <glyphs.h>
#include <hardware/watchdog.h>
#include <pico/time.h>
#include <stdlib.h>

#define TEMP_START 4
#define TEMP_END 2
#define CONFIG_START (TEMP_START + TEMP_END)
#define CONFIG_END 1
#define RESET_START (CONFIG_START + CONFIG_END)
#define RESET_END 1

static const Menu MENUS[] = {
    {MENU_MARKETS, "MARKET", &ICONS_ARR[STOCKS]},
    {MENU_WEATHER, "TEMP", &ICONS_ARR[SUNNY_CLOUDY]},
    {MENU_CONFIG, "CONFIG", &ICONS_ARR[DATA]},
    {MENU_RESET, "RESET", &ICONS_ARR[POWER]},
};

static bool g_display_initialized = false;
static int g_tick = 0;
static bool g_underscore_on = true;
static bool g_in_submenu = false;
static Mode g_current_mode = TEMP_START;

static int g_weather_hour_offset = 0;
static bool g_weather_submenu_ready = false;
static bool g_config_submenu_ready = false;

static void menu_main_set_underscore(bool underscore_on)
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

static void menu_main_set_initial_display(void)
{
    Menu currentMode = MENUS[MENU_STATE.mode];
    matrix_display_word_icon_pair(currentMode.text, &DEFAULT_COLOUR, currentMode.icon, 0);
    g_display_initialized = true;
}

void menu_main_reset_states(Matrix* mtrx)
{
    weather_cleanup();
    config_cleanup();
    menu_markets_cleanup();
    reset_cleanup();

    g_display_initialized = false;
    g_underscore_on = true;
    g_tick = 0;
    g_in_submenu = false;
    g_weather_hour_offset = 0;
    g_weather_submenu_ready = false;
    g_config_submenu_ready = false;

    matrix_clear(mtrx);
    menu_main_set_initial_display();
    menu_main_set_underscore(true);
    matrix_show(mtrx);
}

static void menu_main_slide_right(Matrix* mtrx)
{
    menu_main_set_underscore(true);

    int index_next = (MENU_STATE.mode + 1) % ARRAY_SIZE(MENUS);
    Menu nextMode = MENUS[index_next];

    debug(INFO, "Menu: %s -> %s", MENUS[MENU_STATE.mode].text, nextMode.text);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_right(mtrx, current_state, next_state);

    MENU_STATE.mode = index_next;
    menu_main_reset_states(mtrx);
}

static void menu_main_slide_left(Matrix* mtrx)
{
    menu_main_set_underscore(true);

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

    debug(INFO, "Menu: %s -> %s", MENUS[MENU_STATE.mode].text, nextMode.text);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_left(mtrx, current_state, next_state);

    MENU_STATE.mode = index_previous;
    menu_main_reset_states(mtrx);
}

static void menu_main_enter_sub_menu(Menu menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (menu_state.mode == MENU_MARKETS)
    {
        g_in_submenu = true;
        menu_markets_start(btns, rtr, mtrx);
    }
    else if (menu_state.mode == MENU_WEATHER)
    {
        Mode target_menu = TEMP_START;
        if (!g_in_submenu || g_current_mode != target_menu)
        {
            debug(TRACE, "Entering weather mode");
            g_in_submenu = true;
            g_current_mode = target_menu;
            g_display_initialized = false;
            g_display_initialized = true;
        }
        weather_display(mtrx, &g_weather_hour_offset);
    }
    else if (menu_state.mode == MENU_CONFIG)
    {
        Mode target_menu = CONFIG_START;
        if (!g_in_submenu || g_current_mode != target_menu)
        {
            debug(TRACE, "Entering config mode");
            g_in_submenu = true;
            g_current_mode = target_menu;
            g_display_initialized = false;
            g_display_initialized = true;
        }
        config_display(mtrx);
    }
    else if (menu_state.mode == MENU_RESET)
    {
        Mode target_menu = RESET_START;
        if (!g_in_submenu || g_current_mode != target_menu)
        {
            debug(TRACE, "Entering reset mode");
            g_in_submenu = true;
            g_current_mode = target_menu;
            g_display_initialized = false;
            g_display_initialized = true;
        }
        reset_display(mtrx);
    }
}

static void menu_main_weather_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_weather_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_weather_submenu_ready = true;
            debug(DEBUG, "All buttons released, weather submenu ready for input");
        }
        else
        {
            debug(TRACE, "Waiting for button release: btn0=%d btn1=%d rtr=%d",
                  btns[0].current_state, btns[1].current_state, rtr->current_sw_state);
        }
        return;
    }

    if (input_rtr_long_pressed(rtr))
    {
        menu_main_reset_states(mtrx);
    }

    if (input_rtr_pressed(rtr) && g_weather_hour_offset != 0)
    {
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset = 0;

        weather_display(mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        matrix_slide_left(mtrx, current_state, next_state);
    }

    int max_offset = weather_hours_fetched() - weather_current_hour_index() - 1;
    if (input_rtr_cw(rtr) && g_weather_hour_offset < max_offset)
    {
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

static void menu_main_config_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_config_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_config_submenu_ready = true;
            debug(DEBUG, "All buttons released, config submenu ready for input");
        }
        else
        {
            debug(TRACE, "Waiting for button release: btn0=%d btn1=%d rtr=%d",
                  btns[0].current_state, btns[1].current_state, rtr->current_sw_state);
        }
        return;
    }

    if (input_rtr_long_pressed(rtr))
    {
        menu_main_reset_states(mtrx);
    }
}

static void menu_main_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_in_submenu)
    {
        bool cw = input_rtr_cw(rtr);
        bool acw = input_rtr_anti_cw(rtr);

        if (cw || acw)
        {
            debug(TRACE, "menu_main_detect_inputs: g_in_submenu=%d, cw=%d, acw=%d\n", g_in_submenu,
                  cw, acw);
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
        debug(DEBUG, "Button pressed on main menu, entering submenu for mode=%d", MENU_STATE.mode);
        menu_main_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    if (g_in_submenu)
    {
        if (MENU_STATE.mode == MENU_MARKETS)
        {
            menu_markets_detect_inputs(btns, rtr, mtrx);
        }
        else if (g_current_mode == TEMP_START)
        {
            menu_main_weather_detect_inputs(btns, rtr, mtrx);
        }
        else if (g_current_mode == CONFIG_START)
        {
            menu_main_config_detect_inputs(btns, rtr, mtrx);
        }
        else if (g_current_mode == RESET_START)
        {
            bool was_ready = reset_is_ready();
            reset_detect_inputs(btns, rtr, mtrx);

            if (was_ready && input_any_btn_pressed(btns, rtr))
            {
                menu_main_reset_states(mtrx);
            }
        }
    }
}

void menu_main_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // Reset to avoid integer overflow for long running idle programs
    if (g_tick >= 100000) g_tick = 0;
    g_tick++;

    if (!g_display_initialized)
    {
        menu_main_set_initial_display();
        matrix_show(mtrx);
    }

    if (g_tick % 50 == 0 && !g_in_submenu)
    {
        menu_main_set_underscore(g_underscore_on);
        g_underscore_on = !g_underscore_on;
        matrix_show(mtrx);
    }

    if (g_in_submenu && g_tick % DISPLAY_UPDATE_INTERVAL_TICKS_MS == 0 &&
        g_current_mode != RESET_START && MENU_STATE.mode != MENU_MARKETS)
    {
        menu_main_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    menu_main_detect_inputs(btns, rtr, mtrx);
}

bool menu_main_in_submenu(void) { return g_in_submenu; }

void menu_main_exit_submenu(Matrix* mtrx) { menu_main_reset_states(mtrx); }

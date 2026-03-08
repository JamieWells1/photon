#include <menu.h>

#include <config.h>
#include <const.h>
#include <crypto.h>
#include <flash_memory.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>
#include <stdio.h>
#include <stock.h>
#include <weather.h>

#include <glyphs.h>
#include <hardware/watchdog.h>
#include <pico/time.h>
#include <stdlib.h>

static bool g_display_initialized = false;
static int g_tick = 0;
static bool g_underscore_on = true;
static bool g_in_submenu = false;
static Mode g_current_mode = MENU_CRYPTO;

static int g_weather_hour_offset = 0;
static int g_crypto_offset = 0;
static int g_stock_offset = 0;
static int g_reset_selection = 0;
static bool g_reset_submenu_ready = false;

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
    stock_cleanup();
    crypto_cleanup();
    weather_cleanup();
    config_cleanup();

    g_display_initialized = false;
    g_underscore_on = true;
    g_tick = 0;
    g_in_submenu = false;
    g_weather_hour_offset = 0;
    g_crypto_offset = 0;
    g_stock_offset = 0;
    g_reset_selection = 0;
    g_reset_submenu_ready = false;

    matrix_clear(mtrx);
    menu_set_initial_display();
    menu_set_underscore(true);
    matrix_show(mtrx);
}

static void menu_reset_display(Matrix* mtrx)
{
    matrix_clear(mtrx);

    int yes_x = 2;
    int no_x = 18;
    int text_y = 1;
    int uscr_starting_y = 7;
    int uscr_length_yes = matrix_calculate_word_width_with_space("YES") - 1;
    int uscr_length_no = matrix_calculate_word_width_with_space("NO") - 1;

    matrix_display_word("YES", yes_x, text_y, &GREEN);
    matrix_display_word("NO", no_x, text_y, &RED);

    if (g_reset_selection == 0)
    {
        matrix_draw_horiz_line(yes_x, uscr_starting_y, uscr_length_yes, &GREEN);
    }
    else
    {
        matrix_draw_horiz_line(no_x, uscr_starting_y, uscr_length_no, &RED);
    }

    matrix_show(mtrx);
}

static void menu_main_slide_right(Matrix* mtrx)
{
    menu_set_underscore(true);

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
    menu_reset_states(mtrx);
}

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

    debug(INFO, "Menu: %s -> %s", MENUS[MENU_STATE.mode].text, nextMode.text);

    PixelState current_state[NUM_PIXELS];
    matrix_save_state(current_state);

    matrix_clear(mtrx);
    matrix_display_word_icon_pair(nextMode.text, &DEFAULT_COLOUR, nextMode.icon, 0);
    PixelState next_state[NUM_PIXELS];
    matrix_save_state(next_state);

    matrix_slide_left(mtrx, current_state, next_state);

    MENU_STATE.mode = index_previous;
    menu_reset_states(mtrx);
}

static void menu_enter_sub_menu(Menu menu_state, Button* btns, Rotator* rtr, Matrix* mtrx)
{
    Mode target_menu;

    if (menu_state.mode == MENU_STOCKS)
    {
        target_menu = STK_START;
    }
    else if (menu_state.mode == MENU_CRYPTO)
    {
        target_menu = CRYPTO_START;
    }
    else if (menu_state.mode == MENU_WEATHER)
    {
        target_menu = TEMP_START;
    }
    else if (menu_state.mode == MENU_CONFIG)
    {
        target_menu = CONFIG_START;
    }
    else if (menu_state.mode == MENU_RESET)
    {
        target_menu = RESET_START;
    }
    else
    {
        return;
    }

    // Only initialise if entering a new submenu or first time
    if (!g_in_submenu || g_current_mode != target_menu)
    {
        debug(TRACE, "Entering submenu: target_menu=%d", target_menu);
        g_in_submenu = true;
        g_current_mode = target_menu;
        g_display_initialized = false;

        if (target_menu == RESET_START)
        {
            g_reset_submenu_ready = false;
            debug(DEBUG, "Reset submenu entered, waiting for button release");
        }

        g_display_initialized = true;
    }

    if (g_current_mode == STK_START)
    {
        stock_display(mtrx, &g_stock_offset);
    }
    else if (g_current_mode == CRYPTO_START)
    {
        crypto_display(mtrx, &g_crypto_offset);
    }
    else if (g_current_mode == TEMP_START)
    {
        weather_display(mtrx, &g_weather_hour_offset);
    }
    else if (g_current_mode == CONFIG_START)
    {
        config_display(mtrx);
    }
    else if (g_current_mode == RESET_START)
    {
        debug(TRACE, "Displaying reset menu, ready=%d", g_reset_submenu_ready);
        menu_reset_display(mtrx);
    }
}

static void menu_sub_menu_stocks_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_rtr_long_pressed(rtr))
    {
        menu_reset_states(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        g_stock_offset += 1;
    }
}

static void menu_sub_menu_crypto_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_rtr_long_pressed(rtr))
    {
        menu_reset_states(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        g_crypto_offset += 1;
    }
}

static void menu_sub_menu_weather_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_rtr_long_pressed(rtr))
    {
        menu_reset_states(mtrx);
    }

    // Short press resets to current hour
    if (input_rtr_pressed(rtr) && g_weather_hour_offset != 0)
    {
        // Save current display state
        PixelState current_state[NUM_PIXELS];
        matrix_save_state(current_state);

        g_weather_hour_offset = 0;

        weather_display(mtrx, &g_weather_hour_offset);
        PixelState next_state[NUM_PIXELS];
        matrix_save_state(next_state);

        // Slide left to indicate going back
        matrix_slide_left(mtrx, current_state, next_state);
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

static void menu_sub_menu_config_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (input_rtr_long_pressed(rtr))
    {
        menu_reset_states(mtrx);
    }
}

static void menu_sub_menu_reset_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    if (!g_reset_submenu_ready)
    {
        bool all_released = (btns[0].current_state == true && btns[1].current_state == true &&
                             rtr->current_sw_state == true);

        if (all_released)
        {
            g_reset_submenu_ready = true;
            debug(DEBUG, "All buttons released, reset submenu ready for input");
        }
        else
        {
            debug(TRACE, "Waiting for button release: btn0=%d btn1=%d rtr=%d",
                  btns[0].current_state, btns[1].current_state, rtr->current_sw_state);
        }
        return;
    }

    if (input_rtr_cw(rtr))
    {
        debug(DEBUG, "Reset menu: encoder CW, selection %d -> %d", g_reset_selection,
              (g_reset_selection + 1) % 2);
        g_reset_selection = (g_reset_selection + 1) % 2;
        menu_reset_display(mtrx);
    }

    if (input_rtr_anti_cw(rtr))
    {
        debug(DEBUG, "Reset menu: encoder CCW, selection %d -> %d", g_reset_selection,
              (g_reset_selection - 1 + 2) % 2);
        g_reset_selection = (g_reset_selection - 1 + 2) % 2;
        menu_reset_display(mtrx);
    }

    if (input_any_btn_pressed(btns, rtr))
    {
        debug(INFO, "Reset menu: button pressed, selection=%d (%s)", g_reset_selection,
              g_reset_selection == 0 ? "YES" : "NO");

        if (g_reset_selection == 0)
        {
            debug(INFO, "Executing flash reset");
            flash_memory_erase();

            matrix_clear(mtrx);
            matrix_display_word_icon_pair("DONE", &GREEN, NULL, 0);
            matrix_show(mtrx);
            sleep_ms(1000);

            debug(INFO, "Rebooting device");
            watchdog_enable(1, 1);
            while (1);
        }
        else
        {
            debug(INFO, "Reset cancelled by user");
        }

        menu_reset_states(mtrx);
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
        debug(DEBUG, "Button pressed on main menu, entering submenu for mode=%d", MENU_STATE.mode);
        menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    if (g_in_submenu)
    {
        if (g_current_mode == STK_START)
        {
            menu_sub_menu_stocks_detect_inputs(btns, rtr, mtrx);
        }
        if (g_current_mode == CRYPTO_START)
        {
            menu_sub_menu_crypto_detect_inputs(btns, rtr, mtrx);
        }
        if (g_current_mode == TEMP_START)
        {
            menu_sub_menu_weather_detect_inputs(btns, rtr, mtrx);
        }
        if (g_current_mode == CONFIG_START)
        {
            menu_sub_menu_config_detect_inputs(btns, rtr, mtrx);
        }
        if (g_current_mode == RESET_START)
        {
            menu_sub_menu_reset_detect_inputs(btns, rtr, mtrx);
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

    if (g_in_submenu && g_tick % DISPLAY_UPDATE_INTERVAL_TICKS_MS == 0 &&
        g_current_mode != RESET_START)
    {
        menu_enter_sub_menu(MENU_STATE, btns, rtr, mtrx);
    }

    menu_detect_inputs(btns, rtr, mtrx);
}

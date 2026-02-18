#include <menu.h>

#include <const.h>
#include <global.h>
#include <graphics.h>
#include <input.h>
#include <matrix.h>

#include <pico/time.h>
#include <stdlib.h>
#include "displays.h"

static bool display_initialized = false;
static int tick = 0;
static bool underscore_on = true;

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

    matrix_display_word_icon_pair(currentMode.name, &DEFAULT_COLOUR, currentMode.icon, 0);
    display_initialized = true;
}

static void reset_states(Matrix* mtrx)
{
    display_initialized = false;
    underscore_on = true;
    tick = 0;

    matrix_clear(mtrx);
    menu_set_initial_display();
    menu_set_underscore(true);
    matrix_show(mtrx);
}

// From clockwise turn
static void slide_menu_right(Matrix* mtrx)
{
    menu_set_underscore(true);

    MenuMode currentMode = MENU_MODES[MENU_STATE.main_mode];
    // Last element -> wrap to first element
    int index_next = (MENU_STATE.main_mode + 1) % ARRAY_SIZE(MENU_MODES);
    MenuMode nextMode = MENU_MODES[index_next];

    for (int offset = 0; offset <= MATRIX_WIDTH; offset++)
    {
        matrix_clear(mtrx);

        // Move current out
        matrix_display_word_icon_pair(currentMode.name, &DEFAULT_COLOUR, currentMode.icon, -offset);
        // Move next in
        matrix_display_word_icon_pair(nextMode.name, &DEFAULT_COLOUR, nextMode.icon,
                                      MATRIX_WIDTH - offset);

        matrix_show(mtrx);
        sleep_ms(20);
    }

    MENU_STATE.main_mode = index_next;
    reset_states(mtrx);
}

// From anti-clockwise turn
static void slide_menu_left(Matrix* mtrx)
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

    // Quadratic smoothing for sliding menu
    int sliding_coefficient = -(MATRIX_WIDTH / 2);

    for (int offset = 0; offset <= MATRIX_WIDTH; offset++)
    {
        matrix_clear(mtrx);

        // Move current out
        matrix_display_word_icon_pair(currentMode.name, &DEFAULT_COLOUR, currentMode.icon, offset);
        // Move next in
        matrix_display_word_icon_pair(nextMode.name, &DEFAULT_COLOUR, nextMode.icon,
                                      offset - MATRIX_WIDTH);

        matrix_show(mtrx);
        sleep_ms((sliding_coefficient * sliding_coefficient) / 10);
        sliding_coefficient++;
    }

    MENU_STATE.main_mode = index_previous;
    reset_states(mtrx);
}

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    // Reset to avoid integer overflow for long running idle programs
    if (tick >= 100000) tick = 0;
    tick++;

    if (!display_initialized)
    {
        menu_set_initial_display();
        matrix_show(mtrx);
    }

    if (tick % 60 == 0)
    {
        menu_set_underscore(underscore_on);
        underscore_on = !underscore_on;
        matrix_show(mtrx);
    }

    if (input_rtr_cw(rtr))
    {
        slide_menu_right(mtrx);
    }

    if (input_rtr_anti_cw(rtr))
    {
        slide_menu_left(mtrx);
    }
}

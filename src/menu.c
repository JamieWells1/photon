#include <menu.h>

#include <const.h>
#include <global.h>
#include <graphics.h>
#include <input.h>

#include <pico/time.h>
#include <stdlib.h>
#include "matrix.h"

static bool display_initialized = false;
static int tick = 0;
static bool underscore_on = false;

static void menu_set_underscore()
{
    int uscr_starting_x = 1;
    int uscr_starting_y = 7;
    int uscr_length = 30;

    if (underscore_on)
    {
        matrix_clear_horiz_line(uscr_starting_x, uscr_starting_y, uscr_length);
        underscore_on = false;
    }
    else
    {
        matrix_draw_horiz_line(uscr_starting_x, uscr_starting_y, uscr_length, &DEFAULT_COLOUR);
        underscore_on = true;
    }
}

static void menu_set_initial_display()
{
    // MenuState already initialised in global.c
    MenuMode currentMode = MENU_MODES[MENU_STATE.main_mode];

    matrix_display_word_icon_pair(currentMode.name, &DEFAULT_COLOUR, currentMode.icon);
    display_initialized = true;
}

void menu_start(Button* btns, Rotator* rtr, Matrix* mtrx)
{
    tick++;
    // Only initialize display once
    if (!display_initialized)
    {
        menu_set_initial_display();
        matrix_show(mtrx);
    }

    if (tick % 60 == 0)
    {
        menu_set_underscore();
        matrix_show(mtrx);
    }

    // TODO: Handle button/rotator input to change modes
}

#include <reset.h>

#include <const.h>
#include <flash_memory.h>
#include <input.h>
#include <macro.h>
#include <matrix.h>

#include <stdio.h>

#include <hardware/watchdog.h>
#include <pico/time.h>

static int g_reset_selection = 0;
static bool g_reset_submenu_ready = false;

void reset_display(Matrix* mtrx)
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

void reset_detect_inputs(Button* btns, Rotator* rtr, Matrix* mtrx)
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
        reset_display(mtrx);
    }

    if (input_rtr_anti_cw(rtr))
    {
        debug(DEBUG, "Reset menu: encoder CCW, selection %d -> %d", g_reset_selection,
              (g_reset_selection - 1 + 2) % 2);
        g_reset_selection = (g_reset_selection - 1 + 2) % 2;
        reset_display(mtrx);
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

        // Note: If reset is cancelled, the caller (menu_main.c) should handle exiting the submenu
    }
}

void reset_cleanup(void)
{
    g_reset_selection = 0;
    g_reset_submenu_ready = false;
}

bool reset_is_ready(void) { return g_reset_submenu_ready; }

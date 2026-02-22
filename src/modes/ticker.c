#include <ticker.h>

#include <glyphs.h>
#include <matrix.h>
#include <menu.h>

#include <string.h>

static char g_last_ticker_text[32] = "";

void ticker_display(SubMenu sub_mode, Matrix* mtrx)
{
    // TODO: Implement ticker display logic
    // Only update display if submenu changed
    if (strcmp(sub_mode.text, g_last_ticker_text) != 0)
    {
        matrix_clear(mtrx);
        matrix_display_word_icon_pair(sub_mode.text, &DEFAULT_COLOUR, sub_mode.icon, 0);
        matrix_show(mtrx);

        strncpy(g_last_ticker_text, sub_mode.text, sizeof(g_last_ticker_text) - 1);
        g_last_ticker_text[sizeof(g_last_ticker_text) - 1] = '\0';
    }
}

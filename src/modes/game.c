#include <game.h>
#include <glyphs.h>
#include <menu.h>

#include <string.h>

static char g_last_game_text[32] = "";

void game_display(SubMenu sub_mode, Matrix* mtrx)
{
    // TODO
    // Only update display if submenu changed
    if (strcmp(sub_mode.text, g_last_game_text) != 0)
    {
        matrix_clear(mtrx);
        matrix_display_word_icon_pair(sub_mode.text, &DEFAULT_COLOUR, sub_mode.icon, 0);
        matrix_show(mtrx);

        strncpy(g_last_game_text, sub_mode.text, sizeof(g_last_game_text) - 1);
        g_last_game_text[sizeof(g_last_game_text) - 1] = '\0';
    }
}

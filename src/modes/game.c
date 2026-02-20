#include <displays.h>
#include <game.h>
#include <menu.h>

void game_display(SubMenuMode sub_mode)
{
    matrix_display_word_icon_pair(sub_mode.text, &DEFAULT_COLOUR, sub_mode.icon, 0);
}

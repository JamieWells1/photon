#include <ticker.h>

#include <displays.h>
#include <menu.h>

void ticker_display(SubMenuMode sub_mode)
{
    matrix_display_word(sub_mode.text, 1, 1, &DEFAULT_COLOUR);
}

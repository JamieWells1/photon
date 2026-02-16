#include <const.h>
#include <print.h>

#include <stdio.h>

void debug(const char* str)
{
    if (DEBUG_LEVEL == TRACE)
    {
        printf("[DEBUG]: %s\n", str);
    }
}

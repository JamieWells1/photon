#include <const.h>
#include <print.h>

#include <stdio.h>

void debug(const char* str)
{
    if (DEBUG_LOGS)
    {
        printf("[DEBUG]: %s\n", str);
    }
}

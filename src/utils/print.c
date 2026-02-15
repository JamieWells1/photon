#include <const.h>
#include <print.h>

#include <stdio.h>

void debug(const char* str)
{
    if (DEBUG_LOGS)
    {
        printf("\n[DEBUG]: %s", str);
    }
}

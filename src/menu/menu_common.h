/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef MENU_H
#define MENU_H

#include <glyphs.h>

typedef enum
{
    MENU_MARKETS = 0,
    MENU_WEATHER = 1,
    MENU_CONFIG = 2,
    MENU_RESET = 3,
} Mode;

typedef struct
{
    Mode mode;
    const char* text;
    const Glyph* icon;
} Menu;

#endif  // MENU_H

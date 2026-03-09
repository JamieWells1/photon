/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

// For global state

#include <menu_common.h>

typedef enum
{
    HORIZONTAL,
    VERTICAL,
} MatrixOrientation;

extern Menu MENU_STATE;

#endif  // GLOBAL_H

/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef UPDATE_H
#define UPDATE_H

#include <error.h>
#include <matrix.h>
#include <stdbool.h>

ReturnCode update_check_version(Matrix* mtrx);

bool update_is_available(void);

const char* update_get_remote_version(void);

void update_enter_bootloader_mode(void);

#endif  // UPDATE_H

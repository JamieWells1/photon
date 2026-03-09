/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include <pico/time.h>

uint64_t get_current_time_ms(void);

float parse_price_from_json_array(const char* json, const char* key);
float parse_price_from_json_string(const char* json, const char* key);

#endif  // UTILS_H

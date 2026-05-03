/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef PARSER_H
#define PARSER_H

#include <error.h>
#include <flash_memory.h>
#include <stddef.h>
#include <stdint.h>

void parser_decode_url(char* dst, const char* src, size_t max_len);
void parser_post_data(const char* body, device_config_t* config);
ReturnCode parser_csv_portfolio(const char* csv_data, device_config_t* config);

#endif  // PARSER_H

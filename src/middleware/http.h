/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef HTTP_H
#define HTTP_H

#include <error.h>
#include <matrix.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*http_response_callback_t)(const char* body, size_t len, bool complete);

ReturnCode http_get(const char* host_url, uint16_t port, const char* url_sub_path,
                    http_response_callback_t callback, Matrix* mtrx, bool update_display);

#endif  // HTTP_H

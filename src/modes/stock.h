/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef STOCK_H
#define STOCK_H

#include <matrix.h>
#include <stdint.h>

// ============
// Stock Index
// ============

#define MAX_STOCK_TICKERS 50

typedef struct
{
    char ticker[10];
    float current_price;
    float open_price;
} Stock;

uint8_t stock_get_count(void);
const char* stock_get_ticker(int idx);

void stock_set_enabled_mask(uint64_t mask);

void stock_display(Matrix* mtrx, int* offset);
void stock_cleanup(void);

#endif  // STOCK_H

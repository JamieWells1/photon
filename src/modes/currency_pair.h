/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef CURRENCY_PAIR_H
#define CURRENCY_PAIR_H

#include <matrix.h>
#include <stdbool.h>
#include <stdint.h>

#define CURRENCY_PAIR_MAX 10
#define CURRENCY_PAIR_LEN 6  // e.g., "GBPUSD"

// Default currency pairs: GBPUSD, GBPEUR, USDEUR
#define DEFAULT_CURRENCY_PAIR_COUNT 3

typedef struct
{
    char pair[CURRENCY_PAIR_LEN + 1];  // +1 for null terminator
    float current_price;
    float open_price;
    bool enabled;
} CurrencyPairData;

extern CurrencyPairData g_currency_pairs[CURRENCY_PAIR_MAX];

void currency_pair_display(Matrix* mtrx, int* offset);
void currency_pair_cleanup(void);

uint8_t currency_pair_get_count(void);
void currency_pair_set_pairs(const char pairs[][CURRENCY_PAIR_LEN + 1], uint8_t count);
void currency_pair_get_pairs(char pairs[][CURRENCY_PAIR_LEN + 1], uint8_t* count);

#endif  // CURRENCY_PAIR_H

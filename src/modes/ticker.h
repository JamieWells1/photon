#ifndef TICKER_H
#define TICKER_H

#include <matrix.h>

// ============
// Ticker Index
// ============

#define TKR_XBT "XBT"
#define TKR_XAU "XAU"
#define TKR_XAG "XAG"

typedef enum
{
    XBT = 0,
    XAU = 1,
    XAG = 2,
    TKR_COUNT
} TkrIndex;

// ==========
// Currencies
// ==========

#define CCY_USD "USD"
#define CCY_GBP "GBP"
#define CCY_EUR "EUR"
#define CCY_JPY "JPY"

typedef enum
{
    USD = 0,
    GBP = 1,
    EUR = 2,
    JPY = 3,
    CCY_COUNT
} Currency;

// =====
// Price
// =====

typedef struct
{
    const TkrIndex tkr_idx;
    const char* to_str;
    float current_price;
    float open_price;
} Ticker;

static Ticker TKR_ARR[TKR_COUNT] = {
    {XBT, TKR_XBT, 0.0f, 0.0f}, {XAU, TKR_XAU, 0.0f, 0.0f}, {XAG, TKR_XAG, 0.0f, 0.0f}};

void ticker_display(Matrix* mtrx);
void ticker_cleanup(void);

#endif  // TICKER_H

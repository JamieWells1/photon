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
    TkrIndex tkr;
    char* tkr_str;
    int* price;
} Ticker;

static const Ticker TKR_ARR[TKR_COUNT] = {
    {XBT, TKR_XBT, NULL}, {XAU, TKR_XAU, NULL}, {XAG, TKR_XAG, NULL}};

void ticker_display(Matrix* mtrx);
void ticker_cleanup(void);

#endif  // TICKER_H

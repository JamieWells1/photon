#ifndef TICKER_H
#define TICKER_H

#include <matrix.h>

// ============
// Ticker Index
// ============

#define TKR_XBT "XBT"
#define TKR_ETH "ETH"
#define TKR_SOL "SOL"

typedef enum
{
    XBT = 0,
    ETH = 1,
    SOL = 2,
    TKR_COUNT
} TkrIndex;

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

extern Ticker TKR_ARR[TKR_COUNT];

void ticker_display(Matrix* mtrx, int* offset);
void ticker_cleanup(void);

#endif  // TICKER_H

#ifndef TICKER_H
#define TICKER_H

#include <matrix.h>

// ============
// Ticker Index
// ============

#define TKR_XBT "XBT"
#define TKR_ETH "ETH"
#define TKR_SOL "SOL"
#define TKR_ADA "ADA"
#define TKR_DOT "DOT"
#define TKR_LINK "LINK"
#define TKR_AVAX "AVAX"
#define TKR_DOGE "DOGE"
#define TKR_XRP "XRP"
#define TKR_LTC "LTC"

typedef enum
{
    XBT = 0,
    ETH = 1,
    SOL = 2,
    ADA = 3,
    DOT = 4,
    LINK = 5,
    AVAX = 6,
    DOGE = 7,
    XRP = 8,
    LTC = 9,
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

bool ticker_is_enabled(TkrIndex idx);
void ticker_set_enabled_mask(uint16_t mask);

void ticker_display(Matrix* mtrx, int* offset);
void ticker_cleanup(void);

#endif  // TICKER_H

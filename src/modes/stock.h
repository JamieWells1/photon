#ifndef STOCK_H
#define STOCK_H

#include <matrix.h>

// ============
// Stock Index
// ============

#define STK_AAPL "AAPL"
#define STK_MSFT "MSFT"
#define STK_GOOGL "GOOGL"
#define STK_AMZN "AMZN"
#define STK_TSLA "TSLA"
#define STK_NVDA "NVDA"
#define STK_META "META"
#define STK_JPM "JPM"
#define STK_V "V"
#define STK_DIS "DIS"

typedef enum
{
    AAPL = 0,
    MSFT = 1,
    GOOGL = 2,
    AMZN = 3,
    TSLA = 4,
    NVDA = 5,
    META = 6,
    JPM = 7,
    V = 8,
    DIS = 9,
    STK_COUNT
} StockIndex;

// =====
// Price
// =====

typedef struct
{
    const StockIndex stk_idx;
    const char* to_str;
    float current_price;
    float open_price;
} Stock;

extern Stock STK_ARR[STK_COUNT];

bool stock_is_enabled(StockIndex idx);
void stock_set_enabled_mask(uint16_t mask);

void stock_display(Matrix* mtrx, int* offset);
void stock_cleanup(void);

#endif  // STOCK_H

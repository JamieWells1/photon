#ifndef CRYPTO_H
#define CRYPTO_H

#include <matrix.h>

// ============
// Crypto Index
// ============

#define CRYPTO_XBT "XBT"
#define CRYPTO_ETH "ETH"
#define CRYPTO_SOL "SOL"
#define CRYPTO_ADA "ADA"
#define CRYPTO_DOT "DOT"
#define CRYPTO_LINK "LINK"
#define CRYPTO_AVAX "AVAX"
#define CRYPTO_DOGE "DOGE"
#define CRYPTO_XRP "XRP"
#define CRYPTO_LTC "LTC"

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
    CRYPTO_COUNT
} CryptoIndex;

// =====
// Price
// =====

typedef struct
{
    const CryptoIndex crypto_idx;
    const char* to_str;
    float current_price;
    float open_price;
} Crypto;

extern Crypto CRYPTO_ARR[CRYPTO_COUNT];

bool crypto_is_enabled(CryptoIndex idx);
void crypto_set_enabled_mask(uint16_t mask);

void crypto_display(Matrix* mtrx, int* offset);
void crypto_cleanup(void);

#endif  // CRYPTO_H

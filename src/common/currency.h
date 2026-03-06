#ifndef CURRENCY_H
#define CURRENCY_H

#define CCY_GBP "GBP"
#define CCY_USD "USD"
#define CCY_EUR "EUR"
#define CCY_JPY "JPY"

typedef enum
{
    GBP = 0,
    USD = 1,
    EUR = 2,
    JPY = 3,
    CCY_COUNT
} Currency;

#endif // CURRENCY_H

/*
 * Copyright (c) 2026 Jamie Wells. All Rights Reserved.
 * Proprietary and Confidential - Unauthorized copying or distribution prohibited.
 */

#ifndef CURRENCY_H
#define CURRENCY_H

#define CCY_GBP "GBP"
#define CCY_USD "USD"
#define CCY_EUR "EUR"
#define CCY_JPY "JPY"
#define CCY_CAD "CAD"
#define CCY_AUD "AUD"
#define CCY_CHF "CHF"
#define CCY_CNY "CNY"
#define CCY_INR "INR"
#define CCY_KRW "KRW"
#define CCY_BRL "BRL"
#define CCY_SGD "SGD"

typedef enum
{
    GBP = 0,
    USD = 1,
    EUR = 2,
    JPY = 3,
    CAD = 4,
    AUD = 5,
    CHF = 6,
    CNY = 7,
    INR = 8,
    KRW = 9,
    BRL = 10,
    SGD = 11,
    CCY_COUNT
} Currency;

const char* currency_get_name(Currency ccy);
const char* currency_get_symbol(Currency ccy);

#endif  // CURRENCY_H

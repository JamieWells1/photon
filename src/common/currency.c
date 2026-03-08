#include <currency.h>

const char* currency_get_name(Currency ccy)
{
    switch (ccy)
    {
        case GBP:
            return CCY_GBP;
        case USD:
            return CCY_USD;
        case EUR:
            return CCY_EUR;
        case JPY:
            return CCY_JPY;
        case CAD:
            return CCY_CAD;
        case AUD:
            return CCY_AUD;
        case CHF:
            return CCY_CHF;
        case CNY:
            return CCY_CNY;
        case INR:
            return CCY_INR;
        case KRW:
            return CCY_KRW;
        case BRL:
            return CCY_BRL;
        case SGD:
            return CCY_SGD;
        default:
            return CCY_GBP;
    }
}

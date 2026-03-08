#include "ticker_translate.h"

#include <stddef.h>
#include <string.h>

// LSE/European tickers that trade on London Stock Exchange
// These tickers need .L suffix when querying Yahoo Finance
static const char* LSE_TICKERS[] = {
    // Vanguard S&P 500 UCITS
    "VUAG", "VUSA",

    // Vanguard Global/Regional
    "VWRL", "VWRP", "VNRG", "VEUA", "VFEG", "VJPB", "VDPG",

    // Vanguard Bond & Dividend
    "VAGS", "VHYL", "VEVE",

    // iShares Core (very popular in Europe)
    "IWDA", "SWDA", "CSPX", "EQQQ", "EMIM",

    // Sector ETFs
    "IITU",

    // UK/FTSE ETFs
    "VUKE", "ISF", "IUKD",
};

bool ticker_is_lse(const char* ticker)
{
    if (!ticker) return false;

    for (size_t i = 0; i < sizeof(LSE_TICKERS) / sizeof(LSE_TICKERS[0]); i++)
    {
        if (strcmp(ticker, LSE_TICKERS[i]) == 0)
        {
            return true;
        }
    }

    return false;
}

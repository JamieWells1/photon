#include "ticker_translate.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    const char* t212_ticker;
    const char* us_equivalent;
} TickerMapping;

// Common European/LSE ticker translations to US equivalents
static const TickerMapping TICKER_TRANSLATIONS[] = {
    // Vanguard S&P 500 variants
    {"VUAG", "SPY"},  // Vanguard S&P 500 UCITS ETF (Acc) -> SPDR S&P 500
    {"VUSA", "SPY"},  // Vanguard S&P 500 UCITS ETF (Dist) -> SPDR S&P 500

    // Vanguard Total Market variants
    {"VWRL", "VT"},   // Vanguard FTSE All-World -> Vanguard Total World Stock
    {"VWRP", "VT"},   // Vanguard FTSE All-World (Acc) -> Vanguard Total World Stock

    // Vanguard Regional ETFs
    {"VNRG", "VTI"},  // Vanguard FTSE North America (Acc) -> Vanguard Total Stock Market
    {"VEUA", "VGK"},  // Vanguard FTSE Developed Europe (Acc) -> Vanguard FTSE Europe
    {"VFEG", "VWO"},  // Vanguard FTSE Emerging Markets (Acc) -> Vanguard Emerging Markets
    {"VJPB", "EWJ"},  // Vanguard FTSE Japan (Acc) -> iShares MSCI Japan
    {"VDPG", "VPL"},  // Vanguard FTSE Developed Asia Pacific ex Japan -> Vanguard FTSE Pacific

    // Vanguard Bond ETFs
    {"VAGS", "BND"},  // Vanguard Global Aggregate Bond (Acc) -> Vanguard Total Bond Market

    // Vanguard Dividend ETFs
    {"VHYL", "VYM"},  // Vanguard FTSE All-World High Dividend Yield -> Vanguard High Dividend Yield

    // Vanguard Developed World
    {"VEVE", "VEA"},  // Vanguard FTSE Developed World -> Vanguard Developed Markets

    // iShares Core variants (very popular in Europe)
    {"IWDA", "ACWI"}, // iShares MSCI World -> iShares MSCI ACWI (most popular European ETF)
    {"SWDA", "ACWI"}, // SPDR MSCI World -> iShares MSCI ACWI
    {"CSPX", "SPY"},  // iShares Core S&P 500 -> SPDR S&P 500
    {"EQQQ", "QQQ"},  // iShares NASDAQ 100 -> Invesco QQQ
    {"EMIM", "IEMG"}, // iShares Core MSCI Emerging Markets -> iShares Core MSCI Emerging Markets

    // Sector ETFs
    {"IITU", "VGT"},  // iShares S&P 500 Info Tech -> Vanguard Information Technology

    // UK/FTSE ETFs (LSE to US equivalents)
    {"VUKE", "EWU"},  // Vanguard FTSE 100 -> iShares MSCI United Kingdom
    {"ISF", "EWU"},   // iShares Core FTSE 100 -> iShares MSCI United Kingdom
    {"IUKD", "EWU"},  // iShares UK Dividend -> iShares MSCI United Kingdom

    // Add more translations as needed
};

const char* ticker_translate(const char* ticker)
{
    if (!ticker) return NULL;

    for (size_t i = 0; i < sizeof(TICKER_TRANSLATIONS) / sizeof(TICKER_TRANSLATIONS[0]); i++)
    {
        if (strcmp(ticker, TICKER_TRANSLATIONS[i].t212_ticker) == 0)
        {
            return TICKER_TRANSLATIONS[i].us_equivalent;
        }
    }

    // No translation found, return original ticker
    return ticker;
}

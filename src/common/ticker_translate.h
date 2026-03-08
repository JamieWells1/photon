#ifndef TICKER_TRANSLATE_H
#define TICKER_TRANSLATE_H

// Translates T212/European tickers to Yahoo Finance compatible US equivalents
// Returns the translated ticker, or the original if no translation exists
const char* ticker_translate(const char* ticker);

#endif // TICKER_TRANSLATE_H

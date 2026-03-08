#include <const.h>

const LogLevel LOG_LEVEL = TRACE;

const char* const LOCATION_NAMES[NUM_LOCATIONS] = {
    "London",  "New York",  "Los Angeles", "Chicago", "Miami",          "San Francisco",
    "Seattle", "Toronto",   "Paris",       "Berlin",  "Stockholm",      "Athens",
    "Dubai",   "Singapore", "Tokyo",       "Sydney",  "Rio de Janeiro", "Cape Town"};

const char* const CURRENCY_NAMES[NUM_CURRENCIES] = {
    "GBP", "USD", "EUR", "JPY", "CAD", "AUD", "CHF", "CNY", "INR", "KRW", "BRL", "SGD"};

const char* const CURRENCY_SYMBOLS[NUM_CURRENCIES] = {
    "&#163;", "$", "&#8364;", "&#165;", "$", "$", "Fr", "&#165;", "&#8377;", "&#8361;", "R$", "$"};

const char* const STOCK_TICKER_NAMES[NUM_STOCK_TICKERS] = {
    "AAPL", "MSFT", "GOOGL", "AMZN", "TSLA", "NVDA", "META", "JPM", "V", "DIS"};

const char* const CRYPTO_TICKER_NAMES[NUM_CRYPTO_TICKERS] = {
    "XBT", "ETH", "SOL", "ADA", "DOT", "LINK", "AVAX", "DOGE", "XRP", "LTC"};

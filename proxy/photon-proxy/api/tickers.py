# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

from api.config import logger

# T212 ticker → yfinance base ticker (before exchange suffix is appended).
TICKER_ALIASES = {}

# LSE tickers (.L suffix)
LSE_TICKERS = {
    "VUAG", "VUSA", "VWRL", "VWRP", "VNRG", "VEUA", "VFEG", "VJPB",
    "VDPG", "VAGS", "VHYL", "VEVE", "IWDA", "SWDA", "CSPX", "EQQQ",
    "EMIM", "IITU", "VUKE", "ISF", "IUKD", "EQGB", "XUSE",
}

# Euronext Amsterdam tickers (.AS suffix)
EURONEXT_TICKERS = {
    "VWCE",  # Vanguard FTSE All-World (Acc)
}

# Module-level currency cache keyed by yf_symbol (e.g. "EQQQ.L").
# A stock's currency never changes, so this is populated once per process lifetime
# and avoids repeated fast_info calls on every batch request.
_ticker_currency_cache: dict = {}


def _get_exchange_ticker_currency(yf_sym: str) -> str:
    """Return the currency for an exchange-listed ticker, using a persistent cache."""
    if yf_sym in _ticker_currency_cache:
        return _ticker_currency_cache[yf_sym]
    try:
        import yfinance as yf
        ccy = yf.Ticker(yf_sym).fast_info.currency or "GBP"
        _ticker_currency_cache[yf_sym] = ccy
        logger.info(f"Currency cached for {yf_sym}: {ccy}")
        return ccy
    except Exception as e:
        logger.warning(f"Could not get currency for {yf_sym}: {e}, defaulting to GBP")
        return "GBP"

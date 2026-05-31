# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

from datetime import datetime, time
import zoneinfo

_GMT = zoneinfo.ZoneInfo("Europe/London")
_ET = zoneinfo.ZoneInfo("America/New_York")


def _any_equity_market_open() -> bool:
    """True during the combined LSE (08:00 GMT) → NYSE close (21:00 GMT) window on weekdays."""
    now = datetime.now(_GMT)
    if now.weekday() >= 5:
        return False
    return time(8, 0) <= now.time() <= time(21, 0)


def _futures_market_open() -> bool:
    """True when CME Globex is likely open (nearly 24/5).
    Closed all day Saturday; opens Sunday at 17:00 ET."""
    now = datetime.now(_ET)
    wd = now.weekday()
    if wd == 5:  # Saturday — always closed
        return False
    if wd == 6:  # Sunday — opens 17:00 ET
        return now.time() >= time(17, 0)
    return True  # Mon–Fri always open (ignoring the ~1-hour daily maintenance window)


def _fx_market_open() -> bool:
    """Forex is 24/5 — only closed on full weekends."""
    return datetime.now(_GMT).weekday() < 5


def should_fetch(symbol: str) -> bool:
    """Whether a fresh yfinance fetch is warranted for this symbol.

    Called when the cache is stale to decide whether to hit yfinance or
    serve stale cached data instead.
    """
    if symbol.endswith("=X"):
        return _fx_market_open()
    if symbol.endswith("=F"):
        return _futures_market_open()
    return _any_equity_market_open()


def market_status() -> dict:
    """Return the current open/closed status for each market segment."""
    return {
        "equity": _any_equity_market_open(),
        "futures": _futures_market_open(),
        "fx": _fx_market_open(),
        "timestamp": datetime.now(_GMT).isoformat(),
    }

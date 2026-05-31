# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

import asyncio
import time
from collections import defaultdict
from typing import Optional

from api.config import RATE_LIMIT_ENABLED, RATE_LIMIT_REQUESTS, RATE_LIMIT_WINDOW, logger


class AsyncRateLimiter:
    """Thread-safe async rate limiter"""

    def __init__(self, max_requests: int, window_seconds: int):
        self.max_requests = max_requests
        self.window = window_seconds
        self.requests = defaultdict(list)
        self._lock = asyncio.Lock()

    async def is_allowed(self, client_ip: str) -> bool:
        if not RATE_LIMIT_ENABLED:
            return True

        async with self._lock:
            now = time.time()
            self.requests[client_ip] = [
                t for t in self.requests[client_ip] if now - t < self.window
            ]

            if len(self.requests[client_ip]) >= self.max_requests:
                return False

            self.requests[client_ip].append(now)
            return True


class FXRateCache:
    """Async-safe FX rate cache with TTL"""

    def __init__(self, ttl: int = 3600):
        self.cache = {}
        self.timestamp = 0
        self.ttl = ttl
        self._lock = asyncio.Lock()

    async def get(self, key: str) -> Optional[float]:
        async with self._lock:
            if time.time() - self.timestamp > self.ttl:
                logger.info("FX rate cache expired, clearing")
                self.cache.clear()
                self.timestamp = time.time()

            return self.cache.get(key)

    async def set(self, key: str, value: float):
        async with self._lock:
            self.cache[key] = value


class StockPriceCache:
    """Per-ticker price cache with individual TTL entries (native currency)"""

    def __init__(self, ttl: int = 60):
        self.cache = {}
        self.ttl = ttl
        self._lock = asyncio.Lock()

    async def get(self, symbol: str, allow_stale: bool = False) -> Optional[dict]:
        async with self._lock:
            entry = self.cache.get(symbol)
            if not entry:
                return None
            if time.time() - entry["ts"] < self.ttl or allow_stale:
                return entry
            del self.cache[symbol]
            return None

    async def set(
        self, symbol: str, current: float, prev_close: float, stock_currency: str
    ):
        async with self._lock:
            self.cache[symbol] = {
                "current": current,
                "prev_close": prev_close,
                "stock_currency": stock_currency,
                "ts": time.time(),
            }

    async def get_many(self, symbols: list) -> tuple[dict, dict, list]:
        """Return (fresh_hits, stale_hits, true_misses) for a batch of symbols.

        Stale entries are kept in cache so callers can decide whether to serve
        them (market closed) or refetch (market open).
        """
        fresh = {}
        stale = {}
        misses = []
        async with self._lock:
            now = time.time()
            for sym in symbols:
                entry = self.cache.get(sym)
                if entry:
                    if now - entry["ts"] < self.ttl:
                        fresh[sym] = entry
                    else:
                        stale[sym] = entry  # keep in cache, let caller decide
                else:
                    misses.append(sym)
        return fresh, stale, misses


class HistoryCache:
    """Per-(symbol, period, interval) cache for historical price series.

    Historical daily/weekly closes are immutable once the session ends, so TTLs
    are long. Intraday data (15m etc.) is shorter-lived since bars are still forming.
    """

    _INTRADAY = {"1m", "2m", "5m", "15m", "30m", "60m", "90m", "1h"}

    def __init__(self):
        self.cache: dict = {}
        self._lock = asyncio.Lock()

    def _ttl(self, interval: str) -> int:
        return 3600 if interval in self._INTRADAY else 43200  # 1h intraday, 12h daily+

    async def get(self, symbol: str, period: str, interval: str) -> Optional[dict]:
        key = (symbol, period, interval)
        async with self._lock:
            entry = self.cache.get(key)
            if not entry:
                return None
            if time.time() - entry["ts"] < self._ttl(interval):
                return entry
            del self.cache[key]
            return None

    async def set(self, symbol: str, period: str, interval: str, data: dict):
        key = (symbol, period, interval)
        async with self._lock:
            self.cache[key] = {**data, "ts": time.time()}


rate_limiter = AsyncRateLimiter(RATE_LIMIT_REQUESTS, RATE_LIMIT_WINDOW)
fx_cache = FXRateCache()
stock_cache = StockPriceCache(ttl=600)
history_cache = HistoryCache()

# Shared in-flight request deduplication state
in_flight_requests = {}
in_flight_lock = asyncio.Lock()

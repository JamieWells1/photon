# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
/stocks/history endpoint tests.
Run when modifying _fetch_history_sync, the history route handler,
or HistoryCache.

Cold-cache performance test runs first.
"""

import time
import sys
from base import ProxyTestBase, BASE_URL


class HistoryTests(ProxyTestBase):

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _valid_history(self, data, symbol: str) -> bool:
        """True when response contains a well-formed history entry for symbol."""
        if not data or "result" not in data:
            return False
        entry = data["result"].get(symbol)
        if not entry or "error" in entry:
            return False
        if "dates" not in entry or "closes" not in entry:
            return False
        if not isinstance(entry["dates"], list) or not isinstance(entry["closes"], list):
            return False
        if len(entry["dates"]) == 0 or len(entry["dates"]) != len(entry["closes"]):
            return False
        try:
            float(entry["closes"][0])
            return True
        except (TypeError, ValueError):
            return False

    def _history(self, symbols: str, period: str = "1y", interval: str = "1d"):
        return self._get(
            "/stocks/history",
            {"symbols": symbols, "period": period, "interval": interval},
            timeout=120,
        )

    # ------------------------------------------------------------------
    # Tests
    # ------------------------------------------------------------------

    def test_history_cold_performance(self):
        """US + LSE mixed batch must respond within 90s on a cold cache."""
        name = "History Performance — AAPL + VUAG cold < 90s"
        t0 = time.time()
        data = self._history("AAPL,VUAG", period="1y", interval="1d")
        elapsed = time.time() - t0
        self.logger.info(f"  Cold history fetch in {elapsed:.2f}s")
        if elapsed > 90:
            self._fail(name, f"{elapsed:.2f}s exceeds 90s limit")
            return
        failures = [s for s in ("AAPL", "VUAG") if not self._valid_history(data, s)]
        if failures:
            self._fail(name, f"Missing/invalid: {', '.join(failures)}")
        else:
            self._pass(name)

    def test_history_us_stock_1y(self):
        """1y daily history for a US stock returns ~252 trading days."""
        name = "History — US Stock 1y daily (AAPL ~252 points)"
        data = self._history("AAPL", period="1y", interval="1d")
        if not self._valid_history(data, "AAPL"):
            self._fail(name, f"Invalid response: {data}")
            return
        n = len(data["result"]["AAPL"]["closes"])
        self.logger.info(f"  AAPL: {n} daily closes over 1y")
        # Allow a generous range — holidays / partial years vary
        if 200 <= n <= 270:
            self._pass(name)
        else:
            self._fail(name, f"{n} points — expected 200-270 for 1y daily")

    def test_history_lse_ticker(self):
        """LSE ETF (VUAG) returns valid 1y daily history."""
        name = "History — LSE Ticker 1y daily (VUAG)"
        data = self._history("VUAG", period="1y", interval="1d")
        if not self._valid_history(data, "VUAG"):
            self._fail(name, f"Invalid response: {data}")
            return
        n = len(data["result"]["VUAG"]["closes"])
        self.logger.info(f"  VUAG: {n} daily closes, latest={data['result']['VUAG']['closes'][-1]:.2f}")
        if n >= 100:
            self._pass(name)
        else:
            self._fail(name, f"Only {n} points — suspiciously few for VUAG 1y")

    def test_history_euronext_ticker(self):
        """Euronext ticker (VWCE) returns valid 1y daily history."""
        name = "History — Euronext Ticker 1y daily (VWCE)"
        data = self._history("VWCE", period="1y", interval="1d")
        if not self._valid_history(data, "VWCE"):
            self._fail(name, f"Invalid response: {data}")
            return
        n = len(data["result"]["VWCE"]["closes"])
        self.logger.info(f"  VWCE: {n} closes, latest={data['result']['VWCE']['closes'][-1]:.2f}")
        if n >= 100:
            self._pass(name)
        else:
            self._fail(name, f"Only {n} points")

    def test_history_futures(self):
        """Commodity futures (GC=F gold) returns valid history."""
        name = "History — Futures (GC=F gold) 1y daily"
        data = self._history("GC=F", period="1y", interval="1d")
        if not self._valid_history(data, "GC=F"):
            self._fail(name, f"Invalid response: {data}")
            return
        n = len(data["result"]["GC=F"]["closes"])
        self.logger.info(f"  GC=F: {n} closes, latest={data['result']['GC=F']['closes'][-1]:.2f}")
        if n >= 100:
            self._pass(name)
        else:
            self._fail(name, f"Only {n} points")

    def test_history_max_weekly(self):
        """Max period with weekly interval returns multi-year history."""
        name = "History — Max period weekly (AAPL > 500 weeks)"
        data = self._history("AAPL", period="max", interval="1wk")
        if not self._valid_history(data, "AAPL"):
            self._fail(name, f"Invalid response: {data}")
            return
        n = len(data["result"]["AAPL"]["closes"])
        self.logger.info(f"  AAPL max/weekly: {n} weeks")
        if n >= 500:
            self._pass(name)
        else:
            self._fail(name, f"Only {n} weekly bars for max period — expected 500+")

    def test_history_15m_intraday(self):
        """15m intraday for 5d returns bars with datetime-format timestamps."""
        name = "History — 15m intraday (AAPL 5d)"
        data = self._history("AAPL", period="5d", interval="15m")
        if not self._valid_history(data, "AAPL"):
            self._fail(name, f"Invalid response: {data}")
            return
        dates = data["result"]["AAPL"]["dates"]
        # Intraday dates should include time component (T separator)
        if not any("T" in d for d in dates):
            self._fail(name, f"Intraday dates missing time component: {dates[:3]}")
            return
        self.logger.info(f"  AAPL 15m: {len(dates)} bars, first={dates[0]}")
        self._pass(name)

    def test_history_batch_mixed(self):
        """Batch history for US + LSE + Euronext + futures all return valid data."""
        name = "History — Batch mixed (AAPL, VUAG, VWCE, GC=F)"
        symbols = "AAPL,VUAG,VWCE,GC=F"
        data = self._history(symbols, period="1y", interval="1d")
        failures = [s for s in ("AAPL", "VUAG", "VWCE", "GC=F") if not self._valid_history(data, s)]
        if failures:
            self._fail(name, f"Missing/invalid: {', '.join(failures)}")
        else:
            for s in ("AAPL", "VUAG", "VWCE", "GC=F"):
                n = len(data["result"][s]["closes"])
                self.logger.info(f"  {s}: {n} closes")
            self._pass(name)

    def test_history_lse_gbx_closes_in_pounds(self):
        """GBX-denominated LSE ETF (VUAG) closes must be in GBP, not pence.

        VUAG trades at roughly £100-£200/share. If GBX→GBP conversion is
        missing the closes come back 100× too large (10,000-20,000+), which
        inflates the portfolio performance chart.
        """
        name = "History — LSE GBX closes returned in GBP not pence (VUAG)"
        data = self._history("VUAG", period="1y", interval="1d")
        if not self._valid_history(data, "VUAG"):
            self._fail(name, f"Invalid response: {data}")
            return
        closes = data["result"]["VUAG"]["closes"]
        latest = closes[-1]
        self.logger.info(f"  VUAG latest close: {latest:.4f}")
        # In GBP a sane range is £50–£500; pence values would be 5,000–50,000+
        if latest > 1000:
            self._fail(name, f"Close {latest:.2f} looks like pence — GBX conversion missing")
        elif latest < 10:
            self._fail(name, f"Close {latest:.4f} suspiciously low — check ticker/data")
        else:
            self._pass(name)

    def test_history_cache_hit(self):
        """Second request for same symbols returns faster (cache hit)."""
        name = "History — Cache hit faster than cold fetch"
        # First request (may be warm from earlier tests, that's fine)
        self._history("AAPL", period="1y", interval="1d")
        # Second request should be cached
        t0 = time.time()
        data = self._history("AAPL", period="1y", interval="1d")
        elapsed = time.time() - t0
        self.logger.info(f"  Cache hit returned in {elapsed:.2f}s")
        if not self._valid_history(data, "AAPL"):
            self._fail(name, "Invalid response on cache hit")
        elif elapsed < 5.0:
            self._pass(name)
        else:
            self._fail(name, f"{elapsed:.2f}s — expected <5s for cache hit")

    def test_history_meta_in_response(self):
        """Response includes meta.period and meta.interval fields."""
        name = "History — Response includes meta fields"
        data = self._get(
            "/stocks/history",
            {"symbols": "AAPL", "period": "6mo", "interval": "1d"},
        )
        if not data or "meta" not in data:
            self._fail(name, "Missing 'meta' in response")
            return
        meta = data["meta"]
        if meta.get("period") == "6mo" and meta.get("interval") == "1d":
            self._pass(name)
        else:
            self._fail(name, f"Unexpected meta: {meta}")

    def test_history_invalid_period(self):
        """Invalid period returns 400."""
        name = "History — Invalid period → 400"
        _, status = self._make_request(
            "/stocks/history",
            {"symbols": "AAPL", "period": "99y", "interval": "1d"},
        )
        if status == 400:
            self._pass(name)
        else:
            self._fail(name, f"Expected 400, got {status}")

    def test_history_invalid_interval(self):
        """Invalid interval returns 400."""
        name = "History — Invalid interval → 400"
        _, status = self._make_request(
            "/stocks/history",
            {"symbols": "AAPL", "period": "1y", "interval": "4d"},
        )
        if status == 400:
            self._pass(name)
        else:
            self._fail(name, f"Expected 400, got {status}")

    def test_history_no_symbols(self):
        """Missing required symbols param returns 422 (FastAPI validation)."""
        name = "History — No symbols → 422"
        _, status = self._make_request("/stocks/history", {"period": "1y"})
        if status == 422:
            self._pass(name)
        else:
            self._fail(name, f"Expected 422, got {status}")

    def _run_all(self):
        self.test_history_cold_performance()
        self.test_history_us_stock_1y()
        self.test_history_lse_ticker()
        self.test_history_euronext_ticker()
        self.test_history_futures()
        self.test_history_max_weekly()
        self.test_history_15m_intraday()
        self.test_history_batch_mixed()
        self.test_history_lse_gbx_closes_in_pounds()
        self.test_history_cache_hit()
        self.test_history_meta_in_response()
        self.test_history_invalid_period()
        self.test_history_invalid_interval()
        self.test_history_no_symbols()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = HistoryTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

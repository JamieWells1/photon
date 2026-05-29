# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
/stocks/batch endpoint tests.
Run when modifying _fetch_batch_stock_data_sync, the batch route handler,
or the single-vs-multi-ticker branching logic.

The performance test runs first (before any cache is warm) to get a true
cold-fetch measurement.
"""

import time
import sys
from base import ProxyTestBase, BASE_URL


class BatchTests(ProxyTestBase):

    def test_batch_response_time_cold(self):
        """
        Four ETF tickers (mixed LSE + Euronext) must respond within 60s on a
        cold cache. Run this first so the cache is genuinely empty.
        """
        name = "Batch Performance — 4 ETFs cold < 60s"
        t0 = time.time()
        data = self._get(
            "/stocks/batch",
            {"symbols": "VUAG,EQGB,VWCE,XUSE", "currency": "GBP"},
            timeout=90,
        )
        elapsed = time.time() - t0
        self.logger.info(f"  Cold batch completed in {elapsed:.2f}s")
        if elapsed > 60:
            self._fail(name, f"{elapsed:.2f}s exceeds 60s limit")
            return
        failures = [s for s in ("VUAG", "EQGB", "VWCE", "XUSE") if not self._valid(data, s)]
        if failures:
            self._fail(name, f"Missing/invalid: {', '.join(failures)}")
        else:
            self._pass(name)

    def test_us_stocks(self):
        name = "Batch — US Stocks (AAPL, MSFT, GOOGL)"
        data = self._get("/stocks/batch", {"symbols": "AAPL,MSFT,GOOGL"})
        failures = [s for s in ("AAPL", "MSFT", "GOOGL") if not self._valid(data, s)]
        if not failures:
            for s in ("AAPL", "MSFT", "GOOGL"):
                self.logger.info(f"  {s}: ${self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")

    def test_us_stocks_gbp(self):
        name = "Batch — US Stocks with GBP (AAPL, MSFT)"
        data = self._get("/stocks/batch", {"symbols": "AAPL,MSFT", "currency": "GBP"})
        failures = [s for s in ("AAPL", "MSFT") if not self._valid(data, s)]
        if not failures:
            for s in ("AAPL", "MSFT"):
                self.logger.info(f"  {s}: £{self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Currency conversion failed for: {', '.join(failures)}")

    def test_single_ticker(self):
        """Single-symbol batch hits a different code path — must work correctly."""
        name = "Batch — Single Ticker Code Path (AAPL)"
        data = self._get("/stocks/batch", {"symbols": "AAPL"})
        price = self._price(data, "AAPL")
        if price and price > 0:
            self.logger.info(f"  AAPL: ${price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid response: {data}")

    def test_lse_tickers(self):
        name = "Batch — LSE Tickers (VUSA, VWRL, VUAG)"
        data = self._get("/stocks/batch", {"symbols": "VUSA,VWRL,VUAG"})
        failures = [s for s in ("VUSA", "VWRL", "VUAG") if not self._valid(data, s)]
        if not failures:
            for s in ("VUSA", "VWRL", "VUAG"):
                self.logger.info(f"  {s}: {self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")

    def test_lse_gbp_pence_conversion(self):
        """GBX-priced LSE ETFs must be converted to GBP (÷100), not left in pence."""
        name = "Batch — LSE GBX→GBP (VUAG, VUSA both under £500)"
        data = self._get("/stocks/batch", {"symbols": "VUAG,VUSA", "currency": "GBP"})
        vuag = self._price(data, "VUAG")
        vusa = self._price(data, "VUSA")
        if vuag is None or vusa is None:
            self._fail(name, "Could not extract prices")
            return
        if vuag > 500 or vusa > 500:
            self._fail(name, f"VUAG={vuag:.2f} VUSA={vusa:.2f} — pence may not be converted")
        else:
            self.logger.info(f"  VUAG: £{vuag:.2f}, VUSA: £{vusa:.2f}")
            self._pass(name)

    def test_euronext_ticker(self):
        name = "Batch — Euronext Ticker (VWCE)"
        data = self._get("/stocks/batch", {"symbols": "VWCE"})
        price = self._price(data, "VWCE")
        if price and price > 0:
            self.logger.info(f"  VWCE: {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid response: {data}")

    def test_euronext_gbp(self):
        name = "Batch — Euronext EUR→GBP (VWCE £50-200)"
        data = self._get("/stocks/batch", {"symbols": "VWCE", "currency": "GBP"})
        price = self._price(data, "VWCE")
        if price and 50 < price < 200:
            self.logger.info(f"  VWCE GBP: £{price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"£{price:.2f} outside expected range £50-200")
        else:
            self._fail(name, f"Invalid response: {data}")

    def test_mixed_us_lse(self):
        name = "Batch — Mixed US + LSE (AAPL, VUSA, MSFT)"
        data = self._get("/stocks/batch", {"symbols": "AAPL,VUSA,MSFT", "currency": "GBP"})
        failures = [s for s in ("AAPL", "VUSA", "MSFT") if not self._valid(data, s)]
        if not failures:
            for s in ("AAPL", "VUSA", "MSFT"):
                self.logger.info(f"  {s}: £{self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Failed for: {', '.join(failures)}")

    def test_mixed_us_lse_euronext(self):
        name = "Batch — Mixed US + LSE + Euronext (AAPL, VUAG, VWCE)"
        data = self._get("/stocks/batch", {"symbols": "AAPL,VUAG,VWCE", "currency": "GBP"})
        failures = [s for s in ("AAPL", "VUAG", "VWCE") if not self._valid(data, s)]
        if not failures:
            for s in ("AAPL", "VUAG", "VWCE"):
                self.logger.info(f"  {s}: £{self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Failed for: {', '.join(failures)}")

    def test_full_peepstock_portfolio(self):
        """
        The exact portfolio combination that exposed the EUR/USD bug.
        All four must return GBP prices within plausible ranges.
        """
        name = "Batch — Full PeepStock Portfolio (VUAG, EQGB, VWCE, XUSE) → GBP"
        data = self._get(
            "/stocks/batch",
            {"symbols": "VUAG,EQGB,VWCE,XUSE", "currency": "GBP"},
        )
        failures = [s for s in ("VUAG", "EQGB", "VWCE", "XUSE") if not self._valid(data, s)]
        if failures:
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")
            return

        ranges = {"VUAG": (60, 200), "EQGB": (450, 750), "VWCE": (50, 200), "XUSE": (2, 20)}
        out_of_range = []
        for sym, (lo, hi) in ranges.items():
            p = self._price(data, sym)
            self.logger.info(f"  {sym}: £{p:.2f}")
            if not (lo < p < hi):
                out_of_range.append(f"{sym}=£{p:.2f} (expected £{lo}-£{hi})")

        if out_of_range:
            self._fail(name, "; ".join(out_of_range))
        else:
            self._pass(name)

    def test_consistency_with_single(self):
        """Batch and single endpoints must return prices within 2% of each other."""
        name = "Batch vs Single Consistency (AAPL)"
        single = self._price(self._get("/stocks/AAPL", {"currency": "GBP"}), "AAPL")
        batch = self._price(
            self._get("/stocks/batch", {"symbols": "AAPL", "currency": "GBP"}), "AAPL"
        )
        if not single or not batch:
            self._fail(name, "Could not fetch prices")
            return
        pct = abs(single - batch) / single * 100
        self.logger.info(f"  Single: £{single:.2f}, Batch: £{batch:.2f} ({pct:.2f}% diff)")
        if pct < 2.0:
            self._pass(name)
        else:
            self._fail(name, f"Diverged by {pct:.2f}%")

    def _run_all(self):
        # Performance test runs first while cache is cold
        self.test_batch_response_time_cold()
        self.test_us_stocks()
        self.test_us_stocks_gbp()
        self.test_single_ticker()
        self.test_lse_tickers()
        self.test_lse_gbp_pence_conversion()
        self.test_euronext_ticker()
        self.test_euronext_gbp()
        self.test_mixed_us_lse()
        self.test_mixed_us_lse_euronext()
        self.test_full_peepstock_portfolio()
        self.test_consistency_with_single()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = BatchTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

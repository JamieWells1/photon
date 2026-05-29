# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Ticker routing tests — translation, aliases, and undiscovered-ticker fallback.
Run when modifying LSE_TICKERS, EURONEXT_TICKERS, TICKER_ALIASES, or the
.L retry logic in _fetch_batch_stock_data_sync.
"""

import sys
from base import ProxyTestBase, BASE_URL


class RoutingTests(ProxyTestBase):

    # --- LSE translation ---------------------------------------------------

    def test_lse_ticker_translation(self):
        """VUSA (no suffix) must be resolved to VUSA.L automatically."""
        name = "LSE Translation — VUSA → VUSA.L"
        data = self._get("/stocks/VUSA")
        price = self._price(data, "VUSA")
        if price and 50 < price < 300:
            self.logger.info(f"  VUSA: {price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"Unrealistic price: {price:.2f}")
        else:
            self._fail(name, f"No valid response: {data}")

    def test_lse_batch_translation(self):
        """LSE tickers in a batch must all be translated to .L."""
        name = "LSE Translation — Batch (VUSA, VWRL, VUAG)"
        data = self._get("/stocks/batch", {"symbols": "VUSA,VWRL,VUAG"})
        failures = [s for s in ("VUSA", "VWRL", "VUAG") if not self._valid(data, s)]
        if not failures:
            for s in ("VUSA", "VWRL", "VUAG"):
                self.logger.info(f"  {s}: {self._price(data, s):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")

    # --- Euronext translation ----------------------------------------------

    def test_euronext_ticker_translation(self):
        """VWCE (no suffix) must be resolved to VWCE.AS automatically."""
        name = "Euronext Translation — VWCE → VWCE.AS"
        data = self._get("/stocks/VWCE")
        price = self._price(data, "VWCE")
        if price and price > 0:
            self.logger.info(f"  VWCE: {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid response: {data}")

    def test_euronext_batch_translation(self):
        """VWCE in a batch must be translated to VWCE.AS."""
        name = "Euronext Translation — Batch (VWCE)"
        data = self._get("/stocks/batch", {"symbols": "VWCE"})
        price = self._price(data, "VWCE")
        if price and price > 0:
            self.logger.info(f"  VWCE: {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid response: {data}")

    # --- Ticker alias ------------------------------------------------------

    def test_eqgb_lse_translation(self):
        """EQGB is in LSE_TICKERS and must resolve directly to EQGB.L (~57,000 GBX)."""
        name = "LSE Translation — EQGB → EQGB.L (single)"
        data = self._get("/stocks/EQGB")
        price = self._price(data, "EQGB")
        if price and price > 0:
            self.logger.info(f"  EQGB (via EQGB.L): {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid response: {data}")

    def test_eqgb_lse_translation_batch(self):
        """EQGB must resolve to EQGB.L in a batch."""
        name = "LSE Translation — EQGB → EQGB.L (batch)"
        data = self._get("/stocks/batch", {"symbols": "EQGB,VUAG"})
        eqgb_ok = self._valid(data, "EQGB")
        vuag_ok = self._valid(data, "VUAG")
        if eqgb_ok and vuag_ok:
            self.logger.info(
                f"  EQGB: {self._price(data, 'EQGB'):.2f}, VUAG: {self._price(data, 'VUAG'):.2f}"
            )
            self._pass(name)
        else:
            failures = [s for s, ok in (("EQGB", eqgb_ok), ("VUAG", vuag_ok)) if not ok]
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")

    # --- .L fallback (undiscovered tickers) --------------------------------

    def test_unknown_lse_single(self):
        """A ticker not in LSE_TICKERS should fall back to the .L suffix (single)."""
        name = "LSE Fallback — Undiscovered Ticker Single (HMWO)"
        data = self._get("/stocks/HMWO")
        price = self._price(data, "HMWO")
        if price and price > 0:
            self.logger.info(f"  HMWO via .L fallback: {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid response — .L fallback may be broken: {data}")

    def test_unknown_lse_single_gbp(self):
        """Undiscovered LSE ticker with GBP conversion should still resolve."""
        name = "LSE Fallback — Undiscovered Ticker Single with GBP (HMWO)"
        data = self._get("/stocks/HMWO", {"currency": "GBP"})
        price = self._price(data, "HMWO")
        if price and price > 0:
            self.logger.info(f"  HMWO in GBP: £{price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid response: {data}")

    def test_unknown_lse_batch(self):
        """Undiscovered LSE ticker in a batch should fall back to .L retry."""
        name = "LSE Fallback — Undiscovered Ticker Batch (AAPL, HMWO)"
        data = self._get("/stocks/batch", {"symbols": "AAPL,HMWO"})
        aapl_ok = self._valid(data, "AAPL")
        hmwo_ok = self._valid(data, "HMWO")
        if aapl_ok and hmwo_ok:
            self.logger.info(
                f"  AAPL: ${self._price(data, 'AAPL'):.2f}, "
                f"HMWO via .L fallback: {self._price(data, 'HMWO'):.2f}"
            )
            self._pass(name)
        else:
            failures = [s for s, ok in (("AAPL", aapl_ok), ("HMWO", hmwo_ok)) if not ok]
            self._fail(name, f"Invalid/missing: {', '.join(failures)}")

    def test_invalid_ticker_isolated_in_batch(self):
        """A completely unknown ticker must not prevent valid symbols returning."""
        name = "Error Isolation — Invalid Ticker Doesn't Poison Batch"
        data = self._get("/stocks/batch", {"symbols": "AAPL,XYZINVALIDTICKER999"})
        if not data or "result" not in data:
            self._fail(name, "No result returned at all")
            return
        if self._valid(data, "AAPL"):
            self.logger.info(f"  AAPL still returned: ${self._price(data, 'AAPL'):.2f}")
            self._pass(name)
        else:
            self._fail(name, "AAPL missing — invalid ticker poisoned the batch")

    def test_already_suffixed_ticker(self):
        """Submitting VUSA.L directly should not double-suffix or crash."""
        name = "Already-Suffixed Ticker (VUSA.L)"
        data = self._get("/stocks/VUSA.L")
        if data and "result" in data:
            key = next(iter(data["result"]), None)
            if key:
                entry = data["result"][key]
                if "c" in entry and "o" in entry:
                    self.logger.info(f"  VUSA.L returned as '{key}': {entry['c'][0]}")
                    self._pass(name)
                    return
        if data and "error" in data:
            self.logger.info(f"  VUSA.L returned expected error: {data['error']}")
            self._pass(name)
        else:
            self._fail(name, f"Unexpected response: {data}")

    def _run_all(self):
        self.test_lse_ticker_translation()
        self.test_lse_batch_translation()
        self.test_euronext_ticker_translation()
        self.test_euronext_batch_translation()
        self.test_eqgb_lse_translation()
        self.test_eqgb_lse_translation_batch()
        self.test_unknown_lse_single()
        self.test_unknown_lse_single_gbp()
        self.test_unknown_lse_batch()
        self.test_invalid_ticker_isolated_in_batch()
        self.test_already_suffixed_ticker()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = RoutingTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

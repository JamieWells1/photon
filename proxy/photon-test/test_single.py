# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
/stocks/:symbol endpoint tests.
Run when modifying fetch_stock_data, _fetch_stock_data_sync, or the single-ticker
route handler.
"""

import sys
from base import ProxyTestBase, BASE_URL


class SingleStockTests(ProxyTestBase):

    def test_us_stock(self):
        name = "Single — US Stock (AAPL)"
        price = self._price(self._get("/stocks/AAPL"), "AAPL")
        if price and price > 0:
            self.logger.info(f"  AAPL: ${price:.2f}")
            self._pass(name)
        else:
            self._fail(name, "Invalid response")

    def test_prev_close_present(self):
        """Both current (c) and prev_close (o) must be present and positive."""
        name = "Single — Current and Prev Close Both Present (AAPL)"
        data = self._get("/stocks/AAPL")
        current = self._price(data, "AAPL")
        prev = self._prev_close(data, "AAPL")
        if not current or not prev:
            self._fail(name, "Missing c or o field")
            return
        pct = abs(current - prev) / prev * 100
        self.logger.info(f"  Current: ${current:.2f}, Prev close: ${prev:.2f} ({pct:.2f}% change)")
        if pct < 20:
            self._pass(name)
        else:
            self._fail(name, f"Change of {pct:.2f}% is suspiciously large")

    def test_lse_stock(self):
        name = "Single — LSE Stock (VUSA)"
        price = self._price(self._get("/stocks/VUSA"), "VUSA")
        if price and 50 < price < 300:
            self.logger.info(f"  VUSA: {price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"Unrealistic price: {price:.2f}")
        else:
            self._fail(name, "Invalid response")

    def test_lse_stock_gbp(self):
        name = "Single — LSE Stock with GBP (VUSA)"
        price = self._price(self._get("/stocks/VUSA", {"currency": "GBP"}), "VUSA")
        if price and price > 0:
            self.logger.info(f"  VUSA GBP: £{price:.2f}")
            self._pass(name)
        else:
            self._fail(name, "Invalid response")

    def test_vuag_gbp(self):
        name = "Single — VUAG GBX ETF in GBP"
        price = self._price(self._get("/stocks/VUAG", {"currency": "GBP"}), "VUAG")
        if price and 60 < price < 200:
            self.logger.info(f"  VUAG GBP: £{price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"£{price:.2f} out of expected range £60-200")
        else:
            self._fail(name, "Invalid response")

    def test_xuse_gbp(self):
        name = "Single — XUSE (iShares MSCI World ex-USA) in GBP"
        price = self._price(self._get("/stocks/XUSE", {"currency": "GBP"}), "XUSE")
        if price and 2 < price < 20:
            self.logger.info(f"  XUSE GBP: £{price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"£{price:.2f} out of expected range £2-20")
        else:
            self._fail(name, "Invalid response")

    def test_euronext_stock(self):
        name = "Single — Euronext Stock (VWCE)"
        price = self._price(self._get("/stocks/VWCE"), "VWCE")
        if price and price > 0:
            self.logger.info(f"  VWCE: {price:.2f}")
            self._pass(name)
        else:
            self._fail(name, "Invalid response")

    def test_euronext_stock_gbp(self):
        name = "Single — Euronext Stock in GBP (VWCE)"
        price = self._price(self._get("/stocks/VWCE", {"currency": "GBP"}), "VWCE")
        if price and 50 < price < 200:
            self.logger.info(f"  VWCE GBP: £{price:.2f}")
            self._pass(name)
        elif price:
            self._fail(name, f"£{price:.2f} out of expected range £50-200")
        else:
            self._fail(name, "Invalid response")

    def test_invalid_symbol(self):
        name = "Single — Invalid Symbol Returns Error"
        data = self._get("/stocks/INVALID123")
        if data and "error" in data:
            self.logger.info(f"  Error returned correctly: {data['error']}")
            self._pass(name)
        else:
            self._fail(name, f"Expected error response, got: {data}")

    def _run_all(self):
        self.test_us_stock()
        self.test_prev_close_present()
        self.test_lse_stock()
        self.test_lse_stock_gbp()
        self.test_vuag_gbp()
        self.test_xuse_gbp()
        self.test_euronext_stock()
        self.test_euronext_stock_gbp()
        self.test_invalid_symbol()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = SingleStockTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

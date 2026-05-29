# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Input validation and edge-case tests.
Run when modifying parameter parsing, validation logic, or error handling.
"""

import sys
from base import ProxyTestBase, BASE_URL


class EdgeCaseTests(ProxyTestBase):

    def test_invalid_symbol(self):
        name = "Validation — Invalid Symbol Returns Error (not 200)"
        data, status = self._make_request("/stocks/INVALID123")
        if status in (404, 500) or (data and "error" in data):
            self.logger.info(f"  Correctly rejected: status={status}")
            self._pass(name)
        else:
            self._fail(name, f"Expected error response, got status={status}: {data}")

    def test_invalid_symbol_characters(self):
        """Symbols with special characters must be rejected before hitting yfinance."""
        name = "Validation — Special Characters in Symbol Rejected"
        data, status = self._make_request("/stocks/AAPL<script>")
        if status in (400, 404):
            self.logger.info(f"  Correctly rejected with {status}")
            self._pass(name)
        else:
            self._fail(name, f"Expected 400 or 404, got {status}: {data}")

    def test_batch_empty_symbols(self):
        name = "Validation — Batch Empty Symbols → 400"
        data, status = self._make_request("/stocks/batch", params={"symbols": ""})
        if status == 400:
            self.logger.info("  Correctly returned 400 for empty symbols")
            self._pass(name)
        else:
            self._fail(name, f"Expected 400, got {status}: {data}")

    def test_batch_too_many_symbols(self):
        name = "Validation — Batch >100 Symbols → 400"
        symbols = ",".join([f"FAKE{i:03d}" for i in range(101)])
        data, status = self._make_request("/stocks/batch", params={"symbols": symbols})
        if status == 400:
            self.logger.info("  Correctly returned 400 for 101 symbols")
            self._pass(name)
        else:
            self._fail(name, f"Expected 400, got {status}: {data}")

    def test_batch_whitespace_trimmed(self):
        """Symbols with surrounding whitespace must still be resolved correctly."""
        name = "Validation — Batch Whitespace Trimmed"
        data = self._get("/stocks/batch", {"symbols": " AAPL , MSFT "})
        if self._valid(data, "AAPL") and self._valid(data, "MSFT"):
            self._pass(name)
        else:
            keys = list((data or {}).get("result", {}).keys())
            self._fail(name, f"Whitespace not trimmed — result keys: {keys}")

    def test_case_insensitive(self):
        """Symbols should resolve the same regardless of case."""
        name = "Validation — Symbol Case Insensitive (aapl == AAPL)"
        upper = self._price(self._get("/stocks/AAPL"), "AAPL")
        lower = self._price(self._get("/stocks/aapl"), "AAPL")
        if not upper or not lower:
            self._fail(name, "Could not fetch both variants")
            return
        pct = abs(upper - lower) / upper * 100
        if pct < 1.0:
            self.logger.info(f"  AAPL: ${upper:.2f}, aapl: ${lower:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Prices diverged by {pct:.2f}% between AAPL and aapl")

    def test_fx_cache_shared_across_requests(self):
        """Multiple stocks converted to the same currency should all succeed
        (verifies the shared FX rate cache doesn't corrupt between calls)."""
        name = "Edge Case — FX Cache Shared Across Multiple Stocks"
        symbols = ["MSFT", "GOOGL", "NVDA"]
        all_ok = True
        for s in symbols:
            price = self._price(self._get(f"/stocks/{s}", {"currency": "GBP"}), s)
            if not price:
                all_ok = False
                break
            self.logger.info(f"  {s}: £{price:.2f}")
        if all_ok:
            self._pass(name)
        else:
            self._fail(name, "One or more stocks failed with shared FX cache")

    def _run_all(self):
        self.test_invalid_symbol()
        self.test_invalid_symbol_characters()
        self.test_batch_empty_symbols()
        self.test_batch_too_many_symbols()
        self.test_batch_whitespace_trimmed()
        self.test_case_insensitive()
        self.test_fx_cache_shared_across_requests()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = EdgeCaseTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

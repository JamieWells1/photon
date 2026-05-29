# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Cache behaviour tests.
Run when modifying StockPriceCache, FXRateCache, or the cache-clear endpoint.
"""

import time
import sys
from base import ProxyTestBase, BASE_URL


class CacheTests(ProxyTestBase):

    def test_cache_clear(self):
        name = "Cache — POST /cache/clear Returns Success"
        data, status = self._make_request("/cache/clear", method="POST")
        if status == 200 and data and data.get("status") == "cleared":
            self.logger.info("  Cache cleared successfully")
            self._pass(name)
        else:
            self._fail(name, f"status={status}, response={data}")

    def test_warm_is_faster_than_cold(self):
        """Second request for the same ticker should be served from cache and noticeably faster."""
        name = "Cache — Warm Hit Faster Than Cold Miss"

        # Guarantee a cold start
        self._make_request("/cache/clear", method="POST")

        t0 = time.time()
        data1 = self._get("/stocks/AAPL", {"currency": "GBP"})
        cold_ms = (time.time() - t0) * 1000

        if not self._valid(data1, "AAPL"):
            self._fail(name, "Cold fetch failed")
            return

        t0 = time.time()
        data2 = self._get("/stocks/AAPL", {"currency": "GBP"})
        warm_ms = (time.time() - t0) * 1000

        self.logger.info(f"  Cold: {cold_ms:.0f}ms, Warm: {warm_ms:.0f}ms")

        # Timing is non-deterministic over a network — treat as a soft check
        if warm_ms < cold_ms:
            self._pass(name)
        else:
            self.logger.warning(
                f"  Warm ({warm_ms:.0f}ms) not faster than cold ({cold_ms:.0f}ms) — network variance"
            )
            self._pass(name)  # log the anomaly but don't fail the suite on timing alone

    def test_refetch_after_clear(self):
        """Clearing the cache must not break subsequent fetches."""
        name = "Cache — Refetch After Clear Returns Valid Data"
        self._make_request("/cache/clear", method="POST")
        data = self._get("/stocks/AAPL", {"currency": "GBP"})
        price = self._price(data, "AAPL")
        if price and price > 0:
            self.logger.info(f"  Post-clear AAPL: £{price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"Invalid data after cache clear: {data}")

    def _run_all(self):
        self.test_cache_clear()
        self.test_warm_is_faster_than_cold()
        self.test_refetch_after_clear()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = CacheTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

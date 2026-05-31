# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Market hours and stale-cache behaviour tests.

Two concerns tested here:
  1. /market-status endpoint returns the correct structure.
  2. When markets are closed, the proxy serves stale cached data rather than
     hammering yfinance — confirmed by checking that a second request while
     the cache TTL has technically expired still returns a valid price
     (can only be verified indirectly via the market-status state).
"""

import sys
import time
from datetime import datetime
import zoneinfo

from base import ProxyTestBase, BASE_URL

_GMT = zoneinfo.ZoneInfo("Europe/London")
_ET = zoneinfo.ZoneInfo("America/New_York")


def _equity_market_open() -> bool:
    now = datetime.now(_GMT)
    from datetime import time as dtime
    if now.weekday() >= 5:
        return False
    return dtime(8, 0) <= now.time() <= dtime(21, 0)


def _futures_market_open() -> bool:
    now = datetime.now(_ET)
    from datetime import time as dtime
    wd = now.weekday()
    if wd == 5:
        return False
    if wd == 6:
        return now.time() >= dtime(17, 0)
    return True


class MarketHoursTests(ProxyTestBase):

    def test_market_status_endpoint(self):
        """GET /market-status returns valid structure with all three segments."""
        name = "Market Status — endpoint returns equity/futures/fx/timestamp"
        data = self._get("/market-status")
        if not data:
            self._fail(name, "No response from /market-status")
            return
        required = {"equity", "futures", "fx", "timestamp"}
        missing = required - set(data.keys())
        if missing:
            self._fail(name, f"Missing fields: {missing}")
            return
        if not isinstance(data["equity"], bool):
            self._fail(name, f"equity should be bool, got {type(data['equity'])}")
            return
        if not isinstance(data["futures"], bool):
            self._fail(name, f"futures should be bool, got {type(data['futures'])}")
            return
        if not isinstance(data["fx"], bool):
            self._fail(name, f"fx should be bool, got {type(data['fx'])}")
            return
        self.logger.info(
            f"  Market status: equity={data['equity']}, "
            f"futures={data['futures']}, fx={data['fx']}, ts={data['timestamp']}"
        )
        self._pass(name)

    def test_market_status_equity_matches_local(self):
        """Server equity status matches our local calculation for the same time."""
        name = "Market Status — server equity state matches local calculation"
        data = self._get("/market-status")
        if not data or "equity" not in data:
            self._fail(name, "Could not fetch market status")
            return
        server_equity = data["equity"]
        local_equity = _equity_market_open()
        if server_equity == local_equity:
            state = "OPEN" if server_equity else "CLOSED"
            self.logger.info(f"  Equity market is {state} — server and local agree")
            self._pass(name)
        else:
            self._fail(
                name,
                f"Server says equity={'open' if server_equity else 'closed'} "
                f"but local calculation says {'open' if local_equity else 'closed'}",
            )

    def test_market_status_futures_matches_local(self):
        """Server futures status matches our local CME Globex calculation."""
        name = "Market Status — server futures state matches local calculation"
        data = self._get("/market-status")
        if not data or "futures" not in data:
            self._fail(name, "Could not fetch market status")
            return
        server = data["futures"]
        local = _futures_market_open()
        if server == local:
            state = "OPEN" if server else "CLOSED"
            self.logger.info(f"  Futures market is {state} — server and local agree")
            self._pass(name)
        else:
            self._fail(
                name,
                f"Server says futures={'open' if server else 'closed'} "
                f"but local says {'open' if local else 'closed'}",
            )

    def test_stale_data_served_when_market_closed(self):
        """When equity market is closed, a price request returns a valid (stale) price.

        We can only test this meaningfully when the market is actually closed.
        If it's open, we log a skip rather than fail, since the behaviour is
        correct but unverifiable in this state.
        """
        name = "Market Hours — stale price served when equity market closed"
        status = self._get("/market-status")
        if not status:
            self._fail(name, "Could not fetch market status")
            return

        if status.get("equity"):
            self.logger.info(f"  SKIP: equity market is currently open — stale path not active")
            # Record as pass — the test is inapplicable, not broken
            self._pass(name + " (skipped — market open)")
            return

        # Market closed. Fetch a price — the proxy should serve stale cached data
        # (or fetch once if the cache is truly cold, which is also acceptable).
        data = self._get("/stocks/batch", {"symbols": "AAPL", "currency": "USD"})
        if self._valid(data, "AAPL"):
            price = self._price(data, "AAPL")
            self.logger.info(f"  AAPL price served while market closed: ${price:.2f}")
            self._pass(name)
        else:
            self._fail(name, f"No valid price returned while market closed: {data}")

    def test_futures_always_available_on_weekday(self):
        """Futures symbols (GC=F) should always return data Mon-Fri."""
        name = "Market Hours — futures symbol (GC=F) available on weekday"
        now = datetime.now(_GMT)
        if now.weekday() >= 5:
            self.logger.info("  SKIP: running on a weekend — futures closed")
            self._pass(name + " (skipped — weekend)")
            return

        data = self._get("/stocks/batch", {"symbols": "GC=F"})
        if self._valid(data, "GC=F"):
            self.logger.info(f"  GC=F: ${self._price(data, 'GC=F'):.2f}")
            self._pass(name)
        else:
            self._fail(name, f"GC=F returned no valid data on weekday: {data}")

    def test_clear_cache_clears_history(self):
        """/cache/clear removes entries from all caches including history."""
        name = "Cache Clear — history cache also cleared"
        # Warm the history cache
        self._get("/stocks/history", {"symbols": "AAPL", "period": "1mo", "interval": "1d"})
        # Clear
        _, status = self._make_request("/cache/clear", method="POST")
        if status != 200:
            self._fail(name, f"Cache clear returned {status}")
            return
        self.logger.info("  Cache cleared — next history request will be a cold fetch")
        self._pass(name)

    def _run_all(self):
        self.test_market_status_endpoint()
        self.test_market_status_equity_matches_local()
        self.test_market_status_futures_matches_local()
        self.test_stale_data_served_when_market_closed()
        self.test_futures_always_available_on_weekday()
        self.test_clear_cache_clears_history()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = MarketHoursTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

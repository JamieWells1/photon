# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Full integration test runner — imports every test suite and runs them all.
Used by the systemd service for scheduled runs.

To run a single suite directly:
  python test_routing.py [--local]
  python test_currency.py [--local]
  python test_single.py [--local]
  python test_batch.py [--local]
  python test_cache.py [--local]
  python test_edge_cases.py [--local]
  python test_health.py [--local]
"""

import logging
import sys
import time
from datetime import datetime

import schedule

from base import BASE_URL, TestResult, make_logger
from test_health import HealthTests
from test_routing import RoutingTests
from test_currency import CurrencyTests
from test_single import SingleStockTests
from test_batch import BatchTests
from test_cache import CacheTests
from test_edge_cases import EdgeCaseTests

logger = make_logger("IntegrationTests")

# Suites run one at a time on rotation, every 4 hours.
# BatchTests runs before SingleStockTests so its cold-cache performance
# test lands before anything else has warmed the cache.
SUITES = [
    BatchTests,
    SingleStockTests,
    RoutingTests,
    CurrencyTests,
    CacheTests,
    EdgeCaseTests,
]

_suite_index = 0


def run_next(base_url: str = BASE_URL) -> bool:
    global _suite_index
    Suite = SUITES[_suite_index % len(SUITES)]
    _suite_index += 1

    logger.info("=" * 60)
    logger.info(f"Scheduled run: {Suite.__name__} ({_suite_index}/{len(SUITES)} in rotation)")
    logger.info(f"Time: {datetime.now().isoformat()}")
    logger.info("=" * 60)

    combined = TestResult()
    combined.start_time = datetime.now()
    combined.merge(HealthTests(base_url).run())
    combined.merge(Suite(base_url).run())
    combined.end_time = datetime.now()

    if combined.failed:
        logger.warning(f"⚠️  {Suite.__name__} run had failures — manual investigation required")
    return combined.failed == 0


def run_all(base_url: str = BASE_URL) -> bool:
    """Run every suite in sequence. Used with --once."""
    logger.info("=" * 60)
    logger.info(f"Full run (all suites) against {base_url}")
    logger.info(f"Time: {datetime.now().isoformat()}")
    logger.info("=" * 60)

    combined = TestResult()
    combined.start_time = datetime.now()
    for Suite in SUITES:
        logger.info(f"\n--- {Suite.__name__} ---")
        combined.merge(Suite(base_url).run())
    combined.end_time = datetime.now()
    logger.info("\n" + combined.summary())
    return combined.failed == 0


def main():
    import argparse

    parser = argparse.ArgumentParser(description="Delta Proxy Integration Tests")
    parser.add_argument("--once", action="store_true", help="Run all suites once and exit")
    parser.add_argument("--local", action="store_true", help="Test against localhost:8080")
    parser.add_argument(
        "--interval", type=int, default=4,
        help="Hours between each suite in the rotation (default: 4)"
    )
    args = parser.parse_args()

    url = "http://localhost:8080" if args.local else BASE_URL

    if args.once:
        sys.exit(0 if run_all(url) else 1)

    logger.info(f"Rotating scheduler — one suite every {args.interval}h against {url}")
    logger.info(f"Full cycle completes every {args.interval * len(SUITES)}h")
    run_next(url)
    schedule.every(args.interval).hours.do(run_next, url)

    try:
        while True:
            schedule.run_pending()
            time.sleep(60)
    except KeyboardInterrupt:
        logger.info("Stopping")
        sys.exit(0)


if __name__ == "__main__":
    main()

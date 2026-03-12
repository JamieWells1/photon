# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""
Integration tests for Photon Proxy Stock API
Runs automatically every 12 hours to verify proxy functionality
"""

import requests
import time
import logging
from datetime import datetime
from typing import Dict, Any, Optional
import schedule
import sys

PROXY_HOST = "138.68.190.163"
PROXY_PORT = 8080
BASE_URL = f"http://{PROXY_HOST}:{PROXY_PORT}"

USE_LOCAL = False
if USE_LOCAL:
    BASE_URL = "http://localhost:8080"

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    handlers=[
        logging.FileHandler("integration_tests.log"),
        logging.StreamHandler(sys.stdout),
    ],
)
logger = logging.getLogger(__name__)


class ProxyTestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
        self.start_time = None
        self.end_time = None

    def record_pass(self, test_name: str):
        self.passed += 1
        logger.info(f"✓ {test_name} PASSED")

    def record_fail(self, test_name: str, reason: str):
        self.failed += 1
        error_msg = f"✗ {test_name} FAILED: {reason}"
        self.errors.append(error_msg)
        logger.error(error_msg)

    def summary(self) -> str:
        duration = (
            (self.end_time - self.start_time).total_seconds()
            if self.start_time and self.end_time
            else 0
        )
        total = self.passed + self.failed

        summary = f"""
{'='*60}
Integration Test Summary
{'='*60}
Total Tests: {total}
Passed: {self.passed}
Failed: {self.failed}
Duration: {duration:.2f}s
Success Rate: {(self.passed/total*100) if total > 0 else 0:.1f}%
{'='*60}
"""
        if self.errors:
            summary += "\nErrors:\n"
            for error in self.errors:
                summary += f"  {error}\n"

        return summary


class ProxyIntegrationTests:
    def __init__(self, base_url: str):
        self.base_url = base_url
        self.result = ProxyTestResult()

    def _make_request(
        self, endpoint: str, params: Optional[Dict] = None
    ) -> Optional[Dict]:
        """Make HTTP request to proxy"""
        url = f"{self.base_url}{endpoint}"
        try:
            response = requests.get(url, params=params, timeout=10)

            # Try to parse JSON even if status code indicates error
            # This allows us to get error messages from the server
            try:
                data = response.json()
            except:
                data = None

            # If we got a 4xx or 5xx error but no JSON, treat as error
            if response.status_code >= 400:
                if data is None:
                    # No JSON body, create error dict
                    return {"error": f"HTTP {response.status_code}"}
                # Has JSON body (likely with error field)
                return data

            # Success response
            response.raise_for_status()
            return data

        except requests.exceptions.RequestException as e:
            logger.error(f"Request failed for {endpoint}: {e}")
            return {"error": str(e)}

    def _validate_stock_response(self, data: Dict, symbol: str) -> bool:
        """Validate stock response structure"""
        if not data:
            return False

        if "result" not in data:
            return False

        if symbol not in data["result"]:
            return False

        stock_data = data["result"][symbol]
        if "c" not in stock_data or "o" not in stock_data:
            return False

        if not isinstance(stock_data["c"], list) or len(stock_data["c"]) == 0:
            return False

        try:
            current = float(stock_data["c"][0])
            prev_close = float(stock_data["o"])
            if current <= 0 or prev_close <= 0:
                return False
        except (ValueError, TypeError):
            return False

        return True

    def test_basic_stock_fetch(self):
        """Test 1: Basic stock fetch without currency parameter"""
        test_name = "Basic Stock Fetch (AAPL)"
        data = self._make_request("/stocks/AAPL")

        if self._validate_stock_response(data, "AAPL"):
            price = float(data["result"]["AAPL"]["c"][0])
            logger.info(f"  AAPL price: ${price:.2f}")
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Invalid response structure or price")

    def test_usd_to_gbp_conversion(self):
        """Test 2: US stock with GBP conversion"""
        test_name = "USD to GBP Conversion (AAPL)"
        data = self._make_request("/stocks/AAPL", {"currency": "GBP"})

        if self._validate_stock_response(data, "AAPL"):
            price_gbp = float(data["result"]["AAPL"]["c"][0])

            if 50 < price_gbp < 300:
                logger.info(f"  AAPL price in GBP: £{price_gbp:.2f}")
                self.result.record_pass(test_name)
            else:
                self.result.record_fail(
                    test_name, f"Unrealistic GBP price: £{price_gbp:.2f}"
                )
        else:
            self.result.record_fail(test_name, "Invalid response")

    def test_lse_ticker_translation(self):
        """Test 3: LSE ticker auto-translation (VUSA -> VUSA.L)"""
        test_name = "LSE Ticker Translation (VUSA)"
        data = self._make_request("/stocks/VUSA")

        if self._validate_stock_response(data, "VUSA"):
            price = float(data["result"]["VUSA"]["c"][0])
            if 50 < price < 200:
                logger.info(f"  VUSA price: £{price:.2f}")
                self.result.record_pass(test_name)
            else:
                self.result.record_fail(test_name, f"Unrealistic price: {price:.2f}")
        else:
            self.result.record_fail(test_name, "Invalid response")

    def test_lse_with_currency_param(self):
        """Test 4: LSE stock with explicit GBP parameter"""
        test_name = "LSE Stock with GBP (VUSA)"
        data = self._make_request("/stocks/VUSA", {"currency": "GBP"})

        if self._validate_stock_response(data, "VUSA"):
            price = float(data["result"]["VUSA"]["c"][0])
            logger.info(f"  VUSA price (explicit GBP): £{price:.2f}")
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Invalid response")

    def test_lse_to_usd_conversion(self):
        """Test 5: LSE stock converted to USD"""
        test_name = "GBP to USD Conversion (VUSA)"
        data = self._make_request("/stocks/VUSA", {"currency": "USD"})

        if self._validate_stock_response(data, "VUSA"):
            price_usd = float(data["result"]["VUSA"]["c"][0])
            if 60 < price_usd < 300:
                logger.info(f"  VUSA price in USD: ${price_usd:.2f}")
                self.result.record_pass(test_name)
            else:
                self.result.record_fail(
                    test_name, f"Unrealistic USD price: ${price_usd:.2f}"
                )
        else:
            self.result.record_fail(test_name, "Invalid response")

    def test_multiple_stocks_caching(self):
        """Test 6: Multiple stocks with same currency (tests FX caching)"""
        test_name = "FX Rate Caching (MSFT, GOOGL)"

        stocks = ["MSFT", "GOOGL"]
        all_passed = True

        for symbol in stocks:
            data = self._make_request("/stocks/" + symbol, {"currency": "GBP"})
            if not self._validate_stock_response(data, symbol):
                all_passed = False
                break
            price = float(data["result"][symbol]["c"][0])
            logger.info(f"  {symbol} price: £{price:.2f}")

        if all_passed:
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Failed to fetch one or more stocks")

    def test_invalid_symbol(self):
        """Test 7: Invalid symbol should return error"""
        test_name = "Invalid Symbol Error Handling"
        data = self._make_request("/stocks/INVALID123")

        if data and "error" in data:
            logger.info(f"  Correctly returned error: {data['error']}")
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(
                test_name, "Did not return error for invalid symbol"
            )

    def test_currency_conversion_accuracy(self):
        """Test 8: Verify currency conversion is approximately correct"""
        test_name = "Currency Conversion Accuracy"

        data_usd = self._make_request("/stocks/AAPL")
        data_gbp = self._make_request("/stocks/AAPL", {"currency": "GBP"})

        if self._validate_stock_response(
            data_usd, "AAPL"
        ) and self._validate_stock_response(data_gbp, "AAPL"):

            price_usd = float(data_usd["result"]["AAPL"]["c"][0])
            price_gbp = float(data_gbp["result"]["AAPL"]["c"][0])

            implied_rate = price_gbp / price_usd

            if 0.6 < implied_rate < 0.9:
                logger.info(f"  AAPL USD: ${price_usd:.2f}, GBP: £{price_gbp:.2f}")
                logger.info(f"  Implied FX rate: {implied_rate:.4f}")
                self.result.record_pass(test_name)
            else:
                self.result.record_fail(
                    test_name, f"Unrealistic FX rate: {implied_rate:.4f}"
                )
        else:
            self.result.record_fail(test_name, "Failed to fetch prices")

    def test_prev_close_included(self):
        """Test 9: Verify prev_close is included in response"""
        test_name = "Previous Close Included"
        data = self._make_request("/stocks/AAPL")

        if self._validate_stock_response(data, "AAPL"):
            current = float(data["result"]["AAPL"]["c"][0])
            prev_close = float(data["result"]["AAPL"]["o"])

            pct_change = abs((current - prev_close) / prev_close * 100)

            logger.info(f"  Current: ${current:.2f}, Prev Close: ${prev_close:.2f}")
            logger.info(f"  Change: {pct_change:.2f}%")

            if pct_change < 20:
                self.result.record_pass(test_name)
            else:
                self.result.record_fail(
                    test_name, f"Unusually large change: {pct_change:.2f}%"
                )
        else:
            self.result.record_fail(test_name, "Invalid response")

    def test_health_check(self):
        """Test 10: Health check endpoint"""
        test_name = "Health Check Endpoint"
        data = self._make_request("/health")

        if data and data.get("status") == "healthy":
            logger.info(f"  Server is healthy")
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Health check failed")

    def test_batch_basic(self):
        """Test 11: Basic batch request with multiple US stocks"""
        test_name = "Batch Endpoint - Basic (AAPL,MSFT,GOOGL)"
        data = self._make_request("/stocks/batch", {"symbols": "AAPL,MSFT,GOOGL"})

        if not data or "result" not in data:
            self.result.record_fail(test_name, "Invalid response structure")
            return

        symbols = ["AAPL", "MSFT", "GOOGL"]
        all_valid = True

        for symbol in symbols:
            if not self._validate_stock_response(data, symbol):
                all_valid = False
                break
            price = float(data["result"][symbol]["c"][0])
            logger.info(f"  {symbol}: ${price:.2f}")

        if all_valid:
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "One or more symbols invalid")

    def test_batch_with_currency(self):
        """Test 12: Batch request with currency conversion"""
        test_name = "Batch Endpoint - Currency Conversion (GBP)"
        data = self._make_request(
            "/stocks/batch", {"symbols": "AAPL,MSFT", "currency": "GBP"}
        )

        if not data or "result" not in data:
            self.result.record_fail(test_name, "Invalid response structure")
            return

        symbols = ["AAPL", "MSFT"]
        all_valid = True

        for symbol in symbols:
            if not self._validate_stock_response(data, symbol):
                all_valid = False
                break
            price = float(data["result"][symbol]["c"][0])
            logger.info(f"  {symbol}: £{price:.2f}")

        if all_valid:
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Currency conversion failed")

    def test_batch_lse_tickers(self):
        """Test 13: Batch request with LSE tickers"""
        test_name = "Batch Endpoint - LSE Tickers (VUSA,VWRL,VUAG)"
        data = self._make_request("/stocks/batch", {"symbols": "VUSA,VWRL,VUAG"})

        if not data or "result" not in data:
            self.result.record_fail(test_name, "Invalid response structure")
            return

        symbols = ["VUSA", "VWRL", "VUAG"]
        all_valid = True

        for symbol in symbols:
            if not self._validate_stock_response(data, symbol):
                all_valid = False
                break
            price = float(data["result"][symbol]["c"][0])
            logger.info(f"  {symbol}: £{price:.2f}")

        if all_valid:
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "One or more LSE tickers invalid")

    def test_batch_mixed_markets(self):
        """Test 14: Batch request with mixed US and LSE stocks"""
        test_name = "Batch Endpoint - Mixed Markets"
        data = self._make_request(
            "/stocks/batch", {"symbols": "AAPL,VUSA,MSFT", "currency": "GBP"}
        )

        if not data or "result" not in data:
            self.result.record_fail(test_name, "Invalid response structure")
            return

        symbols = ["AAPL", "VUSA", "MSFT"]
        all_valid = True

        for symbol in symbols:
            if not self._validate_stock_response(data, symbol):
                all_valid = False
                break
            price = float(data["result"][symbol]["c"][0])
            logger.info(f"  {symbol}: £{price:.2f}")

        if all_valid:
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Failed to handle mixed markets")

    def test_batch_single_ticker(self):
        """Test 15: Batch request with just one ticker"""
        test_name = "Batch Endpoint - Single Ticker (AAPL)"
        data = self._make_request("/stocks/batch", {"symbols": "AAPL"})

        if not data or "result" not in data:
            self.result.record_fail(test_name, "Invalid response structure")
            return

        if self._validate_stock_response(data, "AAPL"):
            price = float(data["result"]["AAPL"]["c"][0])
            logger.info(f"  AAPL: ${price:.2f}")
            self.result.record_pass(test_name)
        else:
            self.result.record_fail(test_name, "Invalid response for single ticker")

    def run_all_tests(self):
        """Run all integration tests"""
        logger.info("=" * 60)
        logger.info(f"Starting Integration Tests against {self.base_url}")
        logger.info(f"Time: {datetime.now().isoformat()}")
        logger.info("=" * 60)

        self.result.start_time = datetime.now()

        # Health and single stock tests
        self.test_health_check()
        self.test_basic_stock_fetch()
        self.test_usd_to_gbp_conversion()
        self.test_lse_ticker_translation()
        self.test_lse_with_currency_param()
        self.test_lse_to_usd_conversion()
        self.test_multiple_stocks_caching()
        self.test_currency_conversion_accuracy()
        self.test_prev_close_included()
        self.test_invalid_symbol()

        # Batch endpoint tests
        self.test_batch_basic()
        self.test_batch_with_currency()
        self.test_batch_lse_tickers()
        self.test_batch_mixed_markets()
        self.test_batch_single_ticker()

        self.result.end_time = datetime.now()

        summary = self.result.summary()
        logger.info(summary)

        return self.result.failed == 0


def run_scheduled_tests():
    """Run tests on schedule"""
    logger.info("\n" + "=" * 60)
    logger.info("SCHEDULED TEST RUN")
    logger.info("=" * 60 + "\n")

    tests = ProxyIntegrationTests(BASE_URL)
    success = tests.run_all_tests()

    if not success:
        logger.warning("⚠️  TESTS FAILED - Manual investigation required")
    else:
        logger.info("✓ All tests passed successfully")

    return success


def main():
    """Main entry point"""
    import argparse

    parser = argparse.ArgumentParser(description="Photon Proxy Integration Tests")
    parser.add_argument("--once", action="store_true", help="Run tests once and exit")
    parser.add_argument(
        "--local", action="store_true", help="Test against localhost:8080"
    )
    parser.add_argument(
        "--interval", type=int, default=12, help="Test interval in hours (default: 12)"
    )

    args = parser.parse_args()

    global BASE_URL
    if args.local:
        BASE_URL = "http://localhost:8080"

    if args.once:
        tests = ProxyIntegrationTests(BASE_URL)
        success = tests.run_all_tests()
        sys.exit(0 if success else 1)
    else:
        logger.info(f"Starting scheduled test runner (every {args.interval} hours)")
        logger.info(f"Testing against: {BASE_URL}")
        logger.info("Press Ctrl+C to stop\n")

        run_scheduled_tests()

        schedule.every(args.interval).hours.do(run_scheduled_tests)

        try:
            while True:
                schedule.run_pending()
                time.sleep(60)
        except KeyboardInterrupt:
            logger.info("\nStopping scheduled tests")
            sys.exit(0)


if __name__ == "__main__":
    main()

# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

"""Shared infrastructure for Delta Proxy integration tests."""

import requests
import logging
import sys
from datetime import datetime
from typing import Dict, Optional

PROXY_HOST = "138.68.190.163"
PROXY_PORT = 8080
BASE_URL = f"http://{PROXY_HOST}:{PROXY_PORT}"


def make_logger(name: str) -> logging.Logger:
    logger = logging.getLogger(name)
    if not logger.handlers:
        logger.setLevel(logging.INFO)
        fmt = logging.Formatter("%(asctime)s - %(levelname)s - %(message)s")
        ch = logging.StreamHandler(sys.stdout)
        ch.setFormatter(fmt)
        logger.addHandler(ch)
        fh = logging.FileHandler("integration_tests.log")
        fh.setFormatter(fmt)
        logger.addHandler(fh)
    return logger


class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
        self.start_time = None
        self.end_time = None

    def record_pass(self, name: str, logger: logging.Logger):
        self.passed += 1
        logger.info(f"✓ {name} PASSED")

    def record_fail(self, name: str, reason: str, logger: logging.Logger):
        self.failed += 1
        msg = f"✗ {name} FAILED: {reason}"
        self.errors.append(msg)
        logger.error(msg)

    def summary(self) -> str:
        duration = (
            (self.end_time - self.start_time).total_seconds()
            if self.start_time and self.end_time
            else 0
        )
        total = self.passed + self.failed
        lines = [
            "=" * 60,
            "Test Summary",
            "=" * 60,
            f"Total:   {total}",
            f"Passed:  {self.passed}",
            f"Failed:  {self.failed}",
            f"Time:    {duration:.2f}s",
            f"Rate:    {(self.passed / total * 100) if total else 0:.1f}%",
            "=" * 60,
        ]
        if self.errors:
            lines.append("\nFailures:")
            lines.extend(f"  {e}" for e in self.errors)
        return "\n".join(lines)

    def merge(self, other: "TestResult"):
        self.passed += other.passed
        self.failed += other.failed
        self.errors.extend(other.errors)


class ProxyTestBase:
    """Base class for all proxy test suites."""

    def __init__(self, base_url: str = BASE_URL):
        self.base_url = base_url
        self.result = TestResult()
        self.logger = make_logger(self.__class__.__name__)

    # ------------------------------------------------------------------
    # HTTP helpers
    # ------------------------------------------------------------------

    def _make_request(
        self,
        endpoint: str,
        params: Optional[Dict] = None,
        method: str = "GET",
        timeout: int = 60,
    ) -> tuple[Optional[Dict], int]:
        """Return (parsed_json_or_None, http_status_code)."""
        url = f"{self.base_url}{endpoint}"
        try:
            fn = requests.post if method == "POST" else requests.get
            response = fn(url, params=params, timeout=timeout)
            try:
                data = response.json()
            except Exception:
                data = None
            return data, response.status_code
        except requests.exceptions.RequestException as e:
            self.logger.error(f"Request failed for {endpoint}: {e}")
            return {"error": str(e)}, 0

    def _get(
        self,
        endpoint: str,
        params: Optional[Dict] = None,
        timeout: int = 60,
    ) -> Optional[Dict]:
        data, _ = self._make_request(endpoint, params=params, timeout=timeout)
        return data

    # ------------------------------------------------------------------
    # Response validation helpers
    # ------------------------------------------------------------------

    def _valid(self, data: Optional[Dict], symbol: str) -> bool:
        """True when the response contains a well-formed price entry for symbol."""
        if not data or "result" not in data:
            return False
        entry = data["result"].get(symbol)
        if not entry or "c" not in entry or "o" not in entry:
            return False
        if not isinstance(entry["c"], list) or not entry["c"]:
            return False
        try:
            c = float(entry["c"][0])
            o = float(entry["o"])
            return c > 0 and o > 0
        except (ValueError, TypeError):
            return False

    def _price(self, data: Optional[Dict], symbol: str) -> Optional[float]:
        if not self._valid(data, symbol):
            return None
        try:
            return float(data["result"][symbol]["c"][0])
        except (ValueError, TypeError, KeyError):
            return None

    def _prev_close(self, data: Optional[Dict], symbol: str) -> Optional[float]:
        if not self._valid(data, symbol):
            return None
        try:
            return float(data["result"][symbol]["o"])
        except (ValueError, TypeError, KeyError):
            return None

    # ------------------------------------------------------------------
    # Pass / fail shorthands
    # ------------------------------------------------------------------

    def _pass(self, name: str):
        self.result.record_pass(name, self.logger)

    def _fail(self, name: str, reason: str):
        self.result.record_fail(name, reason, self.logger)

    # ------------------------------------------------------------------
    # Suite runner (subclasses override)
    # ------------------------------------------------------------------

    def run(self) -> TestResult:
        self.result.start_time = datetime.now()
        self._run_all()
        self.result.end_time = datetime.now()
        self.logger.info(self.result.summary())
        return self.result

    def _run_all(self):
        raise NotImplementedError

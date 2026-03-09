# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3

from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.request
import json
import logging
import time
from collections import defaultdict
from datetime import datetime
import sys
import traceback

try:
    from const import *
except ImportError:
    print("Warning: const.py not found, using defaults")
    PROXY_PORT = 8080
    BIND_ADDRESS = "0.0.0.0"
    REQUEST_TIMEOUT = 30
    RATE_LIMIT_ENABLED = True
    RATE_LIMIT_REQUESTS = 100
    RATE_LIMIT_WINDOW = 60
    ALLOWED_HOSTS = ["api.kraken.com", "api.open-meteo.com"]
    LOG_LEVEL = "INFO"
    LOG_FILE = None
    LOG_TO_CONSOLE = True
    HEALTH_CHECK_ENABLED = True
    HEALTH_CHECK_PATH = "/health"
    REQUIRE_PROXY_PREFIX = True
    BLOCK_SUSPICIOUS_REQUESTS = True


def setup_logging():
    logger = logging.getLogger("PhotonProxy")
    logger.setLevel(getattr(logging, LOG_LEVEL))

    formatter = logging.Formatter(
        "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
    )

    if LOG_TO_CONSOLE:
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)

    if LOG_FILE:
        try:
            file_handler = logging.FileHandler(LOG_FILE)
            file_handler.setFormatter(formatter)
            logger.addHandler(file_handler)
        except Exception as e:
            print(f"Warning: Could not create log file {LOG_FILE}: {e}")

    return logger


logger = setup_logging()


class RateLimiter:
    def __init__(self, max_requests, window_seconds):
        self.max_requests = max_requests
        self.window = window_seconds
        self.requests = defaultdict(list)

    def is_allowed(self, client_ip):
        if not RATE_LIMIT_ENABLED:
            return True

        now = time.time()
        self.requests[client_ip] = [
            t for t in self.requests[client_ip] if now - t < self.window
        ]

        if len(self.requests[client_ip]) >= self.max_requests:
            return False

        self.requests[client_ip].append(now)
        return True


rate_limiter = RateLimiter(RATE_LIMIT_REQUESTS, RATE_LIMIT_WINDOW)


class ProxyHandler(BaseHTTPRequestHandler):
    _fx_rate_cache = {}
    _fx_cache_timestamp = 0
    _fx_cache_ttl = 3600  # Cache for 1 hour

    def log_message(self, format, *args):
        logger.info(f"{self.address_string()} - {format % args}")

    def send_json_response(self, code, data):
        response = json.dumps(data).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", len(response))
        self.end_headers()
        self.wfile.write(response)

    def validate_request(self, path):
        if BLOCK_SUSPICIOUS_REQUESTS:
            suspicious_patterns = ["..", "//", "\\", "<", ">", "javascript:", "script"]
            if any(pattern in path.lower() for pattern in suspicious_patterns):
                logger.warning(f"Blocked suspicious request: {path}")
                return False, "Suspicious request pattern detected"

        if REQUIRE_PROXY_PREFIX and not (path.startswith("/proxy/") or path.startswith("/stocks/")):
            return False, "Path must start with /proxy/ or /stocks/"

        return True, None

    def extract_target_url(self, path):
        target_path = path[7:]
        host = target_path.split("/")[0]

        if host not in ALLOWED_HOSTS:
            return None, f"Host '{host}' not in allowed list"

        return f"https://{target_path}", None

    def get_fx_rate(self, from_ccy, to_ccy):
        """Get exchange rate from one currency to another using yfinance with caching"""
        import yfinance as yf
        import time

        if from_ccy == to_ccy:
            return 1.0

        cache_key = f"{from_ccy}{to_ccy}"
        current_time = time.time()

        if current_time - self._fx_cache_timestamp > self._fx_cache_ttl:
            logger.info("FX rate cache expired, clearing")
            self._fx_rate_cache.clear()
            self._fx_cache_timestamp = current_time

        if cache_key in self._fx_rate_cache:
            rate = self._fx_rate_cache[cache_key]
            logger.debug(f"FX rate {cache_key}: {rate} (cached)")
            return rate

        pair = f"{from_ccy}{to_ccy}=X"
        try:
            ticker = yf.Ticker(pair)
            rate = ticker.fast_info.last_price
            if rate and rate > 0:
                logger.info(f"FX rate {pair}: {rate} (fetched)")
                self._fx_rate_cache[cache_key] = rate
                return rate
        except Exception as e:
            logger.debug(f"Direct FX pair {pair} failed: {e}")

        pair = f"{to_ccy}{from_ccy}=X"
        try:
            ticker = yf.Ticker(pair)
            rate = ticker.fast_info.last_price
            if rate and rate > 0:
                inverted = 1.0 / rate
                logger.info(f"FX rate {pair} (inverted): {inverted} (fetched)")
                self._fx_rate_cache[cache_key] = inverted
                return inverted
        except Exception as e:
            logger.debug(f"Inverse FX pair {pair} failed: {e}")

        logger.warning(f"Could not get FX rate for {from_ccy} to {to_ccy}")
        return None

    def convert_price(self, price, from_ccy, to_ccy):
        """Convert price from one currency to another"""
        if not price or price <= 0:
            return price

        if from_ccy == to_ccy:
            return price

        rate = self.get_fx_rate(from_ccy, to_ccy)
        if rate is None:
            logger.error(f"Cannot convert {from_ccy} to {to_ccy}, returning unconverted price")
            return price

        converted = price * rate
        logger.info(f"Converted {price:.4f} {from_ccy} -> {converted:.4f} {to_ccy} (rate: {rate:.6f})")
        return converted

    def handle_stock_request(self, client_ip):
        import yfinance as yf
        import re
        from urllib.parse import parse_qs, urlparse

        # LSE tickers that need .L suffix for Yahoo Finance
        LSE_TICKERS = {
            # Vanguard S&P 500 UCITS
            "VUAG", "VUSA",
            # Vanguard Global/Regional
            "VWRL", "VWRP", "VNRG", "VEUA", "VFEG", "VJPB", "VDPG",
            # Vanguard Bond & Dividend
            "VAGS", "VHYL", "VEVE",
            # iShares Core (popular in Europe)
            "IWDA", "SWDA", "CSPX", "EQQQ", "EMIM",
            # Sector ETFs
            "IITU",
            # UK/FTSE ETFs
            "VUKE", "ISF", "IUKD",
        }

        parsed = urlparse(self.path)
        symbol = parsed.path[8:].upper()
        query_params = parse_qs(parsed.query)
        target_currency = query_params.get('currency', [None])[0]

        if target_currency:
            target_currency = target_currency.upper()
            logger.debug(f"Target currency requested: {target_currency}")

        if not symbol or not re.match(r'^[A-Z0-9]+(\.[A-Z]+)?$', symbol):
            self.send_json_response(400, {"error": "Invalid symbol"})
            return

        original_symbol = symbol
        if symbol in LSE_TICKERS and not symbol.endswith('.L'):
            symbol = f"{symbol}.L"
            logger.info(f"Translated LSE ticker: {original_symbol} -> {symbol}")

        try:
            ticker = yf.Ticker(symbol)
            fast_info = ticker.fast_info
            current = fast_info.last_price
            prev_close = fast_info.previous_close

            if current is None or prev_close is None:
                self.send_json_response(404, {"error": f"No data for symbol '{symbol}'"})
                return

            # Get the stock's native currency
            stock_currency = ticker.info.get('currency', 'USD')
            logger.info(f"Stock {symbol} native currency: {stock_currency}")

            if target_currency and target_currency != stock_currency:
                logger.info(f"Converting from {stock_currency} to {target_currency}")
                current = self.convert_price(current, stock_currency, target_currency)
                prev_close = self.convert_price(prev_close, stock_currency, target_currency)

                if current is None or prev_close is None:
                    self.send_json_response(500, {
                        "error": f"Currency conversion failed from {stock_currency} to {target_currency}"
                    })
                    return

            # Matches Kraken's format so the Pico-side parser works unchanged:
            #   "c": ["price"] -> ticker_parse_price_from_array
            #   "o": "price"   -> ticker_parse_price_from_string
            # Use original symbol in response (without .L) so device code works unchanged
            result = {
                "result": {
                    original_symbol: {
                        "c": [str(current)],
                        "o": str(prev_close),
                    }
                }
            }

            logger.info(f"Stock {symbol} for {client_ip}: current={current}, prev_close={prev_close}")
            self.send_json_response(200, result)

        except Exception as e:
            logger.error(f"yfinance error for {symbol} ({client_ip}): {e}")
            self.send_json_response(500, {"error": f"Failed to fetch data for '{symbol}'"})

    def do_GET(self):
        client_ip = self.client_address[0]

        try:
            if HEALTH_CHECK_ENABLED and self.path == HEALTH_CHECK_PATH:
                self.send_json_response(200, {
                    "status": "healthy",
                    "timestamp": datetime.utcnow().isoformat(),
                    "version": "1.0.0",
                })
                return

            if not rate_limiter.is_allowed(client_ip):
                logger.warning(f"Rate limit exceeded for {client_ip}")
                self.send_json_response(429, {
                    "error": "Rate limit exceeded",
                    "retry_after": RATE_LIMIT_WINDOW,
                })
                return

            valid, error = self.validate_request(self.path)
            if not valid:
                logger.warning(f"Invalid request from {client_ip}: {error}")
                self.send_json_response(400, {"error": error})
                return

            if self.path.startswith("/stocks/"):
                self.handle_stock_request(client_ip)
                return

            https_url, error = self.extract_target_url(self.path)
            if not https_url:
                logger.warning(f"Invalid target URL from {client_ip}: {error}")
                self.send_json_response(400, {"error": error})
                return

            logger.info(f"Proxying: {self.path} -> {https_url}")

            req = urllib.request.Request(https_url)
            req.add_header("User-Agent", "PhotonProxy/1.0")

            with urllib.request.urlopen(req, timeout=REQUEST_TIMEOUT) as response:
                data = response.read()

                self.send_response(200)
                self.send_header("Content-Type", response.headers.get("Content-Type", "application/json"))
                self.send_header("Content-Length", len(data))
                self.end_headers()
                self.wfile.write(data)

                logger.info(f"Proxied {len(data)} bytes to {client_ip}")

        except urllib.error.HTTPError as e:
            logger.error(f"HTTP error for {client_ip}: {e.code} {e.reason}")
            self.send_json_response(e.code, {"error": f"Upstream error: {e.reason}", "code": e.code})

        except urllib.error.URLError as e:
            logger.error(f"URL error for {client_ip}: {e.reason}")
            self.send_json_response(502, {"error": "Bad Gateway", "details": str(e.reason)})

        except Exception as e:
            logger.error(f"Unexpected error for {client_ip}: {e}")
            logger.debug(traceback.format_exc())
            self.send_json_response(500, {"error": "Internal server error"})


class ProxyServer(HTTPServer):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.start_time = time.time()
        self.request_count = 0

    def finish_request(self, request, client_address):
        self.request_count += 1
        super().finish_request(request, client_address)


def run_proxy(port=PROXY_PORT, bind_address=BIND_ADDRESS):
    try:
        server = ProxyServer((bind_address, port), ProxyHandler)

        logger.info(f"Photon HTTPS Proxy listening on {bind_address}:{port}")
        logger.info(f"Allowed hosts: {', '.join(ALLOWED_HOSTS)}")
        logger.info(f"Rate limit: {RATE_LIMIT_REQUESTS} req/{RATE_LIMIT_WINDOW}s per IP")

        server.serve_forever()

    except KeyboardInterrupt:
        uptime = time.time() - server.start_time
        logger.info(f"Shutting down. Uptime: {uptime:.1f}s, requests: {server.request_count}")
        server.shutdown()

    except Exception as e:
        logger.error(f"Failed to start server: {e}")
        logger.debug(traceback.format_exc())
        sys.exit(1)


if __name__ == "__main__":
    run_proxy()

#!/usr/bin/env python3
# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

"""
FastAPI-based HTTPS proxy for Photon project
High-performance async version with better concurrency
"""

from fastapi import FastAPI, Request, HTTPException, Query
from fastapi.responses import JSONResponse, Response
import httpx
import asyncio
import logging
import sys
import time
from collections import defaultdict
from datetime import datetime
from typing import Optional
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

# FastAPI app
app = FastAPI(
    title="Photon HTTPS Proxy",
    description="High-performance async proxy for Photon devices",
    version="2.0.0",
)


class AsyncRateLimiter:
    """Thread-safe async rate limiter"""

    def __init__(self, max_requests: int, window_seconds: int):
        self.max_requests = max_requests
        self.window = window_seconds
        self.requests = defaultdict(list)
        self._lock = asyncio.Lock()

    async def is_allowed(self, client_ip: str) -> bool:
        if not RATE_LIMIT_ENABLED:
            return True

        async with self._lock:
            now = time.time()
            self.requests[client_ip] = [
                t for t in self.requests[client_ip] if now - t < self.window
            ]

            if len(self.requests[client_ip]) >= self.max_requests:
                return False

            self.requests[client_ip].append(now)
            return True


rate_limiter = AsyncRateLimiter(RATE_LIMIT_REQUESTS, RATE_LIMIT_WINDOW)


class FXRateCache:
    """Async-safe FX rate cache with TTL"""

    def __init__(self, ttl: int = 3600):
        self.cache = {}
        self.timestamp = 0
        self.ttl = ttl
        self._lock = asyncio.Lock()

    async def get(self, key: str) -> Optional[float]:
        async with self._lock:
            if time.time() - self.timestamp > self.ttl:
                logger.info("FX rate cache expired, clearing")
                self.cache.clear()
                self.timestamp = time.time()

            return self.cache.get(key)

    async def set(self, key: str, value: float):
        async with self._lock:
            self.cache[key] = value


fx_cache = FXRateCache()


# In-flight request tracking to prevent duplicate yfinance calls
in_flight_requests = {}
in_flight_lock = asyncio.Lock()


async def get_client_ip(request: Request) -> str:
    """Extract client IP from request"""
    if forwarded := request.headers.get("X-Forwarded-For"):
        return forwarded.split(",")[0].strip()
    if real_ip := request.headers.get("X-Real-IP"):
        return real_ip
    return request.client.host if request.client else "unknown"


async def check_rate_limit(request: Request):
    """Middleware to check rate limits"""
    client_ip = await get_client_ip(request)

    if not await rate_limiter.is_allowed(client_ip):
        logger.warning(f"Rate limit exceeded for {client_ip}")
        raise HTTPException(
            status_code=429,
            detail={"error": "Rate limit exceeded", "retry_after": RATE_LIMIT_WINDOW},
        )


def validate_path(path: str) -> None:
    """Validate request path for security"""
    if BLOCK_SUSPICIOUS_REQUESTS:
        suspicious_patterns = ["..", "//", "\\", "<", ">", "javascript:", "script"]
        if any(pattern in path.lower() for pattern in suspicious_patterns):
            logger.warning(f"Blocked suspicious request: {path}")
            raise HTTPException(
                status_code=400, detail="Suspicious request pattern detected"
            )

    if REQUIRE_PROXY_PREFIX and not (
        path.startswith("/proxy/") or path.startswith("/stocks/")
    ):
        raise HTTPException(
            status_code=400, detail="Path must start with /proxy/ or /stocks/"
        )


async def get_fx_rate(from_ccy: str, to_ccy: str) -> Optional[float]:
    """Get exchange rate using yfinance with caching and deduplication"""
    if from_ccy == to_ccy:
        return 1.0

    cache_key = f"{from_ccy}{to_ccy}"

    # Check cache first
    if cached := await fx_cache.get(cache_key):
        logger.debug(f"FX rate {cache_key}: {cached} (cached)")
        return cached

    # Check if another request is already fetching this rate
    async with in_flight_lock:
        if cache_key in in_flight_requests:
            logger.debug(f"Waiting for in-flight FX request: {cache_key}")
            future = in_flight_requests[cache_key]
        else:
            future = asyncio.Future()
            in_flight_requests[cache_key] = future

    # If we created the future, we're responsible for fetching
    if not future.done():
        try:
            # Run yfinance in thread pool (it's synchronous)
            rate = await asyncio.to_thread(_fetch_fx_rate_sync, from_ccy, to_ccy)
            if rate:
                await fx_cache.set(cache_key, rate)
            future.set_result(rate)
        except Exception as e:
            future.set_exception(e)
        finally:
            async with in_flight_lock:
                in_flight_requests.pop(cache_key, None)

    return await future


def _fetch_fx_rate_sync(from_ccy: str, to_ccy: str) -> Optional[float]:
    """Synchronous FX rate fetch (runs in thread pool)"""
    import yfinance as yf

    # Try direct pair
    pair = f"{from_ccy}{to_ccy}=X"
    try:
        ticker = yf.Ticker(pair)
        rate = ticker.fast_info.last_price
        if rate and rate > 0:
            logger.info(f"FX rate {pair}: {rate} (fetched)")
            return rate
    except Exception as e:
        logger.debug(f"Direct FX pair {pair} failed: {e}")

    # Try inverse pair
    pair = f"{to_ccy}{from_ccy}=X"
    try:
        ticker = yf.Ticker(pair)
        rate = ticker.fast_info.last_price
        if rate and rate > 0:
            inverted = 1.0 / rate
            logger.info(f"FX rate {pair} (inverted): {inverted} (fetched)")
            return inverted
    except Exception as e:
        logger.debug(f"Inverse FX pair {pair} failed: {e}")

    logger.warning(f"Could not get FX rate for {from_ccy} to {to_ccy}")
    return None


async def convert_price(price: float, from_ccy: str, to_ccy: str) -> Optional[float]:
    """Convert price from one currency to another"""
    if not price or price <= 0:
        return price

    if from_ccy == to_ccy:
        return price

    rate = await get_fx_rate(from_ccy, to_ccy)
    if rate is None:
        logger.error(
            f"Cannot convert {from_ccy} to {to_ccy}, returning unconverted price"
        )
        return price

    converted = price * rate
    logger.info(
        f"Converted {price:.4f} {from_ccy} -> {converted:.4f} {to_ccy} (rate: {rate:.6f})"
    )
    return converted


async def fetch_stock_data(symbol: str, target_currency: Optional[str], client_ip: str):
    """Fetch stock data with deduplication"""

    # LSE tickers that need .L suffix
    LSE_TICKERS = {
        "VUAG",
        "VUSA",
        "VWRL",
        "VWRP",
        "VNRG",
        "VEUA",
        "VFEG",
        "VJPB",
        "VDPG",
        "VAGS",
        "VHYL",
        "VEVE",
        "IWDA",
        "SWDA",
        "CSPX",
        "EQQQ",
        "EMIM",
        "IITU",
        "VUKE",
        "ISF",
        "IUKD",
    }

    original_symbol = symbol.upper()
    yf_symbol = original_symbol

    if original_symbol in LSE_TICKERS and not original_symbol.endswith(".L"):
        yf_symbol = f"{original_symbol}.L"
        logger.info(f"Translated LSE ticker: {original_symbol} -> {yf_symbol}")

    # Create cache key including currency conversion
    cache_key = f"{yf_symbol}:{target_currency or 'native'}"

    # Check if request is in-flight
    async with in_flight_lock:
        if cache_key in in_flight_requests:
            logger.info(f"Deduplicating request for {cache_key}")
            future = in_flight_requests[cache_key]
        else:
            future = asyncio.Future()
            in_flight_requests[cache_key] = future

    # If we created the future, fetch the data
    if not future.done():
        try:
            result = await asyncio.to_thread(
                _fetch_stock_data_sync,
                yf_symbol,
                original_symbol,
                target_currency,
                client_ip,
            )
            future.set_result(result)
        except Exception as e:
            future.set_exception(e)
        finally:
            async with in_flight_lock:
                in_flight_requests.pop(cache_key, None)

    return await future


def _fetch_stock_data_sync(
    yf_symbol: str, original_symbol: str, target_currency: Optional[str], client_ip: str
):
    """Synchronous stock data fetch (runs in thread pool)"""
    import yfinance as yf

    try:
        ticker = yf.Ticker(yf_symbol)
        fast_info = ticker.fast_info
        current = fast_info.last_price
        prev_close = fast_info.previous_close

        if current is None or prev_close is None:
            raise ValueError(f"No data for symbol '{yf_symbol}'")

        # Get native currency
        stock_currency = ticker.info.get("currency", "USD")
        logger.info(f"Stock {yf_symbol} native currency: {stock_currency}")

        return {
            "current": current,
            "prev_close": prev_close,
            "currency": stock_currency,
            "original_symbol": original_symbol,
            "yf_symbol": yf_symbol,
        }

    except Exception as e:
        logger.error(f"yfinance error for {yf_symbol} ({client_ip}): {e}")
        raise


# Routes


@app.get("/health")
async def health_check():
    """Health check endpoint"""
    if not HEALTH_CHECK_ENABLED:
        raise HTTPException(status_code=404, detail="Health check disabled")

    return {
        "status": "healthy",
        "timestamp": datetime.utcnow().isoformat(),
        "version": "2.0.0-fastapi",
    }


@app.get("/stocks/batch")
async def get_stocks_batch(
    symbols: str = Query(
        ..., description="Comma-separated ticker symbols (e.g., AAPL,GOOGL,MSFT)"
    ),
    currency: Optional[str] = Query(
        None, description="Target currency for all symbols (e.g., USD, GBP)"
    ),
    request: Request = None,
):
    """
    Get multiple stock prices in one request (batch endpoint)

    Example: /stocks/batch?symbols=AAPL,GOOGL,MSFT&currency=USD

    Returns all tickers in Kraken-compatible format
    Makes only 1 yfinance API call regardless of number of tickers
    """
    await check_rate_limit(request)
    client_ip = await get_client_ip(request)

    # Parse symbols
    symbol_list = [s.strip().upper() for s in symbols.split(",") if s.strip()]

    if not symbol_list:
        raise HTTPException(status_code=400, detail="No symbols provided")

    if len(symbol_list) > 100:
        raise HTTPException(status_code=400, detail="Maximum 100 symbols per request")

    target_currency = currency.upper() if currency else None

    try:
        # Batch fetch using yfinance (runs in thread pool)
        batch_data = await asyncio.to_thread(
            _fetch_batch_stock_data_sync, symbol_list, client_ip
        )

        # Build response in Kraken format
        result = {"result": {}}

        for symbol_data in batch_data:
            if "error" in symbol_data:
                # Include failed tickers with error
                result["result"][symbol_data["symbol"]] = {
                    "error": symbol_data["error"]
                }
                continue

            original_symbol = symbol_data["original_symbol"]
            current = symbol_data["current"]
            prev_close = symbol_data["prev_close"]
            stock_currency = symbol_data["currency"]

            # Convert currency if requested
            if target_currency and target_currency != stock_currency:
                current = await convert_price(current, stock_currency, target_currency)
                prev_close = await convert_price(
                    prev_close, stock_currency, target_currency
                )

                if current is None or prev_close is None:
                    result["result"][original_symbol] = {
                        "error": f"Currency conversion failed from {stock_currency} to {target_currency}"
                    }
                    continue

            # Add to result
            result["result"][original_symbol] = {
                "c": [str(current)],
                "o": str(prev_close),
            }

        logger.info(f"Batch request for {len(symbol_list)} symbols from {client_ip}")
        return result

    except Exception as e:
        logger.error(f"Batch request error from {client_ip}: {e}")
        logger.debug(traceback.format_exc())
        raise HTTPException(status_code=500, detail=f"Batch request failed: {str(e)}")


@app.get("/stocks/{symbol}")
async def get_stock(
    symbol: str,
    request: Request,
    currency: Optional[str] = Query(
        None, description="Target currency (e.g., USD, GBP)"
    ),
):
    """Get stock price data"""
    await check_rate_limit(request)
    client_ip = await get_client_ip(request)

    # Validate symbol
    if not symbol or not symbol.replace(".", "").isalnum():
        raise HTTPException(status_code=400, detail="Invalid symbol")

    target_currency = currency.upper() if currency else None

    try:
        # Fetch stock data (with deduplication)
        data = await fetch_stock_data(symbol, target_currency, client_ip)

        current = data["current"]
        prev_close = data["prev_close"]
        stock_currency = data["currency"]
        original_symbol = data["original_symbol"]

        # Convert currency if requested
        if target_currency and target_currency != stock_currency:
            logger.info(f"Converting from {stock_currency} to {target_currency}")
            current = await convert_price(current, stock_currency, target_currency)
            prev_close = await convert_price(
                prev_close, stock_currency, target_currency
            )

            if current is None or prev_close is None:
                raise HTTPException(
                    status_code=500,
                    detail=f"Currency conversion failed from {stock_currency} to {target_currency}",
                )

        # Return in Kraken-compatible format
        result = {
            "result": {
                original_symbol: {
                    "c": [str(current)],
                    "o": str(prev_close),
                }
            }
        }

        logger.info(
            f"Stock {symbol} for {client_ip}: current={current}, prev_close={prev_close}"
        )
        return result

    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        logger.error(f"Error fetching {symbol}: {e}")
        logger.debug(traceback.format_exc())
        raise HTTPException(
            status_code=500, detail=f"Failed to fetch data for '{symbol}'"
        )


def _fetch_batch_stock_data_sync(symbol_list: list, client_ip: str):
    """
    Synchronous batch fetch using yfinance.download()
    Makes only 1 API call for all symbols
    """
    import yfinance as yf
    import pandas as pd

    # LSE tickers mapping
    LSE_TICKERS = {
        "VUAG",
        "VUSA",
        "VWRL",
        "VWRP",
        "VNRG",
        "VEUA",
        "VFEG",
        "VJPB",
        "VDPG",
        "VAGS",
        "VHYL",
        "VEVE",
        "IWDA",
        "SWDA",
        "CSPX",
        "EQQQ",
        "EMIM",
        "IITU",
        "VUKE",
        "ISF",
        "IUKD",
    }

    # Map symbols to yfinance format (add .L for LSE)
    symbol_map = {}  # original -> yfinance
    for sym in symbol_list:
        if sym in LSE_TICKERS and not sym.endswith(".L"):
            symbol_map[sym] = f"{sym}.L"
            logger.info(f"Translated LSE ticker: {sym} -> {sym}.L")
        else:
            symbol_map[sym] = sym

    yf_symbols = list(symbol_map.values())

    try:
        # Batch download - SINGLE API CALL
        logger.info(
            f"Batch fetching {len(yf_symbols)} symbols: {', '.join(yf_symbols)}"
        )

        data = yf.download(
            tickers=" ".join(yf_symbols),
            period="2d",  # Get 2 days to ensure we have prev_close
            interval="1d",
            progress=False,
            threads=True,
        )

        results = []

        # Handle single vs multiple tickers (yfinance returns different formats)
        if len(yf_symbols) == 1:
            # Single ticker - data is a simple DataFrame
            yf_symbol = yf_symbols[0]
            original_symbol = [k for k, v in symbol_map.items() if v == yf_symbol][0]

            if data.empty or "Close" not in data.columns:
                results.append(
                    {"symbol": original_symbol, "error": "No data available"}
                )
            else:
                try:
                    # Handle both Series and DataFrame cases
                    close_data = data["Close"]
                    if isinstance(close_data, pd.DataFrame):
                        close_list = close_data.iloc[:, 0].tolist()
                    else:
                        close_list = close_data.tolist()

                    current = float(close_list[-1])
                    prev_close = (
                        float(close_list[-2]) if len(close_list) > 1 else current
                    )

                    # Get currency from ticker info
                    ticker_obj = yf.Ticker(yf_symbol)
                    stock_currency = ticker_obj.info.get("currency", "USD")

                    results.append(
                        {
                            "original_symbol": original_symbol,
                            "yf_symbol": yf_symbol,
                            "current": current,
                            "prev_close": prev_close,
                            "currency": stock_currency,
                        }
                    )
                except Exception as e:
                    results.append({"symbol": original_symbol, "error": str(e)})
        else:
            # Multiple tickers - data has MultiIndex columns
            for yf_symbol in yf_symbols:
                original_symbol = [k for k, v in symbol_map.items() if v == yf_symbol][
                    0
                ]

                try:
                    # Extract close prices for this ticker
                    if isinstance(data.columns, pd.MultiIndex):
                        close_data = data["Close"][yf_symbol]
                    else:
                        # Fallback for single column
                        close_data = data["Close"]

                    if close_data.empty or close_data.isna().all():
                        results.append(
                            {"symbol": original_symbol, "error": "No data available"}
                        )
                        continue

                    # Get current and previous close
                    close_values = close_data.dropna()
                    if len(close_values) == 0:
                        results.append(
                            {"symbol": original_symbol, "error": "No valid price data"}
                        )
                        continue

                    # Convert to list to ensure Python scalars
                    close_list = close_values.tolist()
                    current = float(close_list[-1])
                    prev_close = (
                        float(close_list[-2])
                        if len(close_list) > 1
                        else current
                    )

                    # Get currency from ticker info
                    ticker_obj = yf.Ticker(yf_symbol)
                    stock_currency = ticker_obj.info.get("currency", "USD")

                    results.append(
                        {
                            "original_symbol": original_symbol,
                            "yf_symbol": yf_symbol,
                            "current": current,
                            "prev_close": prev_close,
                            "currency": stock_currency,
                        }
                    )

                except Exception as e:
                    logger.error(f"Error processing {yf_symbol}: {e}")
                    results.append({"symbol": original_symbol, "error": str(e)})

        logger.info(f"Batch fetch complete: {len(results)} results from 1 API call")
        return results

    except Exception as e:
        logger.error(f"Batch download failed: {e}")
        # Return errors for all symbols
        return [{"symbol": sym, "error": str(e)} for sym in symbol_list]


@app.get("/proxy/{full_path:path}")
async def proxy_request(full_path: str, request: Request):
    """Proxy HTTPS requests to allowed hosts"""
    await check_rate_limit(request)
    client_ip = await get_client_ip(request)

    path = f"/proxy/{full_path}"
    validate_path(path)

    # Extract target URL
    target_path = full_path
    host = target_path.split("/")[0]

    if host not in ALLOWED_HOSTS:
        raise HTTPException(
            status_code=400, detail=f"Host '{host}' not in allowed list"
        )

    # Build URL with query parameters from original request
    https_url = f"https://{target_path}"
    if request.url.query:
        https_url = f"{https_url}?{request.url.query}"

    logger.info(f"Proxying: {path} -> {https_url}")

    try:
        async with httpx.AsyncClient(timeout=REQUEST_TIMEOUT) as client:
            response = await client.get(
                https_url, headers={"User-Agent": "PhotonProxy/2.0"}
            )
            response.raise_for_status()

            logger.info(f"Proxied {len(response.content)} bytes to {client_ip}")

            # Return raw response without parsing/re-serializing to preserve format
            return Response(
                content=response.content,
                status_code=response.status_code,
                headers=dict(response.headers),
            )

    except httpx.HTTPStatusError as e:
        logger.error(f"HTTP error for {client_ip}: {e.response.status_code}")
        raise HTTPException(
            status_code=e.response.status_code,
            detail={"error": "Upstream error", "code": e.response.status_code},
        )
    except httpx.RequestError as e:
        logger.error(f"Request error for {client_ip}: {e}")
        raise HTTPException(status_code=502, detail="Bad Gateway")
    except Exception as e:
        logger.error(f"Unexpected error for {client_ip}: {e}")
        logger.debug(traceback.format_exc())
        raise HTTPException(status_code=500, detail="Internal server error")


@app.exception_handler(HTTPException)
async def http_exception_handler(request: Request, exc: HTTPException):
    """Custom exception handler for consistent error responses"""
    return JSONResponse(
        status_code=exc.status_code,
        content={"error": exc.detail} if isinstance(exc.detail, str) else exc.detail,
    )


if __name__ == "__main__":
    import argparse
    import uvicorn

    parser = argparse.ArgumentParser(description="Photon HTTPS Proxy (FastAPI)")
    parser.add_argument(
        "--port",
        type=int,
        default=PROXY_PORT,
        help=f"Port to listen on (default: {PROXY_PORT})",
    )
    parser.add_argument(
        "--bind", default=BIND_ADDRESS, help=f"Bind address (default: {BIND_ADDRESS})"
    )
    parser.add_argument(
        "--workers", type=int, default=1, help="Number of worker processes (default: 1)"
    )
    args = parser.parse_args()

    logger.info(f"Starting Photon FastAPI Proxy on {args.bind}:{args.port}")
    logger.info(f"Allowed hosts: {', '.join(ALLOWED_HOSTS)}")
    logger.info(f"Rate limit: {RATE_LIMIT_REQUESTS} req/{RATE_LIMIT_WINDOW}s per IP")

    uvicorn.run(
        "https_proxy_fastapi:app",
        host=args.bind,
        port=args.port,
        workers=args.workers,
        log_level=LOG_LEVEL.lower(),
        access_log=True,
    )

# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

import asyncio
import time
import traceback
from datetime import datetime
from typing import Optional

import httpx
from fastapi import APIRouter, HTTPException, Query, Request
from fastapi.responses import JSONResponse, Response

from api.config import (
    ALLOWED_HOSTS,
    BLOCK_SUSPICIOUS_REQUESTS,
    HEALTH_CHECK_ENABLED,
    RATE_LIMIT_WINDOW,
    REQUEST_TIMEOUT,
    REQUIRE_PROXY_PREFIX,
    logger,
)
from api.cache import fx_cache, rate_limiter, stock_cache
from api.fx import convert_price
from api.stocks import _fetch_batch_stock_data_sync, fetch_stock_data

router = APIRouter()


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


@router.post("/cache/clear")
async def clear_cache(request: Request):
    """Clear all stock price and FX rate caches"""
    await check_rate_limit(request)
    async with stock_cache._lock:
        stock_cache.cache.clear()
    async with fx_cache._lock:
        fx_cache.cache.clear()
        fx_cache.timestamp = 0
    logger.info("Cache cleared manually")
    return {"status": "cleared"}


@router.get("/health")
async def health_check():
    """Health check endpoint"""
    if not HEALTH_CHECK_ENABLED:
        raise HTTPException(status_code=404, detail="Health check disabled")

    return {
        "status": "healthy",
        "timestamp": datetime.utcnow().isoformat(),
        "version": "2.0.0-fastapi",
    }


@router.get("/stocks/batch")
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
    t_req_start = time.time()

    symbol_list = [s.strip().upper() for s in symbols.split(",") if s.strip()]

    if not symbol_list:
        raise HTTPException(status_code=400, detail="No symbols provided")

    if len(symbol_list) > 100:
        raise HTTPException(status_code=400, detail="Maximum 100 symbols per request")

    target_currency = currency.upper() if currency else None

    try:
        cache_hits, misses = await stock_cache.get_many(symbol_list)
        logger.info(
            f"Batch request from {client_ip}: {len(symbol_list)} symbols, "
            f"{len(cache_hits)} cached, {len(misses)} fetching from yfinance "
            f"(target_currency={target_currency})"
        )

        fetched_data = []
        if misses:
            fetched_data = await asyncio.to_thread(
                _fetch_batch_stock_data_sync, misses, client_ip
            )
            for symbol_data in fetched_data:
                if "error" not in symbol_data:
                    await stock_cache.set(
                        symbol_data["original_symbol"],
                        symbol_data["current"],
                        symbol_data["prev_close"],
                        symbol_data["currency"],
                    )

        result = {"result": {}}

        for original_symbol, entry in cache_hits.items():
            current = entry["current"]
            prev_close = entry["prev_close"]
            stock_currency = entry["stock_currency"]

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

            result["result"][original_symbol] = {
                "c": [str(current)],
                "o": str(prev_close),
            }

        for symbol_data in fetched_data:
            if "error" in symbol_data:
                result["result"][symbol_data["symbol"]] = {
                    "error": symbol_data["error"]
                }
                continue

            original_symbol = symbol_data["original_symbol"]
            current = symbol_data["current"]
            prev_close = symbol_data["prev_close"]
            stock_currency = symbol_data["currency"]

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

            result["result"][original_symbol] = {
                "c": [str(current)],
                "o": str(prev_close),
            }

        elapsed = time.time() - t_req_start
        logger.info(
            f"Batch request complete for {client_ip}: "
            f"{len(result['result'])} symbols returned in {elapsed:.2f}s"
        )
        return result

    except Exception as e:
        logger.error(f"Batch request error from {client_ip}: {e}")
        logger.debug(traceback.format_exc())
        raise HTTPException(status_code=500, detail=f"Batch request failed: {str(e)}")


@router.get("/stocks/{symbol}")
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

    if not symbol or not symbol.replace(".", "").isalnum():
        raise HTTPException(status_code=400, detail="Invalid symbol")

    target_currency = currency.upper() if currency else None

    try:
        cached = await stock_cache.get(symbol.upper())
        if cached:
            logger.info(f"Stock {symbol} for {client_ip}: serving from cache")
            current = cached["current"]
            prev_close = cached["prev_close"]
            stock_currency = cached["stock_currency"]
            original_symbol = symbol.upper()
        else:
            data = await fetch_stock_data(symbol, target_currency, client_ip)
            current = data["current"]
            prev_close = data["prev_close"]
            stock_currency = data["currency"]
            original_symbol = data["original_symbol"]
            await stock_cache.set(original_symbol, current, prev_close, stock_currency)

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


@router.get("/proxy/{full_path:path}")
async def proxy_request(full_path: str, request: Request):
    """Proxy HTTPS requests to allowed hosts"""
    await check_rate_limit(request)
    client_ip = await get_client_ip(request)

    path = f"/proxy/{full_path}"
    validate_path(path)

    target_path = full_path
    host = target_path.split("/")[0]

    if host not in ALLOWED_HOSTS:
        raise HTTPException(
            status_code=400, detail=f"Host '{host}' not in allowed list"
        )

    https_url = f"https://{target_path}"
    if request.url.query:
        https_url = f"{https_url}?{request.url.query}"

    logger.info(f"Proxying: {path} -> {https_url}")

    try:
        async with httpx.AsyncClient(timeout=REQUEST_TIMEOUT) as client:
            response = await client.get(
                https_url, headers={"User-Agent": "DeltaProxy/2.0"}
            )
            response.raise_for_status()

            logger.info(f"Proxied {len(response.content)} bytes to {client_ip}")

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

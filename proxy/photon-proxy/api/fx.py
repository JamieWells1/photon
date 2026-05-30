# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

import asyncio
from typing import Optional

from api.config import logger
from api.cache import fx_cache, in_flight_requests, in_flight_lock


async def get_fx_rate(from_ccy: str, to_ccy: str) -> Optional[float]:
    """Get exchange rate using yfinance with caching and deduplication"""
    if from_ccy == to_ccy:
        return 1.0

    cache_key = f"{from_ccy}{to_ccy}"

    if cached := await fx_cache.get(cache_key):
        logger.debug(f"FX rate {cache_key}: {cached} (cached)")
        return cached

    is_owner = False
    async with in_flight_lock:
        if cache_key in in_flight_requests:
            logger.debug(f"Waiting for in-flight FX request: {cache_key}")
            future = in_flight_requests[cache_key]
        else:
            future = asyncio.Future()
            in_flight_requests[cache_key] = future
            is_owner = True

    if is_owner:
        try:
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

    pair = f"{from_ccy}{to_ccy}=X"
    try:
        ticker = yf.Ticker(pair)
        rate = ticker.fast_info.last_price
        if rate and rate > 0:
            logger.info(f"FX rate {pair}: {rate} (fetched)")
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
            return inverted
    except Exception as e:
        logger.debug(f"Inverse FX pair {pair} failed: {e}")

    logger.warning(f"Could not get FX rate for {from_ccy} to {to_ccy}")
    return None


async def convert_price(price: float, from_ccy: str, to_ccy: str) -> Optional[float]:
    """Convert price from one currency to another"""
    if not price or price <= 0:
        return price

    # GBX / GBp = pence — divide by 100 to get GBP first
    if from_ccy in ("GBX", "GBp"):
        price = price / 100
        from_ccy = "GBP"
        logger.debug(f"GBX→GBP conversion: divided by 100 → {price:.4f}")

    if from_ccy == to_ccy:
        return price

    rate = await get_fx_rate(from_ccy, to_ccy)
    if rate is None:
        logger.error(
            f"Cannot convert {from_ccy} to {to_ccy}, returning unconverted price"
        )
        return price

    converted = price * rate
    logger.debug(
        f"Converted {price:.4f} {from_ccy} -> {converted:.4f} {to_ccy} (rate: {rate:.6f})"
    )
    return converted

# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

import asyncio
import time
from typing import Optional

from api.config import logger
from api.cache import stock_cache, in_flight_requests, in_flight_lock
from api.tickers import (
    TICKER_ALIASES,
    LSE_TICKERS,
    EURONEXT_TICKERS,
    _get_exchange_ticker_currency,
)

_INTRADAY_INTERVALS = {"1m", "2m", "5m", "15m", "30m", "60m", "90m", "1h"}


async def fetch_stock_data(symbol: str, target_currency: Optional[str], client_ip: str):
    """Fetch stock data with deduplication"""
    original_symbol = symbol.upper()
    yf_base = TICKER_ALIASES.get(original_symbol, original_symbol)
    yf_symbol = yf_base

    if yf_base in LSE_TICKERS and not yf_base.endswith(".L"):
        yf_symbol = f"{yf_base}.L"
        logger.info(f"Translated LSE ticker: {original_symbol} -> {yf_symbol}")
    elif yf_base in EURONEXT_TICKERS and not yf_base.endswith(".AS"):
        yf_symbol = f"{yf_base}.AS"
        logger.info(f"Translated Euronext ticker: {original_symbol} -> {yf_symbol}")

    cache_key = f"{yf_symbol}:{target_currency or 'native'}"

    is_owner = False
    async with in_flight_lock:
        if cache_key in in_flight_requests:
            logger.info(f"Deduplicating request for {cache_key}")
            future = in_flight_requests[cache_key]
        else:
            future = asyncio.Future()
            in_flight_requests[cache_key] = future
            is_owner = True

    if is_owner:
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
        try:
            current = ticker.fast_info.last_price
            prev_close = ticker.fast_info.previous_close
        except Exception:
            current = None
            prev_close = None

        def try_suffixes(suffixes):
            """Try a list of exchange suffixes, return (ticker, symbol, current, prev_close) or raise."""
            import pandas as pd
            for suffix in suffixes:
                candidate = f"{yf_symbol}{suffix}"
                logger.info(f"No data for {yf_symbol}, retrying as {candidate}")
                t = yf.Ticker(candidate)
                try:
                    cur = t.fast_info.last_price
                    prv = t.fast_info.previous_close
                except Exception:
                    cur = prv = None
                if cur is not None and prv is not None:
                    return t, candidate, cur, prv
                # fast_info unreliable for some exchange ETFs — fall back to download()
                dl = yf.download(candidate, period="5d", interval="1d", progress=False, threads=False)
                if dl.empty:
                    continue
                try:
                    close_series = (
                        dl["Close"][candidate] if isinstance(dl.columns, pd.MultiIndex) else dl["Close"]
                    )
                except KeyError:
                    continue
                close_values = close_series.dropna()
                if close_values.empty:
                    continue
                cur = float(close_values.iloc[-1])
                prv = float(close_values.iloc[-2]) if len(close_values) >= 2 else cur
                return t, candidate, cur, prv
            raise ValueError(f"No data for symbol '{original_symbol}'")

        if current is None or prev_close is None:
            has_suffix = any(yf_symbol.endswith(s) for s in (".L", ".AS", ".DE", ".PA", ".MI"))
            if not has_suffix:
                ticker, yf_symbol, current, prev_close = try_suffixes([".L", ".AS", ".DE", ".PA"])
            else:
                raise ValueError(f"No data for symbol '{yf_symbol}'")

        if any(yf_symbol.endswith(s) for s in (".L", ".AS", ".DE", ".PA", ".MI")):
            stock_currency = _get_exchange_ticker_currency(yf_symbol)
        else:
            stock_currency = "USD"
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


def _fetch_batch_stock_data_sync(symbol_list: list, client_ip: str):
    """
    Synchronous batch fetch using yfinance.download()
    Makes only 1 API call for all symbols
    """
    import yfinance as yf
    import pandas as pd

    symbol_map = {}
    for sym in symbol_list:
        yf_base = TICKER_ALIASES.get(sym, sym)
        if yf_base in LSE_TICKERS and not yf_base.endswith(".L"):
            symbol_map[sym] = f"{yf_base}.L"
            logger.info(f"Translated LSE ticker: {sym} -> {symbol_map[sym]}")
        elif yf_base in EURONEXT_TICKERS and not yf_base.endswith(".AS"):
            symbol_map[sym] = f"{yf_base}.AS"
            logger.info(f"Translated Euronext ticker: {sym} -> {symbol_map[sym]}")
        else:
            symbol_map[sym] = yf_base

    yf_symbols = list(symbol_map.values())

    # Currency inference: exchange-listed tickers use the module-level cache
    # (populated once per process, never expires). US tickers infer USD directly.
    def infer_currency(yf_sym: str) -> str:
        # Any exchange-listed ticker needs a real currency lookup — .L can be
        # GBX or GBP, .AS is EUR, .DE/.PA/.MI are EUR, etc.
        for suffix in (".L", ".AS", ".DE", ".PA", ".MI"):
            if yf_sym.endswith(suffix):
                return _get_exchange_ticker_currency(yf_sym)
        return "USD"

    t_batch_start = time.time()
    try:
        # Batch download - SINGLE API CALL
        # 1m interval over 2 days: gives prices within ~1 minute of real-time
        # and lets us split on the date boundary to get a true prev_close.
        logger.info(
            f"Batch fetching {len(yf_symbols)} symbols: {', '.join(yf_symbols)}"
        )

        data = yf.download(
            tickers=" ".join(yf_symbols),
            period="2d",
            interval="1m",
            progress=False,
            threads=True,
        )

        results = []

        def extract_current_and_prev_close(close_series):
            """
            Given a minute-resolution Close series spanning up to 2 trading days,
            return (current_price, prev_close).

            current  = last available price (most recent minute bar)
            prev_close = last price of the previous calendar day's session.
                         Falls back to the first bar if only one day is present.
            """
            close_values = close_series.dropna()
            if close_values.empty:
                return None, None

            current = float(close_values.iloc[-1])

            dates = close_values.index.normalize()
            unique_dates = dates.unique()

            if len(unique_dates) >= 2:
                prev_day = unique_dates[-2]
                prev_day_data = close_values[dates == prev_day]
                prev_close = float(prev_day_data.iloc[-1])
            else:
                # Only one trading session available (e.g. weekend/holiday boundary) —
                # return current as prev_close so delta displays as 0 rather than wrong.
                prev_close = current

            return current, prev_close

        # Handle single vs multiple tickers (yfinance returns different formats)
        if len(yf_symbols) == 1:
            yf_symbol = yf_symbols[0]
            original_symbol = [k for k, v in symbol_map.items() if v == yf_symbol][0]

            if data.empty or "Close" not in data.columns:
                results.append(
                    {"symbol": original_symbol, "error": "No data available"}
                )
            else:
                try:
                    close_data = data["Close"]
                    if isinstance(close_data, pd.DataFrame):
                        close_series = close_data.iloc[:, 0]
                    else:
                        close_series = close_data

                    current, prev_close = extract_current_and_prev_close(close_series)
                    if current is None:
                        raise ValueError("No valid price data")

                    stock_currency = infer_currency(yf_symbol)
                    logger.info(
                        f"{original_symbol}: current={current:.4f}, "
                        f"prev_close={prev_close:.4f}, currency={stock_currency}"
                    )

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
            for yf_symbol in yf_symbols:
                original_symbol = [k for k, v in symbol_map.items() if v == yf_symbol][0]

                try:
                    if isinstance(data.columns, pd.MultiIndex):
                        close_data = data["Close"][yf_symbol]
                    else:
                        close_data = data["Close"]

                    if close_data.empty or close_data.isna().all():
                        results.append(
                            {"symbol": original_symbol, "error": "No data available"}
                        )
                        continue

                    current, prev_close = extract_current_and_prev_close(close_data)
                    if current is None:
                        results.append(
                            {"symbol": original_symbol, "error": "No valid price data"}
                        )
                        continue

                    stock_currency = infer_currency(yf_symbol)
                    logger.info(
                        f"{original_symbol}: current={current:.4f}, "
                        f"prev_close={prev_close:.4f}, currency={stock_currency}"
                    )

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

        # Retry symbols that returned no data with .L suffix
        retry_map = {
            r["symbol"]: f"{symbol_map.get(r['symbol'], r['symbol'])}.L"
            for r in results
            if "error" in r
            and not symbol_map.get(r["symbol"], r["symbol"]).endswith(".L")
        }

        if retry_map:
            retry_yf_symbols = list(retry_map.values())
            logger.info(
                f"Retrying {len(retry_yf_symbols)} failed symbols with .L suffix: "
                f"{', '.join(retry_yf_symbols)}"
            )
            try:
                retry_data = yf.download(
                    tickers=" ".join(retry_yf_symbols),
                    period="2d",
                    interval="1m",
                    progress=False,
                    threads=True,
                )
                retry_results = {}
                if len(retry_yf_symbols) == 1:
                    orig = next(iter(retry_map))
                    yf_sym = retry_yf_symbols[0]
                    if not retry_data.empty and "Close" in retry_data.columns:
                        close_data = retry_data["Close"]
                        if isinstance(close_data, pd.DataFrame):
                            close_data = close_data.iloc[:, 0]
                        current, prev_close = extract_current_and_prev_close(close_data)
                        if current is not None:
                            retry_results[orig] = {
                                "original_symbol": orig,
                                "yf_symbol": yf_sym,
                                "current": current,
                                "prev_close": prev_close,
                                "currency": infer_currency(yf_sym),
                            }
                            logger.info(
                                f"{orig} (.L retry): current={current:.4f}, prev_close={prev_close:.4f}"
                            )
                else:
                    for orig, yf_sym in retry_map.items():
                        try:
                            close_data = (
                                retry_data["Close"][yf_sym]
                                if isinstance(retry_data.columns, pd.MultiIndex)
                                else retry_data["Close"]
                            )
                            if not close_data.empty and not close_data.isna().all():
                                current, prev_close = extract_current_and_prev_close(
                                    close_data
                                )
                                if current is not None:
                                    retry_results[orig] = {
                                        "original_symbol": orig,
                                        "yf_symbol": yf_sym,
                                        "current": current,
                                        "prev_close": prev_close,
                                        "currency": infer_currency(yf_sym),
                                    }
                                    logger.info(
                                        f"{orig} (.L retry): current={current:.4f}, prev_close={prev_close:.4f}"
                                    )
                        except Exception as e:
                            logger.debug(f"Retry processing failed for {yf_sym}: {e}")

                results = [
                    retry_results.get(r.get("symbol"), r) if "error" in r else r
                    for r in results
                ]
            except Exception as e:
                logger.error(f"Retry batch download failed: {e}")

        elapsed = time.time() - t_batch_start
        logger.info(
            f"Batch fetch complete: {len(results)} results from 1 API call "
            f"in {elapsed:.2f}s"
        )
        return results

    except Exception as e:
        logger.error(f"Batch download failed: {e}")
        return [{"symbol": sym, "error": str(e)} for sym in symbol_list]


async def fetch_history(
    symbols: list, period: str, interval: str, client_ip: str
) -> dict:
    """Async wrapper — runs the synchronous yfinance download in a thread pool."""
    return await asyncio.to_thread(
        _fetch_history_sync, symbols, period, interval, client_ip
    )


def _fetch_history_sync(
    symbols: list, period: str, interval: str, client_ip: str
) -> dict:
    """Fetch historical close prices for a list of symbols.

    Returns dict keyed by original symbol:
      { "dates": [...], "closes": [...] }   on success
      { "error": "..." }                    on failure

    Applies the same LSE/Euronext suffix translation and .L retry logic as
    the batch price endpoint.
    """
    import yfinance as yf
    import pandas as pd

    # Translate display tickers to yfinance symbols
    symbol_map: dict[str, str] = {}
    for sym in symbols:
        yf_base = TICKER_ALIASES.get(sym, sym)
        if yf_base in LSE_TICKERS and not yf_base.endswith(".L"):
            symbol_map[sym] = f"{yf_base}.L"
            logger.info(f"History: LSE translation {sym} -> {symbol_map[sym]}")
        elif yf_base in EURONEXT_TICKERS and not yf_base.endswith(".AS"):
            symbol_map[sym] = f"{yf_base}.AS"
            logger.info(f"History: Euronext translation {sym} -> {symbol_map[sym]}")
        else:
            symbol_map[sym] = yf_base

    yf_symbols = list(symbol_map.values())
    rev_map = {v: k for k, v in symbol_map.items()}

    date_fmt = "%Y-%m-%dT%H:%M" if interval in _INTRADAY_INTERVALS else "%Y-%m-%d"

    def extract_series(series, yf_sym: str) -> tuple[list, list] | tuple[None, None]:
        series = series.dropna()
        if series.empty:
            return None, None
        dates = [d.strftime(date_fmt) for d in series.index]
        closes = [round(float(v), 4) for v in series.values]
        # GBX-denominated LSE tickers return prices in pence — convert to pounds.
        if any(yf_sym.endswith(s) for s in (".L", ".AS", ".DE", ".PA", ".MI")):
            if _get_exchange_ticker_currency(yf_sym) == "GBX":
                closes = [round(v / 100, 6) for v in closes]
        return dates, closes

    logger.info(
        f"History fetch: {len(yf_symbols)} symbols period={period} interval={interval} "
        f"from {client_ip}"
    )

    try:
        data = yf.download(
            tickers=" ".join(yf_symbols),
            period=period,
            interval=interval,
            progress=False,
            threads=True,
        )
    except Exception as e:
        logger.error(f"History download failed: {e}")
        return {sym: {"error": str(e)} for sym in symbols}

    results: dict = {}
    failed: list[str] = []

    if len(yf_symbols) == 1:
        yf_sym = yf_symbols[0]
        orig = rev_map[yf_sym]
        if data.empty or "Close" not in data.columns:
            failed.append(orig)
        else:
            close_data = data["Close"]
            if isinstance(close_data, pd.DataFrame):
                close_data = close_data.iloc[:, 0]
            dates, closes = extract_series(close_data, yf_sym)
            if dates:
                results[orig] = {"dates": dates, "closes": closes}
            else:
                failed.append(orig)
    else:
        for yf_sym in yf_symbols:
            orig = rev_map[yf_sym]
            try:
                close_data = (
                    data["Close"][yf_sym]
                    if isinstance(data.columns, pd.MultiIndex)
                    else data["Close"]
                )
                dates, closes = extract_series(close_data, yf_sym)
                if dates:
                    results[orig] = {"dates": dates, "closes": closes}
                else:
                    failed.append(orig)
            except Exception as e:
                logger.warning(f"History: no data for {yf_sym}: {e}")
                failed.append(orig)

    # Retry failed symbols with .L suffix (mirrors batch endpoint pattern)
    retry_map = {
        orig: f"{symbol_map.get(orig, orig)}.L"
        for orig in failed
        if not symbol_map.get(orig, orig).endswith(".L")
    }

    if retry_map:
        retry_yf_symbols = list(retry_map.values())
        logger.info(f"History retry with .L: {', '.join(retry_yf_symbols)}")
        try:
            retry_data = yf.download(
                tickers=" ".join(retry_yf_symbols),
                period=period,
                interval=interval,
                progress=False,
                threads=True,
            )
            if len(retry_yf_symbols) == 1:
                orig = next(iter(retry_map))
                yf_sym = retry_yf_symbols[0]
                if not retry_data.empty and "Close" in retry_data.columns:
                    close_data = retry_data["Close"]
                    if isinstance(close_data, pd.DataFrame):
                        close_data = close_data.iloc[:, 0]
                    dates, closes = extract_series(close_data, yf_sym)
                    if dates:
                        results[orig] = {"dates": dates, "closes": closes}
                        logger.info(f"History retry success: {orig} as {yf_sym}")
            else:
                for orig, yf_sym in retry_map.items():
                    try:
                        close_data = (
                            retry_data["Close"][yf_sym]
                            if isinstance(retry_data.columns, pd.MultiIndex)
                            else retry_data["Close"]
                        )
                        dates, closes = extract_series(close_data, yf_sym)
                        if dates:
                            results[orig] = {"dates": dates, "closes": closes}
                            logger.info(f"History retry success: {orig} as {yf_sym}")
                    except Exception as e:
                        logger.debug(f"History retry failed for {yf_sym}: {e}")
        except Exception as e:
            logger.error(f"History retry batch failed: {e}")

    # Any symbol still without data gets an error entry
    for orig in failed:
        if orig not in results:
            results[orig] = {"error": f"No historical data available for '{orig}'"}

    return results

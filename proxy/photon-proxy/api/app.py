# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse

from api.config import (
    ALLOWED_HOSTS,
    BIND_ADDRESS,
    LOG_LEVEL,
    PROXY_PORT,
    RATE_LIMIT_REQUESTS,
    RATE_LIMIT_WINDOW,
    logger,
)
from api.routes import router

app = FastAPI(
    title="Delta HTTPS Proxy",
    description="High-performance async proxy for Delta devices",
    version="2.0.0",
)

app.include_router(router)


@app.exception_handler(HTTPException)
async def http_exception_handler(request, exc: HTTPException):
    """Custom exception handler for consistent error responses"""
    return JSONResponse(
        status_code=exc.status_code,
        content={"error": exc.detail} if isinstance(exc.detail, str) else exc.detail,
    )


if __name__ == "__main__":
    import argparse

    import uvicorn

    parser = argparse.ArgumentParser(description="Delta HTTPS Proxy (FastAPI)")
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

    logger.info(f"Starting Delta FastAPI Proxy on {args.bind}:{args.port}")
    logger.info(f"Allowed hosts: {', '.join(ALLOWED_HOSTS)}")
    logger.info(f"Rate limit: {RATE_LIMIT_REQUESTS} req/{RATE_LIMIT_WINDOW}s per IP")

    uvicorn.run(
        "https_proxy:app",
        host=args.bind,
        port=args.port,
        workers=args.workers,
        log_level=LOG_LEVEL.lower(),
        access_log=True,
    )

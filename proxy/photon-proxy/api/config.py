# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

import logging
import sys

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
    logger = logging.getLogger("DeltaProxy")
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

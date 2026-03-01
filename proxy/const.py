"""
Configuration constants for HTTPS proxy
"""

# Server configuration
PROXY_PORT = 8080
BIND_ADDRESS = '0.0.0.0'  # Listen on all interfaces

# Request limits
REQUEST_TIMEOUT = 30  # seconds
MAX_REQUEST_SIZE = 10 * 1024 * 1024  # 10MB

# Rate limiting (per IP)
RATE_LIMIT_ENABLED = True
RATE_LIMIT_REQUESTS = 100  # requests
RATE_LIMIT_WINDOW = 60  # seconds

# Allowed target hosts (whitelist for security)
ALLOWED_HOSTS = [
    'api.kraken.com',
    'api.open-meteo.com',
]

# Logging
LOG_LEVEL = 'INFO'  # DEBUG, INFO, WARNING, ERROR
LOG_FILE = '/var/log/photon-proxy.log'  # Set to None to disable file logging
LOG_TO_CONSOLE = True

# Health check
HEALTH_CHECK_ENABLED = True
HEALTH_CHECK_PATH = '/health'

# Security
REQUIRE_PROXY_PREFIX = True  # Require /proxy/ prefix
BLOCK_SUSPICIOUS_REQUESTS = True

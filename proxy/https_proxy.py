#!/usr/bin/env python3
"""
Production-ready HTTP to HTTPS proxy for Pico W
Features: rate limiting, logging, error handling, security
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.request
import urllib.parse
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


# Setup logging
def setup_logging():
    """Configure logging with file and console handlers"""
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


# Rate limiting
class RateLimiter:
    """Simple rate limiter using sliding window"""

    def __init__(self, max_requests, window_seconds):
        self.max_requests = max_requests
        self.window = window_seconds
        self.requests = defaultdict(list)

    def is_allowed(self, client_ip):
        """Check if request from client_ip is allowed"""
        if not RATE_LIMIT_ENABLED:
            return True

        now = time.time()
        # Clean old requests
        self.requests[client_ip] = [
            req_time
            for req_time in self.requests[client_ip]
            if now - req_time < self.window
        ]

        # Check limit
        if len(self.requests[client_ip]) >= self.max_requests:
            return False

        # Add new request
        self.requests[client_ip].append(now)
        return True

    def get_remaining(self, client_ip):
        """Get remaining requests for client"""
        return max(0, self.max_requests - len(self.requests[client_ip]))


rate_limiter = RateLimiter(RATE_LIMIT_REQUESTS, RATE_LIMIT_WINDOW)


class ProxyHandler(BaseHTTPRequestHandler):
    """Production-ready proxy request handler"""

    def log_message(self, format, *args):
        """Override to use our logger"""
        logger.info(f"{self.address_string()} - {format % args}")

    def send_json_response(self, code, data):
        """Send JSON response"""
        response = json.dumps(data).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", len(response))
        self.end_headers()
        self.wfile.write(response)

    def handle_health_check(self):
        """Health check endpoint"""
        self.send_json_response(
            200,
            {
                "status": "healthy",
                "timestamp": datetime.utcnow().isoformat(),
                "version": "1.0.0",
            },
        )

    def validate_request(self, path):
        """Validate request for security"""
        # Check for suspicious patterns
        if BLOCK_SUSPICIOUS_REQUESTS:
            suspicious_patterns = ["..", "//", "\\", "<", ">", "javascript:", "script"]
            if any(pattern in path.lower() for pattern in suspicious_patterns):
                logger.warning(f"Blocked suspicious request: {path}")
                return False, "Suspicious request pattern detected"

        # Check prefix
        if REQUIRE_PROXY_PREFIX and not path.startswith("/proxy/"):
            return False, "Path must start with /proxy/"

        return True, None

    def extract_target_url(self, path):
        """Extract and validate target URL"""
        if not path.startswith("/proxy/"):
            return None, "Invalid path format"

        target_path = path[7:]  # Remove '/proxy/'

        # Extract host
        if "/" in target_path:
            host = target_path.split("/")[0]
        else:
            host = target_path

        # Validate against whitelist
        if host not in ALLOWED_HOSTS:
            return None, f"Host '{host}' not in allowed list"

        return f"https://{target_path}", None

    def do_GET(self):
        """Handle GET requests"""
        client_ip = self.client_address[0]

        try:
            # Health check
            if HEALTH_CHECK_ENABLED and self.path == HEALTH_CHECK_PATH:
                self.handle_health_check()
                return

            # Rate limiting
            if not rate_limiter.is_allowed(client_ip):
                logger.warning(f"Rate limit exceeded for {client_ip}")
                self.send_json_response(
                    429,
                    {"error": "Rate limit exceeded", "retry_after": RATE_LIMIT_WINDOW},
                )
                return

            # Validate request
            valid, error = self.validate_request(self.path)
            if not valid:
                logger.warning(f"Invalid request from {client_ip}: {error}")
                self.send_json_response(400, {"error": error})
                return

            # Extract target URL
            https_url, error = self.extract_target_url(self.path)
            if not https_url:
                logger.warning(f"Invalid target URL from {client_ip}: {error}")
                self.send_json_response(400, {"error": error})
                return

            logger.info(
                f"Proxying request from {client_ip}: {self.path} -> {https_url}"
            )

            # Make HTTPS request
            req = urllib.request.Request(https_url)
            req.add_header("User-Agent", "PhotonProxy/1.0")

            with urllib.request.urlopen(req, timeout=REQUEST_TIMEOUT) as response:
                data = response.read()

                # Send response
                self.send_response(200)
                self.send_header(
                    "Content-Type",
                    response.headers.get("Content-Type", "application/json"),
                )
                self.send_header("Content-Length", len(data))
                self.send_header("X-Proxy-Cache", "MISS")
                self.end_headers()
                self.wfile.write(data)

                logger.info(f"Success: {len(data)} bytes proxied to {client_ip}")

        except urllib.error.HTTPError as e:
            logger.error(f"HTTP error for {client_ip}: {e.code} {e.reason}")
            self.send_json_response(
                e.code, {"error": f"Upstream error: {e.reason}", "code": e.code}
            )

        except urllib.error.URLError as e:
            logger.error(f"URL error for {client_ip}: {e.reason}")
            self.send_json_response(
                502, {"error": "Bad Gateway", "details": str(e.reason)}
            )

        except Exception as e:
            logger.error(f"Unexpected error for {client_ip}: {e}")
            logger.debug(traceback.format_exc())
            self.send_json_response(
                500, {"error": "Internal server error", "message": str(e)}
            )


class ProxyServer(HTTPServer):
    """Custom server with graceful shutdown"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.start_time = time.time()
        self.request_count = 0

    def finish_request(self, request, client_address):
        """Track request count"""
        self.request_count += 1
        super().finish_request(request, client_address)


def run_proxy(port=PROXY_PORT, bind_address=BIND_ADDRESS):
    """Run the proxy server"""
    try:
        server = ProxyServer((bind_address, port), ProxyHandler)

        logger.info("=" * 60)
        logger.info("Photon HTTPS Proxy - Production Mode")
        logger.info("=" * 60)
        logger.info(f"Listening on: {bind_address}:{port}")
        logger.info(f"Allowed hosts: {', '.join(ALLOWED_HOSTS)}")
        logger.info(
            f"Rate limit: {RATE_LIMIT_REQUESTS} req/{RATE_LIMIT_WINDOW}s per IP"
        )
        logger.info(
            f"Health check: {HEALTH_CHECK_PATH if HEALTH_CHECK_ENABLED else 'Disabled'}"
        )
        logger.info(f"Log level: {LOG_LEVEL}")
        logger.info("=" * 60)
        logger.info("Press Ctrl+C to stop")
        logger.info("")

        server.serve_forever()

    except KeyboardInterrupt:
        logger.info("\nShutting down gracefully...")
        uptime = time.time() - server.start_time
        logger.info(f"Uptime: {uptime:.1f}s, Requests served: {server.request_count}")
        server.shutdown()
        logger.info("Server stopped")

    except Exception as e:
        logger.error(f"Failed to start server: {e}")
        logger.debug(traceback.format_exc())
        sys.exit(1)


if __name__ == "__main__":
    run_proxy()

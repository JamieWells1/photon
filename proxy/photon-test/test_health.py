# Copyright (c) 2026 Jamie Wells. All Rights Reserved.
# Proprietary and Confidential - Unauthorized copying or distribution prohibited.

#!/usr/bin/env python3
"""Health endpoint tests. Run when touching any proxy startup / routing logic."""

import sys
from base import ProxyTestBase, BASE_URL


class HealthTests(ProxyTestBase):
    def test_health_check(self):
        name = "Health Check Endpoint"
        data = self._get("/health")
        if data and data.get("status") == "healthy":
            self.logger.info("  Server is healthy")
            self._pass(name)
        else:
            self._fail(name, f"Unexpected response: {data}")

    def _run_all(self):
        self.test_health_check()


if __name__ == "__main__":
    url = "http://localhost:8080" if "--local" in sys.argv else BASE_URL
    result = HealthTests(url).run()
    sys.exit(0 if result.failed == 0 else 1)

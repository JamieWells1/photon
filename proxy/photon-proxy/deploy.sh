#!/bin/bash

set -e

# Single instance only — multiple instances each have their own in-memory cache
# so cache hits are unreliable when a load balancer distributes across them.
NUM_INSTANCES=1

echo "======================================"
echo "Delta Proxy Deployment (single instance)"
echo "======================================"

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

# Stop and disable ALL possible old instances (not just 0..NUM_INSTANCES-1)
# so stale instances from a previous multi-instance deploy don't linger.
echo "Stopping any existing instances..."
for i in $(seq 0 9); do
    if systemctl is-active --quiet delta-proxy@$i 2>/dev/null; then
        echo "  - Stopping delta-proxy@$i"
        sudo systemctl stop delta-proxy@$i
        sudo systemctl disable delta-proxy@$i
    fi
done

echo "Starting $NUM_INSTANCES instance..."
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    PORT=$((8080 + i))
    echo "  - Instance $i on port $PORT"
    sudo systemctl enable delta-proxy@$i
    sudo systemctl start delta-proxy@$i
done

sleep 2

echo ""
echo "Service Status:"
echo "---------------"
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    if systemctl is-active --quiet delta-proxy@$i; then
        echo "  ✓ delta-proxy@$i is running on port $((8080 + i))"
    else
        echo "  ✗ delta-proxy@$i FAILED"
        sudo systemctl status delta-proxy@$i --no-pager -n 5
    fi
done

echo ""
echo "Restarting delta-test..."
if sudo systemctl restart delta-test; then
    echo "  ✓ delta-test is running"
else
    echo "  ✗ delta-test FAILED"
    sudo systemctl status delta-test --no-pager -n 5
fi

echo ""
echo "======================================"
echo "Deployment Complete!"
echo "======================================"
echo ""
echo "Management commands:"
echo "  Status:    sudo systemctl status delta-proxy@0"
echo "  Stop:      sudo systemctl stop delta-proxy@0"
echo "  Restart:   sudo systemctl restart delta-proxy@0"
echo "  Logs:      sudo journalctl -u delta-proxy@0 -f"
echo "  Test logs: sudo journalctl -u delta-test -f"
echo ""
echo "Direct access: http://YOUR_SERVER_IP:8080/"

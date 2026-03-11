#!/bin/bash

set -e

NUM_INSTANCES=3

echo "======================================"
echo "Photon Proxy Multi-Instance Deployment"
echo "======================================"

echo "Copying service template..."
sudo cp photon-proxy@.service /etc/systemd/system/

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

echo "Stopping existing instances..."
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    sudo systemctl stop photon-proxy@$i 2>/dev/null || true
done

echo "Starting $NUM_INSTANCES instances..."
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    PORT=$((8080 + i))
    echo "  - Instance $i on port $PORT"
    sudo systemctl enable photon-proxy@$i
    sudo systemctl start photon-proxy@$i
done

sleep 2

echo ""
echo "Service Status:"
echo "---------------"
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    if systemctl is-active --quiet photon-proxy@$i; then
        echo "  ✓ photon-proxy@$i is running on port $((8080 + i))"
    else
        echo "  ✗ photon-proxy@$i FAILED"
        sudo systemctl status photon-proxy@$i --no-pager -n 5
    fi
done

if command -v nginx &> /dev/null; then
    echo ""
    read -p "Setup nginx load balancer? (y/N) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Setting up nginx..."
        sudo cp nginx-photon-lb.conf /etc/nginx/sites-available/photon-proxy
        sudo ln -sf /etc/nginx/sites-available/photon-proxy /etc/nginx/sites-enabled/
        sudo nginx -t && sudo systemctl reload nginx
        echo "✓ nginx configured and reloaded"
    fi
fi

echo ""
echo "======================================"
echo "Deployment Complete!"
echo "======================================"
echo ""
echo "Management commands:"
echo "  View all:        sudo systemctl status 'photon-proxy@*'"
echo "  Stop all:        sudo systemctl stop 'photon-proxy@*'"
echo "  Start all:       sudo systemctl start 'photon-proxy@*'"
echo "  Restart all:     sudo systemctl restart 'photon-proxy@*'"
echo "  View logs:       sudo journalctl -u 'photon-proxy@*' -f"
echo "  View instance 0: sudo journalctl -u photon-proxy@0 -f"
echo ""
echo "Access via nginx:  http://YOUR_SERVER_IP/"
echo "Direct instances:"
for i in $(seq 0 $((NUM_INSTANCES - 1))); do
    echo "  Instance $i:     http://YOUR_SERVER_IP:$((8080 + i))/"
done

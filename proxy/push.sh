#!/bin/bash

set -e

VM_USER="root"
VM_HOST="138.68.190.163"
PROXY_DIR="/opt/delta-proxy"
TESTS_DIR="/opt/delta-test"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROXY_FILE="$SCRIPT_DIR/photon-proxy/https_proxy.py"
TESTS_DIR_LOCAL="$SCRIPT_DIR/photon-test"

TEST_FILES=(
    "base.py"
    "integration_tests.py"
    "test_health.py"
    "test_routing.py"
    "test_currency.py"
    "test_single.py"
    "test_batch.py"
    "test_cache.py"
    "test_edge_cases.py"
    "delta-test.service"
)

echo "======================================"
echo "Pushing files to $VM_USER@$VM_HOST"
echo "======================================"

echo "  -> $PROXY_DIR/https_proxy.py"
scp "$PROXY_FILE" "$VM_USER@$VM_HOST:$PROXY_DIR/https_proxy.py"

for f in "${TEST_FILES[@]}"; do
    if [[ "$f" == *.service ]]; then
        echo "  -> /etc/systemd/system/$f"
        scp "$TESTS_DIR_LOCAL/$f" "$VM_USER@$VM_HOST:/etc/systemd/system/$f"
    else
        echo "  -> $TESTS_DIR/$f"
        scp "$TESTS_DIR_LOCAL/$f" "$VM_USER@$VM_HOST:$TESTS_DIR/$f"
    fi
done

echo ""
echo "Done. To restart services:"
echo "  ssh $VM_USER@$VM_HOST 'systemctl daemon-reload && systemctl restart delta-proxy@0 && systemctl restart delta-test'"

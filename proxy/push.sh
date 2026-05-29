#!/bin/bash

set -e

VM_USER="root"
VM_HOST="138.68.190.163"
PROXY_DIR="/opt/delta-proxy"
TESTS_DIR="/opt/delta-test"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROXY_FILE="$SCRIPT_DIR/photon-proxy/https_proxy.py"
TESTS_FILE="$SCRIPT_DIR/photon-test/integration_tests.py"

echo "======================================"
echo "Pushing files to $VM_USER@$VM_HOST"
echo "======================================"

echo "  -> $PROXY_DIR/https_proxy.py"
scp "$PROXY_FILE" "$VM_USER@$VM_HOST:$PROXY_DIR/https_proxy.py"

echo "  -> $TESTS_DIR/integration_tests.py"
scp "$TESTS_FILE" "$VM_USER@$VM_HOST:$TESTS_DIR/integration_tests.py"

echo ""
echo "Done. To restart the proxy:"
echo "  ssh $VM_USER@$VM_HOST 'cd $VM_DIR && bash deploy.sh'"

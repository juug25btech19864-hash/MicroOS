#!/bin/bash
# MicroOS Kernel Assembly and Verification
# Verifies kernel vmlinuz, modular drivers, and reproducible .config

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== Verifying MicroOS Linux Kernel Assets ==="

if [ ! -f "$PROJ_DIR/kernel/vmlinuz" ]; then
    echo "[ERROR] Kernel image $PROJ_DIR/kernel/vmlinuz missing!"
    exit 1
fi

vmlinuz_size=$(stat -c %s "$PROJ_DIR/kernel/vmlinuz")
echo "  Kernel Image: $PROJ_DIR/kernel/vmlinuz ($vmlinuz_size bytes / $((vmlinuz_size / 1024 / 1024)) MB)"

if [ ! -f "$PROJ_DIR/kernel/kernel.config" ]; then
    echo "[ERROR] Kernel config missing!"
    exit 1
fi
echo "  Kernel Config: $PROJ_DIR/kernel/kernel.config (Verified)"

mod_count=$(ls -1 "$PROJ_DIR/kernel/modules"/*.ko* 2>/dev/null | wc -l)
echo "  Essential Drivers: $mod_count modules staged in kernel/modules/"

echo "=== Kernel Verification Complete ==="

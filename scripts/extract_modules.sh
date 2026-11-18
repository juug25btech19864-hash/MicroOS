#!/bin/bash
# MicroOS Essential Kernel Module Extractor & Optimizer
# Extracts storage (AHCI, NVMe), network (e1000, r8169), and wireless modules into kernel/modules

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
MODULES_DIR="$PROJ_DIR/kernel/modules"
DEB_FILE="/tmp/ktest/linux-modules-7.0.0-43-generic_7.0.0-43.43_amd64.deb"

mkdir -p "$MODULES_DIR"

if [ -f "$DEB_FILE" ]; then
    echo "=== Extracting Essential Modular Drivers from $DEB_FILE ==="
    WORKDIR="/tmp/mod_extract"
    rm -rf "$WORKDIR"
    mkdir -p "$WORKDIR"
    
    # Extract only drivers/ata, drivers/nvme, drivers/net/ethernet, drivers/usb, net/wireless
    dpkg-deb --fsys-tarfile "$DEB_FILE" | tar -xf - -C "$WORKDIR" \
        --wildcards \
        '*kernel/drivers/ata/ahci.ko*' \
        '*kernel/drivers/ata/libahci.ko*' \
        '*kernel/drivers/nvme/host/nvme.ko*' \
        '*kernel/drivers/nvme/host/nvme-core.ko*' \
        '*kernel/drivers/net/ethernet/intel/e1000/e1000.ko*' \
        '*kernel/drivers/net/ethernet/intel/e1000e/e1000e.ko*' \
        '*kernel/drivers/net/ethernet/realtek/r8169.ko*' \
        '*kernel/drivers/usb/storage/usb-storage.ko*' \
        '*kernel/net/wireless/cfg80211.ko*' \
        2>/dev/null || true

    find "$WORKDIR" -name "*.ko*" -exec cp {} "$MODULES_DIR/" \;
    rm -rf "$WORKDIR"

    # Decompress zst if needed and strip debug symbols
    for m in "$MODULES_DIR"/*.zst; do
        [ -f "$m" ] || continue
        zstd -d --rm "$m" 2>/dev/null || true
    done

    for m in "$MODULES_DIR"/*.ko; do
        [ -f "$m" ] || continue
        strip --strip-debug "$m" 2>/dev/null || true
    done

    echo "=== Essential Modules Extracted ==="
    ls -lh "$MODULES_DIR"
else
    echo "[WARN] Module deb not found at $DEB_FILE."
fi

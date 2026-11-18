#!/bin/bash
# MicroOS Initramfs Packaging Pipeline
# Packages rootfs into a compressed cpio archive (initramfs.cpio.gz)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ROOTFS_DIR="$PROJ_DIR/rootfs"
BUILD_DIR="$PROJ_DIR/build"
INITRAMFS_OUT="$BUILD_DIR/initramfs.cpio.gz"
BOOT_DIR="$PROJ_DIR/boot"

echo "=== Packaging MicroOS Initramfs ==="

mkdir -p "$BUILD_DIR" "$BOOT_DIR"

cd "$ROOTFS_DIR"
find . -mindepth 1 | cpio -o -H newc 2>/dev/null | gzip -9 > "$INITRAMFS_OUT"

cp "$INITRAMFS_OUT" "$BOOT_DIR/initramfs.cpio.gz"

initramfs_size=$(stat -c %s "$INITRAMFS_OUT")
echo "  Initramfs successfully created: $INITRAMFS_OUT"
echo "  Exact Size: $initramfs_size bytes ($((initramfs_size / 1024 / 1024)) MB, $((initramfs_size / 1024)) KB)"

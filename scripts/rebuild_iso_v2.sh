#!/bin/bash
# MicroOS ISO Rebuild v2 - Fixed GRUB configuration, no module restrictions
set -e

PROJ_DIR="/mnt/c/Users/M SRIVAS/.gemini/antigravity/scratch/MicroOS"
STAGING_DIR="/tmp/microos_iso_staging"
ISO_OUT="/tmp/microos-v2.iso"

echo "=== MicroOS ISO Rebuild v2 ==="

# Clean staging
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR/boot/grub"

# Stage kernel
echo "[STAGE] Kernel vmlinuz..."
cp "$PROJ_DIR/kernel/vmlinuz" "$STAGING_DIR/boot/vmlinuz"
echo "  Kernel size: $(stat -c %s "$STAGING_DIR/boot/vmlinuz") bytes"

# Stage initramfs
echo "[STAGE] Initramfs..."
if [ -f "$PROJ_DIR/build/initramfs.cpio.gz" ]; then
    cp "$PROJ_DIR/build/initramfs.cpio.gz" "$STAGING_DIR/boot/initramfs.cpio.gz"
    echo "  Initramfs size: $(stat -c %s "$STAGING_DIR/boot/initramfs.cpio.gz") bytes"
else
    echo "ERROR: initramfs not found at $PROJ_DIR/build/initramfs.cpio.gz"
    exit 1
fi

# Stage updated grub.cfg
echo "[STAGE] GRUB config..."
cp "$PROJ_DIR/boot/grub/grub.cfg" "$STAGING_DIR/boot/grub/grub.cfg"
cat "$STAGING_DIR/boot/grub/grub.cfg"

# Build ISO - no --install-modules restriction, full module set
echo "[BUILD] Running grub-mkrescue..."
grub-mkrescue -o "$ISO_OUT" "$STAGING_DIR" \
    --locales="" \
    --fonts=""

ISO_SIZE=$(stat -c %s "$ISO_OUT")
echo ""
echo "=== BUILD COMPLETE ==="
echo "ISO: $ISO_OUT"
echo "Size: $ISO_SIZE bytes ($(echo "scale=2; $ISO_SIZE/1024/1024" | bc) MB)"

if [ $ISO_SIZE -le 50000000 ]; then
    echo "STATUS: PASS - Under 50 MB limit"
else
    echo "STATUS: FAIL - Exceeds 50 MB limit!"
fi

# Copy to release
cp "$ISO_OUT" "$PROJ_DIR/release/microos-v2.iso"
sha256sum "$ISO_OUT" > "$PROJ_DIR/release/microos-v2.iso.sha256"
echo "Copied to: $PROJ_DIR/release/microos-v2.iso"

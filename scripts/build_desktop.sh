#!/usr/bin/env bash
set -e
PROJ_DIR="/mnt/c/Users/M SRIVAS/.gemini/antigravity/scratch/MicroOS"
SRC="$PROJ_DIR/desktop/microos-desktop.c"
OUT="$PROJ_DIR/rootfs/usr/bin/desktop"

echo "=== Building MicroOS Desktop ==="
# Ensure output directory exists
mkdir -p "$(dirname "$OUT")"
# Build static binary (size‑optimized)
# Use -static only if libc is static on the system; otherwise omit.
# We'll try dynamic linking first to avoid missing static libs.

gcc -Os -s -o "$OUT" "$SRC" -lncurses -lm

echo "Desktop binary size: $(stat -c %s \"$OUT\") bytes"

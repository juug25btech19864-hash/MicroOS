#!/usr/bin/env python3
"""
MicroOS ISO Size Verification Script
Authoritative check to guarantee compliance with the 50,000,000 byte hard maximum.
"""

import sys
import os

MAX_ALLOWED_BYTES = 50_000_000  # 50 MB hard limit

def verify_size(iso_path: str, max_bytes: int = MAX_ALLOWED_BYTES):
    if not os.path.isfile(iso_path):
        print(f"[ERROR] ISO file not found at: {iso_path}", file=sys.stderr)
        sys.exit(1)

    actual_size = os.path.getsize(iso_path)
    percent = (actual_size / max_bytes) * 100.0

    print("=" * 60)
    print("MICROOS ISO SIZE VERIFICATION")
    print(f"File:           {iso_path}")
    print(f"Actual size:    {actual_size:,} bytes ({actual_size / (1024*1024):.2f} MB)")
    print(f"Hard limit:     {max_bytes:,} bytes ({max_bytes / (1024*1024):.2f} MB)")
    print(f"Budget used:    {percent:.2f}%")
    print("=" * 60)

    if actual_size > max_bytes:
        excess = actual_size - max_bytes
        print(f"[FAILED] ISO exceeds size budget by {excess:,} bytes!", file=sys.stderr)
        sys.exit(2)
    else:
        remaining = max_bytes - actual_size
        print(f"[PASSED] ISO is strictly within budget. Remaining headroom: {remaining:,} bytes.")
        sys.exit(0)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: verify_size.py <path_to_iso> [max_bytes]")
        sys.exit(1)
    
    target_path = sys.argv[1]
    limit = int(sys.argv[2]) if len(sys.argv) > 2 else MAX_ALLOWED_BYTES
    verify_size(target_path, limit)

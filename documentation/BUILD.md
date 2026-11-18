# MicroOS Build System & Compilation Guide

## 1. Prerequisites & Toolchain

The MicroOS build system utilizes a modern x86_64 toolchain:
- **C Compiler**: GCC 14+ or Clang 18+ (C11 standard)
- **Assembler & Linker**: GNU binutils (as, ld, objcopy, strip)
- **Archival**: GNU cpio, gzip, xz-utils
- **ISO Tools**: xorriso, grub-mkrescue, mtools, dosfstools
- **Init Base**: BusyBox 1.36+ (multi-call static or glibc)

---

## 2. Directory Output Mapping

| Phase Output | Build Path | Release Path |
| :--- | :--- | :--- |
| Arcade Binaries | `build/games/<game>` | `rootfs/usr/games/<game>` |
| Hardware Tools | `hardware/hardware` | `rootfs/usr/bin/hardware` |
| Storage Tools | `storage/disks` | `rootfs/usr/bin/disks` |
| Network Tools | `networking/wifi` | `rootfs/usr/bin/wifi` |
| Kernel vmlinuz | `kernel/vmlinuz` | `boot/vmlinuz` |
| Kernel Modules | `kernel/modules/` | `rootfs/lib/modules/` |
| Initramfs Image | `build/initramfs.cpio.gz` | `boot/initramfs.cpio.gz` |
| GRUB Config | `boot/grub/grub.cfg` | `boot/grub/grub.cfg` |
| Bootable ISO | `build/microos.iso` | `release/microos.iso` |

---

## 3. Build Commands

```bash
# Clean all intermediate build objects
make clean

# Compile only terminal games
make games

# Build root filesystem tree
make rootfs

# Build custom kernel & initramfs
make kernel
make initramfs

# Master build: full pipeline & size check
make iso
```

---

## 4. Size Enforcement Rule
After generating `release/microos.iso`, the build pipeline executes:
```bash
python scripts/verify_size.py release/microos.iso 50000000
```
If the size exceeds 50,000,000 bytes, the build aborts immediately with a non-zero exit code.

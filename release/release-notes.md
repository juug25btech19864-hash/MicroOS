# MicroOS v1.0 Release Notes

**Minimal Linux. Maximum Control.**

MicroOS v1.0 is an ultra-lightweight, bootable x86_64 Linux operating system engineered for native laptops and virtualized environments.

## Highlights
- **Under 50 MB Hard Limit**: Base bootable medium occupies **25 MB (26298368 bytes)**, strictly meeting project constraints.
- **Dual Boot Architecture**: Seamlessly boots on modern UEFI systems and legacy BIOS machines.
- **Built-in Micro Arcade**: 8 standalone offline terminal games included in base ISO (`snake`, `tetris`, `pong`, `mines`, `2048`, `guess`, `tictactoe`, `spaceinvaders`, and `arcade` menu launcher).
- **Persistent Debian APT**: Independent persistent userspace architecture supporting Debian-compatible packages and repositories on attached SSD/USB storage.
- **Automatic Hardware Autodetection**: Built-in non-destructive hardware inspection tools (`hardware`, `hardware cpu`, `hardware memory`, `hardware network`, `hardware storage`, `hardware usb`).
- **Safe Storage Management**: Safe non-destructive disk utilities (`disks`, `disks info`, `mount`, `unmount`).
- **Native Wireless Networking**: Linux networking stack with `wifi` CLI and `iwd` integration.

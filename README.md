# MicroOS — Ultra-Lightweight Linux Operating System

MicroOS is a production-grade, ultra-lightweight, bootable x86_64 Linux operating system engineered for native hardware and virtual machine environments.

> **Minimal Linux. Maximum Control.**

---

## 1. Key Specifications

| Specification | Target / Implementation |
| :--- | :--- |
| **Architecture** | x86_64 (Intel & AMD) |
| **Boot Mode** | Hybrid BIOS (MBR/El Torito) & UEFI (x86_64 EFI) |
| **Bootloader** | GNU GRUB 2.14+ |
| **Kernel** | Customized Linux x86_64 (Modular Drivers, ACPI, NVMe, AHCI, USB, ext4/FAT) |
| **Init System** | BusyBox Init (`/init` -> inittab) |
| **Shell** | BusyBox ash with custom MicroOS CLI environment |
| **Networking** | Native Linux stack, iwd, BusyBox udhcpc, iproute2, custom `wifi` tool |
| **Storage Management** | Safe non-destructive disk manager (`disks`, `mount`, `unmount`) |
| **Package Management** | Debian-compatible APT & dpkg via persistent userspace container/layer |
| **Built-in Arcade** | 8 standalone C terminal arcade games (`arcade`, `snake`, `tetris`, etc.) |
| **ISO Size Target** | **< 50 MB (50,000,000 bytes hard cap)** |
| **Offline Operation** | 100% functional without host OS or internet connection |
| **Testing Target** | Oracle VirtualBox (1024 MB RAM, 1 vCPU, virtual HDD) |

---

## 2. Directory Structure

```text
MicroOS/
├── boot/              # Bootloader configuration, GRUB assets, EFI staging
├── kernel/            # Kernel configs, build scripts, kernel artifacts
├── rootfs/            # Root filesystem tree, skeleton, BusyBox integration
├── init/              # Init scripts, inittab, runlevel scripts (/init, rcS, rcK)
├── networking/        # Network stack scripts, iwd profiles, wifi management CLI
├── hardware/          # Hardware auto-detection scripts and utilities
├── storage/           # Disk management utilities, safe mounting tools
├── package-manager/   # Debian/APT persistent integration architecture
├── shell/             # Custom MicroOS shell prompts, profile, aliases, sysinfo
├── arcade/            # Arcade launcher frontend and high-score manager
├── games/             # Source code and Makefiles for the 8 terminal games
├── scripts/           # Master build and ISO generation automation scripts
├── configs/           # Kernel, BusyBox, and system configuration templates
├── tests/             # Automated test harnesses (VirtualBox, unit tests)
├── documentation/     # Comprehensive system documentation
├── build/             # Intermediate build objects and progress.json
├── logs/              # Build, test, and optimization logs
├── release/           # Final bootable ISO, checksums, and release notes
├── Makefile           # Master orchestration Makefile
└── README.md          # Primary project overview
```

---

## 3. Fast Start & Building

To build the complete MicroOS distribution from source:

```bash
make all
```

This compiles:
1. Terminal arcade games (`games/`)
2. Hardware detection & system utilities (`hardware/`, `storage/`, `networking/`, `shell/`)
3. Root filesystem skeleton (`rootfs/`)
4. Linux kernel and modules (`kernel/`)
5. Compressed initramfs (`init/`)
6. Hybrid bootable ISO image under 50 MB (`release/microos.iso`)
7. SHA256 verification checksums

---

## 4. Virtual Machine Testing

MicroOS is verified on Oracle VirtualBox:
- 1024 MB RAM
- 1 vCPU
- No host network adapter required for offline validation
- Virtual SATA/AHCI hard drive attached

*Note: Per strict project requirements, QEMU is never used.*

---

## 5. License

Developed for the MicroOS Open Source Operating System Initiative. Distributed under GPLv2/GPLv3.

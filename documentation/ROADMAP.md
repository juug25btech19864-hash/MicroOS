# MicroOS Project Roadmap & Phase Tracking

This roadmap defines the sequential engineering phases for MicroOS v1.0, tracking objectives, technical strategies, and verification gates.

---

## Phase Matrix

| Phase | Description | Key Deliverable | Verification Gate |
| :--- | :--- | :--- | :--- |
| **Phase 1** | Project Initialization & Tooling | Directory skeleton, Makefile, Docs, Progress tracker | Clean workspace, build scripts executable |
| **Phase 2** | Linux Kernel Build & Config | Optimized x86_64 vmlinuz + stripped modules | Kernel image < 12 MB, x86_64 EFI/BIOS capable |
| **Phase 3** | Minimal Root Filesystem | BusyBox static/glibc tree, `/bin`, `/sbin`, `/etc` | Functional standard POSIX shell & utilities |
| **Phase 4** | Bootloader & Init System | GRUB BIOS/UEFI + `/init` + BusyBox inittab | Successful kernel-to-userspace handover |
| **Phase 5** | Automatic Hardware Detection | `hardware` tool suite (sysfs/procfs parser) | CPU, RAM, PCI, USB, Storage correctly reported |
| **Phase 6** | Wi-Fi & Networking Stack | Native stack, iwd, DHCP, `wifi` CLI | Network interfaces detected, link state managed |
| **Phase 7** | Storage & SSD Management | `disks`, `mount`, `unmount` tools | Non-destructive disk scanning, GPT/MBR/ext4/FAT |
| **Phase 8** | Debian APT & DPKG Integration | Persistent userspace package manager architecture | glibc runtime compatibility, apt toolchain wrapper |
| **Phase 9** | MicroOS Custom CLI Shell | Custom branded shell, prompt, commands, help | Fast responsive ash shell with custom utilities |
| **Phase 10** | Persistent Storage Subsystem | SSD persistence layer for packages/data/scores | Live fallback if SSD absent; persistence if present |
| **Phase 11** | ISO Generation & Size Verification| Hybrid xorriso ISO image (`microos.iso`) | **Hard ISO size < 50,000,000 bytes** |
| **Phase 12** | VirtualBox Virtual Machine Testing| Oracle VirtualBox automated test run | BIOS & UEFI boot, 1024 MB RAM, 1 CPU, no QEMU |
| **Phase 13** | Debugging & Size Optimization | Strip binaries, optimize XZ/GZIP compression | ISO size reduction, zero missing core modules |
| **Phase 14** | USB Flashing & Boot Deployment | Rufus / Ventoy / balenaEtcher instructions | SHA256 checksums, hybrid ISO MBR/GPT validation |
| **Phase 15** | Native Hardware Compatibility | Physical laptop hardware compatibility matrix | Real hardware detection validation |
| **Phase 16** | Final Release & Packaging | MicroOS v1.0 complete distribution package | Signed release artifacts, documentation |
| **Phase 17** | Built-in Terminal Micro Arcade | 8 standalone C arcade games + launcher | Snake, Tetris, Pong, Mines, 2048, Guess, TTT, Space |

---

## Critical Milestone Gates

1. **Size Gate**: `stat -c %s release/microos.iso` <= 50,000,000 bytes.
2. **Boot Gate**: Boots cleanly to MicroOS shell in VirtualBox (1024MB RAM, 1 CPU, no QEMU).
3. **Execution Gate**: Terminal arcade games launch instantly, render cleanly, and capture user input correctly.
4. **Safety Gate**: Storage management never performs unprompted formatting or destructive operations on host or target disks.

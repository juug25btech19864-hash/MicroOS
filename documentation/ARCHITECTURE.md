# MicroOS System Architecture

## 1. Overview & Architectural Principles

MicroOS is built around three core engineering principles:
1. **Ultra-lightweight Footprint**: Base bootable medium strictly under 50 MB.
2. **Deterministic Boot Sequence**: Robust multi-platform hardware support (Intel/AMD x86_64) across legacy BIOS and modern UEFI.
3. **Decoupled Persistent Userspace**: Separation of the immutable, crash-resilient core live environment from the persistent Debian-compatible APT/dpkg application space.

```
+----------------------------------------------------------------------------------+
|                                    MicroOS                                       |
+----------------------------------------------------------------------------------+
|  User Interface: MicroOS Branded ash Shell + Custom CLI (`hardware`, `wifi`, etc)|
|  Micro Arcade: 8 Native C Terminal Games (`snake`, `tetris`, `pong`, ...)        |
+----------------------------------------------------------------------------------+
|  Persistent Userspace (Optional SSD / USB Partition)                             |
|  - glibc Userspace Compatibility Layer                                            |
|  - Debian APT / dpkg / repository cache / user binaries / arcade high scores    |
+----------------------------------------------------------------------------------+
|  Base Live Root Filesystem (Initramfs / RAM Disk)                                |
|  - BusyBox multicall binary (ash, coreutils, procps, net-tools, util-linux)       |
|  - /init initialization orchestrator & inittab runlevels                         |
|  - Hardware autodetection daemon & device nodes (/dev, /sys, /proc, /run)        |
|  - Core glibc / musl runtimes & shared libraries                                 |
+----------------------------------------------------------------------------------+
|  Linux Kernel (x86_64 vmlinuz + Essential Builtin/Modular Drivers)               |
|  - CPU: Intel Core/Xeon/Atom, AMD Ryzen/EPYC, ACPI 6+, x86 APIC/IO-APIC          |
|  - Storage: NVMe 1.4+, AHCI SATA, USB Mass Storage, SCSI                         |
|  - Filesystems: ext4, VFAT, FAT32, exFAT, ISO9660, SquashFS/tmpfs               |
|  - Networking: Realtek, Intel, MediaTek ethernet & Wi-Fi subsystems (cfg80211)   |
+----------------------------------------------------------------------------------+
|  Bootloader: GNU GRUB 2.14+ Hybrid (El Torito BIOS + EFI System Partition x86_64)|
+----------------------------------------------------------------------------------+
|  Hardware / Hypervisor: Physical Laptop / Oracle VirtualBox (1024MB, 1 CPU)      |
+----------------------------------------------------------------------------------+
```

---

## 2. Boot Flow Architecture

```
Firmware (BIOS or UEFI)
       │
       ▼
GNU GRUB 2 (grub.cfg / bios.img / bootx64.efi)
       │
       ▼
Linux Kernel (vmlinuz x86_64)
       │
       ▼
Compressed Initramfs (initramfs.cpio.gz)
       │
       ▼
/init (PID 1 Orchestrator)
       │  Mounts /proc, /sys, /dev, /dev/pts, /run, /tmp
       │  Populates mdev / devtmpfs
       │  Scans for Persistent Storage (label MICROOS_PERSIST / MICROOS_DATA)
       │  Configures TTY console & branding
       │
       ▼
BusyBox Init (inittab: sysinit -> respawn ash)
       │
       ▼
MicroOS Interactive CLI Environment
       │
       ├─► Built-in Commands (`help`, `sysinfo`, `hardware`, `wifi`, `disks`, `mount`)
       ├─► Micro Arcade Launcher (`arcade`, `snake`, `tetris`, `pong`, ...)
       └─► Persistent Debian APT Subsystem (`apt update`, `apt install`)
```

---

## 3. Storage & Persistence Decoupling Strategy

To honor the **50 MB ISO hard maximum** while delivering full **Debian APT/dpkg** compatibility:
1. **Live Base**: The ISO contains the complete operational operating system, shell, hardware autodetection, networking stack, disk manager, and all 8 arcade games.
2. **Persistent Overlay / Root**:
   - APT, dpkg, debian package repositories, downloaded debs, and installed glibc packages reside on an external or internal drive partition (e.g. `/mnt/ssd/microos-packages` or dedicated ext4 partition).
   - If persistent storage is present, the `/init` script mounts it to `/var/cache/apt`, `/var/lib/dpkg`, and `/opt/debian`, binding paths seamlessly into `$PATH` and `$LD_LIBRARY_PATH`.
   - If persistent storage is absent, MicroOS boots directly into the live RAM environment, alerting the user that APT operations require mounting a persistent partition.

---

## 4. Hardware Detection Philosophy

MicroOS does not make external calls or guess device hardware:
- Directly traverses `/sys/bus/pci/devices`, `/sys/class/net`, `/sys/block`, `/sys/devices/virtual/dmi/id`.
- Reads `/proc/cpuinfo`, `/proc/meminfo`, `/proc/partitions`.
- Zero external Python/Perl dependencies inside the initramfs: pure POSIX shell and optimized C parsing.

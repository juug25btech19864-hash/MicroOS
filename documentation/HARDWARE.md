# MicroOS Hardware Support & Detection Architecture

## 1. Supported Hardware Profiles

MicroOS is engineered to run seamlessly across both modern and legacy x86_64 architectures:

| Subsystem | Physical Laptop Implementation | VirtualBox Virtual Machine Implementation |
| :--- | :--- | :--- |
| **Processor** | Intel Core (Nehalem through Raptor Lake/Meteor Lake), AMD Ryzen (Zen 1 through Zen 5), AMD FX | 1 vCPU (x86_64 host pass-through) |
| **System Memory** | 512 MB minimum, 1024 MB recommended, up to 128 GB+ | 1024 MB assigned RAM |
| **Firmware Mode** | UEFI x86_64 (CSM on/off) and Legacy BIOS | Oracle VirtualBox BIOS or EFI |
| **Storage Controllers** | PCI-Express NVMe 1.3/1.4+, AHCI SATA III, USB 2.0/3.0 Mass Storage (UAS/bot) | VirtualBox SATA Controller (AHCI) |
| **Partition Tables** | GPT (GUID Partition Table) & MBR (MS-DOS) | GPT & MBR |
| **Filesystems** | ext4, FAT16, FAT32, exFAT, ISO9660 | ext4, FAT32, ISO9660 |
| **Network (Ethernet)**| Realtek (r8169), Intel (e1000e, igb), Broadcom (tg3) | Intel PRO/1000 MT Desktop (82540EM) |
| **Network (Wi-Fi)** | Intel Wireless (iwlwifi), Realtek (rtw88/rtl8xxxu), MediaTek (mt76), Atheros (ath9k/ath10k) | Unused for offline tests |
| **Display/Console** | Standard VGA, VESA FB, EFI framebuffer, Intel/AMD/Nvidia direct TTY console | VESA / VBoxVGA text framebuffer |

---

## 2. Hardware Autodetection Engine

Hardware inspection is implemented through the `hardware` suite located in `/usr/bin/hardware`.

### Syntax
```bash
hardware           # Complete hardware overview report
hardware cpu       # CPU model, vendor, frequency, cores, flags
hardware memory    # RAM total, free, buffers, cached, swap
hardware network   # Network interfaces (MAC, state, driver, bus)
hardware storage   # Block devices, partitions, controllers (NVMe/SATA/USB)
hardware usb       # USB host controllers, hubs, connected devices
hardware pci       # PCI bus inspection (IDs, vendors, kernel drivers)
```

### Detection Mechanics
- **CPU**: Extracted from `/proc/cpuinfo` parsing `model name`, `vendor_id`, `cpu MHz`, and `flags`.
- **Memory**: Extracted from `/proc/meminfo` computing active memory usage and availability.
- **Storage**: Traverses `/sys/block/` verifying non-virtual physical devices (`sd*`, `nvme*`, `vd*`).
- **Network**: Traverses `/sys/class/net/`, inspecting `/sys/class/net/<iface>/device/uevent` to extract driver identity and bus addresses.
- **Safety**: Purely read-only; zero disk writing, probing, or destructive bus commands.

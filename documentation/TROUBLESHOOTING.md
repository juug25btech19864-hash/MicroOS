# MicroOS Troubleshooting & Diagnostics Guide

This guide provides diagnostic procedures and recovery workflows for common operating system scenarios.

---

## 1. Boot Scenarios & Recovery

### Issue: System hangs during early boot before reaching shell
- **Cause**: Kernel modesetting or framebuffer issue on specific discrete graphics cards (NVIDIA/AMD).
- **Resolution**:
  1. In the GRUB boot menu, select `MicroOS v1.0 (Verbose / Diagnostics Mode)`.
  2. Or press `e` in GRUB to edit the boot entry and append `nomodeset vga=current` to the `linux` line.
  3. Press `Ctrl+X` or `F10` to boot.

### Issue: Drops into emergency shell during boot
- **Cause**: Filesystem corruption on initramfs or missing critical root device.
- **Resolution**:
  - The `/init` script includes an automated `trap` that drops into an emergency BusyBox ash shell (`/bin/sh`).
  - At the emergency shell, type:
    ```sh
    cat /proc/cmdline
    dmesg | tail -n 30
    ```
  - Inspect kernel messages to identify missing drivers or bus enumeration issues.

---

## 2. Storage & Filesystem Issues

### Issue: Storage drive not appearing in `disks`
- **Cause**: Storage controller kernel module not loaded (e.g. specialized PCIe RAID).
- **Resolution**:
  - Run `hardware storage` to check raw sysfs block devices.
  - Run `hardware pci` to verify if the storage controller is detected on the PCI bus.
  - If the controller uses AHCI or NVMe, ensure the device is not set to proprietary "Intel RST / VMD" in the laptop BIOS; switch storage mode to **AHCI / Standard NVMe**.

### Issue: Cannot write to mounted partition
- **Cause**: Partition mounted in safe Read-Only mode due to unclean dirty flags (e.g. previous Windows Fast Startup hibernation).
- **Resolution**:
  - Windows Fast Startup leaves NTFS partitions in a locked hibernation state. Disable Fast Startup in Windows Power Settings before accessing partitions from Linux.
  - Remount manually: `mount /dev/sda1 /mnt/ssd -o remount,rw`.

---

## 3. Wi-Fi & Networking Issues

### Issue: `wifi scan` reports "No physical Wi-Fi radio detected"
- **Cause 1**: Running inside Oracle VirtualBox.
  - *Explanation*: Virtual machines do not emulate 802.11 Wi-Fi radios. Use the virtual Ethernet adapter or boot MicroOS natively on a physical laptop.
- **Cause 2**: Missing proprietary wireless firmware.
  - *Explanation*: To maintain the strict <50MB ISO size, proprietary multi-megabyte firmware blobs are not pre-bundled. Connect via Ethernet or copy required firmware to `/lib/firmware` on a persistent drive.

---

## 4. Package Management (APT) Issues

### Issue: `apt update` reports "Persistent storage not mounted"
- **Cause**: MicroOS base ISO is immutable and runs in RAM. APT package databases and installed software require persistent storage to prevent consuming all RAM.
- **Resolution**:
  ```sh
  disks
  mount /dev/sda1 /mnt/ssd
  apt init
  apt update
  ```

---

## 5. Micro Arcade Terminal Rendering

### Issue: Game graphics distorted or misaligned
- **Cause**: Terminal window dimensions too small.
- **Resolution**:
  - Resize your terminal to at least 80 columns by 25 rows.
  - Ensure your terminal emulator supports standard ANSI escape sequences (Linux TTY console, PuTTY, xterm, Windows Terminal all fully supported).

# MicroOS USB Flashing & Deployment Guide

MicroOS v1.0 is packaged as a standard hybrid ISO image (`release/microos.iso`), making it natively compatible with USB flash drives, memory cards, and virtual machines.

---

## 1. Cryptographic Hash Verification

Before flashing, always verify the SHA256 integrity of the ISO image:

### Windows PowerShell
```powershell
Get-FileHash .\release\microos.iso -Algorithm SHA256
```
Compare the resulting hash against `release\microos.iso.sha256`.

### Linux / WSL / macOS
```bash
cd release
sha256sum -c microos.iso.sha256
```
Expected output:
```text
microos.iso: OK
```

---

## 2. Flashing Tools & Instructions

### Tool A: Rufus (Windows)
1. Download and launch **Rufus** (v4.0 or newer).
2. Under **Device**, select your USB flash drive (minimum 128 MB capacity required).
3. Under **Boot selection**, click **SELECT** and browse to `release\microos.iso`.
4. Under **Partition scheme**:
   - Select **MBR** for dual BIOS/UEFI compatibility.
   - Select **GPT** for strictly modern UEFI machines.
5. Under **Target system**, select **BIOS or UEFI**.
6. Click **START**. If prompted between "ISO Image mode" and "DD Image mode", choose either (both work thanks to the hybrid ISO layout).
7. Wait for completion and safely eject the USB drive.

### Tool B: balenaEtcher (Windows, macOS, Linux)
1. Open **balenaEtcher**.
2. Click **Flash from file** and choose `release/microos.iso`.
3. Click **Select target** and choose your USB drive.
4. Click **Flash!**
5. Etcher will write the hybrid MBR/GPT structures and verify data integrity automatically.

### Tool C: Ventoy (Multi-Boot USB)
1. Prepare a USB drive with **Ventoy** (v1.0.90+).
2. Simply copy `release/microos.iso` into the Ventoy exFAT/FAT partition.
3. Boot your computer, select the USB drive in the firmware boot menu (F12/F11/F8/Esc), and choose **MicroOS** from the Ventoy menu.

### Tool D: Linux Terminal (\`dd\`)
```bash
# Identify your USB device (e.g. /dev/sdb)
lsblk

# Write raw hybrid image to USB device (replace /dev/sdX with your actual USB block device)
sudo dd if=release/microos.iso of=/dev/sdX bs=4M status=progress conv=fsync
```

---

## 3. Booting on Native Laptop Hardware

1. Insert the flashed USB drive into the target laptop.
2. Power on the laptop and immediately tap the BIOS/UEFI Boot Menu key:
   - **Dell**: `F12`
   - **Lenovo**: `F12` or `Enter` -> `F12`
   - **HP**: `F9` or `Esc` -> `F9`
   - **ASUS / Acer**: `F8` or `F12`
3. Select your USB drive from the boot menu (either UEFI or Legacy USB).
4. The GNU GRUB boot menu will appear:
   - Select `MicroOS v1.0 (Standard Boot)` for normal operation.
   - Select `MicroOS v1.0 (Verbose / Diagnostics Mode)` for hardware debugging.
5. In under 2 seconds, the MicroOS shell prompt will appear.

---

## 4. Non-Destructive Safety Guarantee

MicroOS runs **entirely in RAM**. It does **not**:
- Overwrite existing operating systems (Windows, macOS, or other Linux distributions).
- Repartition or format your internal NVMe or SATA SSDs automatically.
- Modify the host EFI system partition or NVRAM boot variables.

All storage operations require explicit manual commands using the safe `disks`, `mount`, and `unmount` toolset.

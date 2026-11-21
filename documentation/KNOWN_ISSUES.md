# MicroOS Known Issues & Design Trade-offs

This document maintains an honest, empirical log of known issues, hardware edge cases, and design constraints.

---

## 1. ISO Size Constraints (< 50 MB)

- **Issue**: Standard Debian repositories contain hundreds of megabytes of firmware blobs (`linux-firmware`), multi-megabyte shared libraries, and large manpages/locales.
- **Resolution**:
  - Unnecessary firmware is excluded from the base ISO. Only critical network and storage drivers/firmware are bundled.
  - An optional external firmware and package repository mechanism is provided for persistent storage (`/mnt/ssd`).
  - Strict binary stripping (`strip --strip-all`) is enforced on all compiled binaries and kernel modules.

---

## 2. Emulated vs. Physical Wi-Fi

- **Issue**: Oracle VirtualBox does not emulate a wireless 802.11 card; it presents an emulated Intel PRO/1000 or PCnet-PCI II wired Ethernet NIC.
- **Resolution**:
  - The `wifi` tool cleanly detects whether an 802.11 wireless PHY interface exists.
  - In VirtualBox, `wifi scan` reports that no physical Wi-Fi radio is detected and directs the user to Ethernet or informs them that physical Wi-Fi requires native laptop hardware boot.
  - Native laptop hardware boots detect physical Intel, Realtek, Atheros, or MediaTek Wi-Fi chipsets seamlessly.

---

## 3. Persistent Debian APT Architecture

- **Issue**: Running Debian `apt` and `dpkg` requires glibc, dpkg databases, status files, and repository lists that would easily exceed the 50 MB ISO limit if pre-bundled with full databases.
- **Resolution**:
  - The base boot medium operates a pristine RAM disk with full core utilities.
  - Persistent storage (`/mnt/ssd` or designated USB partition) hosts the dynamic APT cache, `/var/lib/dpkg`, and installed Debian packages.
  - When storage is not attached, `apt` outputs a clear, instructive message indicating persistent storage mounting is required for package installations.

---

## 4. Host File Locks & Permissions during Rebuilds

- **Issue**: If `microos.iso` is mounted by Windows Explorer as a virtual CD-ROM drive (e.g., drive `D:`), or if VirtualBox holds the image open in a background service (`VBoxSDS`), rebuild scripts report `Permission denied` when overwriting `release/microos.iso`.
- **Resolution**:
  - The build script compiles the ISO image to a scratch staging path (`/tmp/microos_build.iso`) and then copies it.
  - If Windows locks the ISO, dismount via `Dismount-DiskImage -ImagePath release/microos.iso`.
  - In VirtualBox, stale DVD media entries are safely closed with `VBoxManage closemedium dvd <uuid>`.

---

## 5. ICMP Ping Socket Permissions

- **Issue**: Traditional Linux raw ICMP sockets require `CAP_NET_RAW` or root UID, which can cause `ping: permission denied` for standard unprivileged users.
- **Resolution**:
  - `/init` early user space configuration writes `0 2147483647` into `/proc/sys/net/ipv4/ping_group_range`.
  - Setuid bit `chmod 4755` is applied to `/usr/bin/ping`.
  - Unprivileged users can ping both IPv4 and IPv6 endpoints seamlessly.

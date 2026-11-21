# MicroOS v2.0 Networking Architecture & Guide

MicroOS v2.0 features a native, unified Linux networking stack providing wireless (802.11) and wired (Ethernet) management, DNS resolution, and diagnostics.

---

## 1. Network Components & Stack

| Component | Technology / Implementation | Role |
| :--- | :--- | :--- |
| **Kernel Subsystem** | Linux `cfg80211` + `mac80211` | Core wireless device management and regulatory compliance |
| **Wireless Daemon** | `iwd` (Intel Wireless Daemon) | High-speed WPA2/WPA3 enterprise and personal connection manager |
| **Wireless Client** | `iwctl` / MicroOS `wifi` CLI | Interactive network scanner, connection orchestrator, and status reporter |
| **Network Manager** | `net` | Unified status card, routing table inspection, DNS auditor, interface control |
| **DHCP Client** | BusyBox `udhcpc` | Dynamic IP address and lease acquisition |
| **Diagnostic Ping** | `ping` (ICMP Echo) | Round-trip latency, packet loss, and offline failure diagnosis |
| **DNS Resolver** | `/etc/resolv.conf` | Static and dynamic upstream nameserver resolution |

---

## 2. Command Reference

### Wi-Fi Controller (`wifi`)

- **Interactive Menu:**
  ```sh
  wifi
  ```
  Opens the interactive menu with options to Scan, Connect, Disconnect, View Status, and Inspect Adapter Hardware.

- **Scan Networks:**
  ```sh
  wifi scan
  ```
  Activates wireless interface, unblocks `rfkill` if required, and retrieves all active nearby 802.11 SSIDs.

- **Connect to Network (Secure Interactive Prompt):**
  ```sh
  wifi connect <SSID>
  ```
  Prompts for the WPA2/WPA3 network passphrase with masked input (hidden from terminal display, shell history, and logs).

- **Connect with Inline Passphrase:**
  ```sh
  wifi connect <SSID> --password <passphrase>
  ```

- **Disconnect:**
  ```sh
  wifi disconnect
  ```

- **Status & Adapter Information:**
  ```sh
  wifi status
  wifi info
  ```

---

### Unified Network Manager (`net`)

- **Live Status Card:**
  ```sh
  net
  # or
  net status
  ```
  Displays an ASCII/Unicode status dashboard showing active interface, connection type (Wi-Fi vs Ethernet), IP address, default gateway, DNS servers, and Internet connectivity status.

- **Interface Enumeration:**
  ```sh
  net interfaces
  ```
  Lists all physical and virtual interfaces (`eth0`, `wlan0`, `lo`), operational states, MAC addresses, and assigned IP addresses.

- **IP & Routing:**
  ```sh
  net ip
  net route
  ```

- **DNS Configuration:**
  ```sh
  net dns
  ```

- **Restart & Interface Control:**
  ```sh
  net restart
  net connect <interface>
  net disconnect <interface>
  ```

---

### Diagnostic Ping (`ping`)

- **Continuous Ping:**
  ```sh
  ping 8.8.8.8
  ping google.com
  ```
  *(Press `Ctrl+C` to terminate and review packet statistics)*

- **Count-limited Ping:**
  ```sh
  ping -c 4 8.8.8.8
  ping -c 5 1.1.1.1
  ```

- **Offline Diagnostics:**
  If the network is unreachable, `ping` automatically detects the missing interface or missing default gateway route and outputs actionable troubleshooting guidance rather than a silent failure.

---

## 3. Virtualization vs Physical Hardware Notes

> [!NOTE]
> - **VirtualBox Testing:** Oracle VirtualBox does not emulate 802.11 Wi-Fi radios. It provides an emulated wired Ethernet controller (e.g. Intel PRO/1000 or PCnet-PCI II). In VirtualBox, use `net` or `net status` to manage the wired link.
> - **Native Laptops:** When booted natively via USB or SSD, `cfg80211` and wireless drivers initialize the physical Wi-Fi chipset, enabling full 802.11 scanning and connection via `wifi`.

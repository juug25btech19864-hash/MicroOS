# MicroOS v2.0 Shell & Command Layer Architecture

The MicroOS command layer provides an intuitive, high-reliability command-line environment built on top of BusyBox `ash`.

---

## 1. Shell Environment

- **Primary Shell:** BusyBox Ash multicall binary (`/bin/sh` -> `/bin/busybox`).
- **Prompt:**
  ```sh
  micro@micro ~ $
  ```
  Styled with bold cyan for user/hostname, royal blue for current working directory (`\w`), and emerald green for the prompt symbol (`$`).
- **Configuration Path:** `/etc/profile`

---

## 2. Command Reference & Dispatcher

Typing `help` loads the dynamic MicroOS Command Reference.

```
╔══════════════════════════════════════════════════════════════════╗
║                            MICROOS                               ║
║                       COMMAND REFERENCE                          ║
║                Minimal Linux. Maximum Control.                   ║
╚══════════════════════════════════════════════════════════════════╝
```

### Dedicated Subcommand Manuals (`help <cmd>`)

Typing `help <cmd>` provides detailed syntax, options, security notes, and examples for that command.

| Command | Manual Synopsis |
| :--- | :--- |
| `help wifi` | Interactive Wi-Fi menu, network scanning, secure passphrase connection, and driver info |
| `help net` | Unified network status dashboard, route tables, DNS, and interface control |
| `help ping` | Packet transmission, latency tracking, and offline troubleshooting |
| `help hardware` | CPU, RAM, PCI, USB, and storage hardware audit |
| `help disks` | Storage drive enumeration and filesystem inspection |
| `help mount` | Safe storage mounting with read-only corruption fallback |
| `help unmount` | Storage buffer flushing and clean unmounting |
| `help apt` | Persistent Debian package manager on SSD storage |
| `help dashboard` | Windows 11/macOS-style Control Center launcher |
| `help settings` | System settings, hostname, date/time, and color themes |
| `help diagnostics` | Automated 8-point system and network health suite |
| `help arcade` | Retro terminal arcade launcher and 8 included C games |
| `help sysinfo` | System dashboard, uptime, load averages, and memory |

---

## 3. Command Aliases

To streamline usability and match muscle memory from Windows PowerShell, macOS, and Linux:

| Alias | Target Command | Purpose |
| :--- | :--- | :--- |
| `cls` | `clear` | Clears terminal display (Windows muscle memory) |
| `ll` | `ls -la` | Long listing with hidden files and permissions |
| `la` | `ls -A` | List almost all files |
| `l` | `ls -CF` | Formatted column listing |
| `info` | `sysinfo` | System dashboard and resource metrics |
| `netinfo` | `net status` | Unified network status card |
| `games` | `arcade` | Launch retro arcade menu |
| `control` | `settings` | System preferences menu |
| `restart` | `reboot` | Safely reboot operating system |
| `shutdown` | `poweroff` | Safely halt hardware power |
| `umount` | `unmount` | Safe filesystem unmounting with sync |

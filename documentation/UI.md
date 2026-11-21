# MicroOS v2.0 UI & Terminal Experience Guide

MicroOS v2.0 delivers a terminal-based user experience inspired by the modern aesthetic and usability of Windows 11 and macOS, while remaining a lightweight, graphical-server-free Linux operating system.

---

## 1. Visual Design Language

- **Unicode Box Drawing:** Uses `┌───┐`, `└───┘`, `╔═══╗`, and `╚═══╝` borders to organize information into visual cards and modular panels.
- **Color Palette:**
  - **Headers / Accent:** Cyan (`\033[1;36m`)
  - **Success / Status Up:** Emerald Green (`\033[1;32m`)
  - **Warning / Subheadings:** Warm Yellow (`\033[1;33m`)
  - **Errors / Critical:** Crimson Red (`\033[1;31m`)
  - **Dividers & Hints:** Subtle Gray (`\033[0;90m`)
- **Graceful Degradation:** All tools automatically detect non-interactive pipelines or monochromatic serial terminals and disable escape codes to prevent garbled text.

---

## 2. Interactive Control Center (`dashboard`)

The `dashboard` command launches an interactive Control Center:

```
╔══════════════════════════════════════════════════╗
║                    MICROOS                       ║
║                 CONTROL CENTER                   ║
╠══════════════════════════════════════════════════╣
║                                                  ║
║  [1] System Information                          ║
║  [2] Network & Wi-Fi                             ║
║  [3] Storage & Disks                             ║
║  [4] Applications & Packages (APT)               ║
║  [5] Micro Arcade (8 Retro Games)                ║
║  [6] System Settings & Control                   ║
║  [7] System Diagnostics                          ║
║  [8] Return to Terminal Shell                    ║
║  [9] Power / Reboot / Shutdown                   ║
║                                                  ║
╚══════════════════════════════════════════════════╝
```

Each menu item launches its designated subsystem with pause-and-return navigation, enabling users to manage the OS without memorizing CLI syntax.

---

## 3. System Settings Panel (`settings` / `control`)

The `settings` command provides interactive configuration:
1. **Network & IP:** Review interfaces and trigger DHCP renewals.
2. **Wi-Fi Connections:** Launch interactive wireless scan and connection wizard.
3. **System Information:** Live CPU, memory, and hardware device audit.
4. **Storage & Partitions:** Disk maps and mount manager.
5. **Hostname:** Live hostname modification (`/etc/hostname`).
6. **Date & Time:** Real-time clock inspection and manual adjustment.
7. **Terminal Theme:** Switch between color palettes (Default Cyan, Monokai Green, Windows Azure, Amber CRT).
8. **Automated Diagnostics:** Trigger comprehensive 8-point system health check.
9. **Power Controls:** Reboot or halt system.

---

## 4. Built-in Micro Arcade (`arcade`)

MicroOS includes 8 terminal arcade games written in C with pure ANSI rendering:
- `snake` — Classic Snake with real-time arrow/WASD movement, dynamic food, and collision detection.
- `tetris` — Full Tetris with rotation, line clears, and score acceleration.
- `pong` — Two-player / Single-player vs AI paddle ball table tennis.
- `mines` — Grid-based Minesweeper with cursor navigation and flagging.
- `2048` — 4-directional number merge sliding puzzle.
- `guess` — Interactive number guessing with difficulty modes.
- `tictactoe` — Tic-Tac-Toe with an unbeatable Minimax computer opponent or 2-player mode.
- `spaceinvaders` — Real-time ASCII space shooter.

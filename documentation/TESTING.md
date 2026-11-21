# MicroOS Testing & Quality Assurance Framework

## 1. Quality Assurance Directives

MicroOS testing enforces strict, non-negotiable standards:
- **Oracle VirtualBox Exclusively**: All virtual machine verification must execute inside Oracle VirtualBox.
- **Strict Prohibition of QEMU**: Under no circumstances is QEMU or QEMU-based hypervisors permitted.
- **Size Boundary**: The generated `release/microos.iso` must not exceed 50,000,000 bytes.
- **Empirical Validation**: No feature, command, game, or driver may be marked passed without explicit execution logs and verified exit codes.

---

## 2. VirtualBox Test Specification

- **VM Name**: `MicroOS-Test-VM`
- **Memory (RAM)**: 1024 MB
- **Processor**: 1 virtual CPU
- **Firmware**: BIOS mode and UEFI mode dual-validation
- **Storage**:
  - IDE/SATA Primary: `release/microos.iso`
  - SATA Secondary: 2 GB dynamically expanding test virtual disk (`tests/test_disk.vdi`)
- **Network Adapter**: Disabled (`--nic1 none`) for offline validation suite.

---

## 3. Test Suites

### Suite A: Static Integrity & Size Check
1. ISO file existence check.
2. Exact byte size check (`<= 50,000,000` bytes).
3. SHA256 checksum generation and integrity audit.
4. ISO structure verification via `xorriso` / `isoinfo`.
5. Kernel and initramfs unpack verification.

### Suite B: Boot & Init Validation
1. GRUB bootloader prompt & countdown.
2. Kernel decompression and handover to `/init`.
3. `/proc`, `/sys`, `/dev` mounting without errors.
4. BusyBox init transition to runlevel.
5. MicroOS shell prompt display.

### Suite C: CLI & Utilities Validation
1. Execution of `sysinfo`, `hardware`, `disks`, `wifi`, `help`.
2. Safe mounting and unmounting of virtual disks.
3. Verification that non-destructive policies are upheld.

### Suite D: Micro Arcade Validation
1. Verification of all 8 terminal game binaries:
   - `snake`
   - `tetris`
   - `pong`
   - `mines`
   - `2048`
   - `guess`
   - `tictactoe`
   - `spaceinvaders`
2. Menu launcher `arcade` navigation and clean exit.

### Suite E: Network, Subsystems & UI Validation (v2.0)
1. Execution of `tests/run_v2_tests.sh`:
   - `help` general reference and all subcommands (`help wifi`, `help net`, etc.)
   - Unknown command reporting
   - `net status`, `net interfaces`, `net ip`, `net dns`, `net route`
   - `wifi status`, `wifi info`, `wifi help`
   - `ping --help` and offline loopback / gateway handling
   - `dashboard` Control Center navigation and clean exit
   - `settings` System Preferences navigation and clean exit
   - `diagnostics` automated 8-point system test suite
2. VirtualBox v2.0 boot test via `tests/vbox_v2_qa.ps1`:
   - Verified headless boot with 1024 MB RAM, 1 CPU, offline mode
   - Guest serial output audited: Windows/macOS banner -> MicroOS ash shell -> `micro@micro / $ ` prompt
   - Zero QEMU usage strictly enforced

---

## 4. Test Logs
All testing outputs are written to `logs/test.log` with timestamped execution traces.

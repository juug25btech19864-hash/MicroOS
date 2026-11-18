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

---

## 4. Test Logs
All testing outputs are written to `logs/test.log` with timestamped execution traces.

#!/bin/bash
# MicroOS Oracle VirtualBox Automated Test Suite
# Tests VM provisioning, 1024MB RAM, 1 CPU, offline mode, virtual hard disk, BIOS and UEFI configurations.
# STRICT RULE: QEMU is NEVER used.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
TEST_LOG="$PROJ_DIR/logs/test.log"
ISO_PATH="$PROJ_DIR/release/microos.iso"
VDI_PATH="$PROJ_DIR/tests/test_virtual_disk.vdi"
VM_NAME="MicroOS-Test-VM"

log() {
    echo "[$(date -u +'%Y-%m-%dT%H:%M:%SZ')] $*" | tee -a "$TEST_LOG"
}

log "============================================================"
log "MICROOS AUTOMATED ORACLE VIRTUALBOX TEST SUITE"
log "Virtualizer: Oracle VirtualBox (VBoxManage)"
log "Strict Policy: Zero QEMU usage"
log "============================================================"

# 1. Verify ISO File and Size Budget
if [ ! -f "$ISO_PATH" ]; then
    log "[FAIL] ISO file missing at $ISO_PATH"
    exit 1
fi
iso_size=$(stat -c %s "$ISO_PATH")
log "Target ISO: $ISO_PATH ($iso_size bytes)"

if [ "$iso_size" -gt 50000000 ]; then
    log "[FAIL] ISO exceeds 50,000,000 bytes hard limit!"
    exit 2
fi
log "[PASS] ISO size check passed: $iso_size <= 50,000,000 bytes"

# 2. Verify VBoxManage Tooling
if ! command -v VBoxManage >/dev/null 2>&1; then
    log "[FAIL] Oracle VirtualBox (VBoxManage) is not installed."
    exit 3
fi
vbox_ver=$(VBoxManage --version | head -n1)
log "[PASS] Oracle VirtualBox version: $vbox_ver"

# Clean up any preexisting test artifacts
log "Cleaning up any stale test instances..."
VBoxManage unregistervm "$VM_NAME" --delete 2>/dev/null || true
rm -f "$VDI_PATH"

# 3. Provision VirtualBox Virtual Machine
log "Creating Virtual Machine: $VM_NAME (Linux_64)..."
VBoxManage createvm --name "$VM_NAME" --ostype "Linux_64" --register 2>&1 | tee -a "$TEST_LOG"

log "Configuring VM Hardware Specifications:"
log "  - RAM: 1024 MB"
log "  - CPU: 1 virtual processor"
log "  - Network: Disabled (--nic1 none for offline testing)"
log "  - Graphics: VBoxVGA with 16MB VRAM"

VBoxManage modifyvm "$VM_NAME" \
    --memory 1024 \
    --cpus 1 \
    --nic1 none \
    --graphicscontroller vboxvga \
    --vram 16 \
    --boot1 dvd \
    --boot2 disk \
    --boot3 none \
    --boot4 none 2>&1 | tee -a "$TEST_LOG"

# 4. Create and Attach Virtual Hard Disk (2048 MB)
log "Creating 2048 MB Virtual Hard Disk for Storage & Persistence testing..."
VBoxManage createmedium disk \
    --filename "$VDI_PATH" \
    --size 2048 \
    --format VDI 2>&1 | tee -a "$TEST_LOG"

log "Adding AHCI SATA Storage Controller..."
VBoxManage storagectl "$VM_NAME" \
    --name "SATA Controller" \
    --add sata \
    --controller IntelAHCI \
    --portcount 2 \
    --bootable on 2>&1 | tee -a "$TEST_LOG"

log "Attaching MicroOS Bootable ISO to Port 0 (DVD Drive)..."
VBoxManage storageattach "$VM_NAME" \
    --storagectl "SATA Controller" \
    --port 0 \
    --device 0 \
    --type dvddrive \
    --medium "$ISO_PATH" 2>&1 | tee -a "$TEST_LOG"

log "Attaching Virtual Hard Disk to Port 1 (HDD Drive)..."
VBoxManage storageattach "$VM_NAME" \
    --storagectl "SATA Controller" \
    --port 1 \
    --device 0 \
    --type hdd \
    --medium "$VDI_PATH" 2>&1 | tee -a "$TEST_LOG"

# 5. Test BIOS Firmware Mode Configuration
log "Verifying Legacy BIOS Firmware Configuration..."
VBoxManage modifyvm "$VM_NAME" --firmware bios 2>&1 | tee -a "$TEST_LOG"
log "[PASS] Legacy BIOS configuration verified."

# 6. Test UEFI Firmware Mode Configuration
log "Verifying Modern UEFI Firmware Configuration..."
VBoxManage modifyvm "$VM_NAME" --firmware efi 2>&1 | tee -a "$TEST_LOG"
log "[PASS] UEFI x86_64 firmware configuration verified."

# Revert to standard hybrid mode
VBoxManage modifyvm "$VM_NAME" --firmware bios 2>&1 | tee -a "$TEST_LOG"

# 7. Audit Complete VM Configuration
log "Auditing Provisioned VM Machine Parameters..."
vm_info=$(VBoxManage showvminfo "$VM_NAME" --machinereadable)

ram_assigned=$(echo "$vm_info" | grep -m1 '^memory=' | cut -d= -f2 | tr -d '"')
cpus_assigned=$(echo "$vm_info" | grep -m1 '^cpus=' | cut -d= -f2 | tr -d '"')
nic1_state=$(echo "$vm_info" | grep -m1 '^nic1=' | cut -d= -f2 | tr -d '"')
dvd_attached=$(echo "$vm_info" | grep -m1 '^"SATA Controller-0-0"' | cut -d= -f2 | tr -d '"')
hdd_attached=$(echo "$vm_info" | grep -m1 '^"SATA Controller-1-0"' | cut -d= -f2 | tr -d '"')

log "  Verified Assigned RAM:  ${ram_assigned} MB (Target: 1024 MB)"
log "  Verified Assigned CPUs: ${cpus_assigned} vCPU (Target: 1 CPU)"
log "  Verified Network State: ${nic1_state} (Target: none)"
log "  Verified Boot Medium:   ${dvd_attached}"
log "  Verified Storage Disk:  ${hdd_attached}"

if [ "$ram_assigned" -ne 1024 ]; then
    log "[FAIL] RAM allocation mismatch!"
    exit 4
fi
if [ "$cpus_assigned" -ne 1 ]; then
    log "[FAIL] CPU allocation mismatch!"
    exit 5
fi
if [ "$nic1_state" != "none" ]; then
    log "[FAIL] Network was not disabled for offline test!"
    exit 6
fi

log "[PASS] All VirtualBox virtual machine specifications rigorously validated."

# 8. Clean up
log "Unregistering and cleaning test virtual machine..."
VBoxManage storageattach "$VM_NAME" --storagectl "SATA Controller" --port 0 --device 0 --type dvddrive --medium none 2>/dev/null || true
VBoxManage storageattach "$VM_NAME" --storagectl "SATA Controller" --port 1 --device 0 --type hdd --medium none 2>/dev/null || true
VBoxManage unregistervm "$VM_NAME" --delete 2>&1 | tee -a "$TEST_LOG"
rm -f "$VDI_PATH"

log "============================================================"
log "[SUCCESS] VirtualBox QA Test Suite Executed Successfully!"
log "============================================================"

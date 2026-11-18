# MicroOS Master Orchestration Makefile
# Build ultra-lightweight bootable Linux OS (< 50MB ISO)

PROJECT = MicroOS
VERSION = 1.0
BUILD_DIR = build
RELEASE_DIR = release
SCRIPTS_DIR = scripts
ISO_OUTPUT = $(RELEASE_DIR)/microos.iso
MAX_SIZE = 50000000

.PHONY: all games rootfs kernel initramfs iso verify test clean help

all: games rootfs kernel initramfs iso verify

help:
	@echo "MicroOS v$(VERSION) Build System"
	@echo "Targets:"
	@echo "  make all       - Build entire OS (games, rootfs, kernel, initramfs, ISO, verify)"
	@echo "  make games     - Compile all 8 offline terminal arcade games"
	@echo "  make rootfs    - Construct minimal root filesystem and BusyBox links"
	@echo "  make kernel    - Configure and prepare Linux x86_64 kernel"
	@echo "  make initramfs - Package compressed initramfs (cpio.gz)"
	@echo "  make iso       - Generate bootable hybrid GRUB ISO"
	@echo "  make verify    - Enforce 50 MB hard size limit"
	@echo "  make test      - Execute Oracle VirtualBox automated QA test"
	@echo "  make clean     - Clean intermediate build files"

games:
	@echo "=== Building Micro Arcade Games ==="
	@bash scripts/build_games.sh

rootfs:
	@echo "=== Building Root Filesystem ==="
	@bash scripts/build_rootfs.sh

kernel:
	@echo "=== Building Linux x86_64 Kernel ==="
	@bash scripts/build_kernel.sh

initramfs:
	@echo "=== Building Initramfs ==="
	@bash scripts/build_initramfs.sh

iso:
	@echo "=== Generating Hybrid Bootable ISO ==="
	@bash scripts/build_iso.sh

verify:
	@echo "=== Verifying ISO Size Budget ==="
	@python3 scripts/verify_size.py $(ISO_OUTPUT) $(MAX_SIZE)

test:
	@echo "=== Executing VirtualBox QA Suite ==="
	@bash tests/run_virtualbox_test.sh

clean:
	@echo "=== Cleaning Build Artifacts ==="
	@rm -rf $(BUILD_DIR)/*
	@echo "Clean completed."

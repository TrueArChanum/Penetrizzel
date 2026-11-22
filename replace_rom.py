#!/usr/bin/env python3

# WARNING:
# Use at ur own risk. The author takes no responsibility for
# data loss, corrupted disk images, or any damage caused by using
# these scripts. Always keep backups of ur original images.

import os
import sys
import stat

# ============================================================
# Penetrizzel - Genesis ROM Replacement
#
# Usage:
#   python replace_rom.py disk.img ROM.bin
#   python replace_rom.py uefi.img ROM.bin
#
# ALWAYS writes exactly 6 MiB.
# ============================================================

SECTOR_SIZE = 512

ROM_START_MIB = 77
ROM_END_MIB = 83

ROM_START_LBA = ROM_START_MIB * 2048
ROM_REGION_SECTORS = (ROM_END_MIB - ROM_START_MIB) * 2048

ROM_OFFSET = ROM_START_LBA * SECTOR_SIZE
ROM_CAPACITY = ROM_REGION_SECTORS * SECTOR_SIZE

ALLOWED_IMAGES = ("disk.img", "uefi.img")


def error(message):
    print(f"\n[ERROR] {message}")
    sys.exit(1)


# ------------------------------------------------------------
# Arguments
# ------------------------------------------------------------

if len(sys.argv) != 3:
    error("Usage: python replace_rom.py <disk.img|uefi.img> ROM.bin")

image_name = sys.argv[1]
rom_name = sys.argv[2]

if image_name not in ALLOWED_IMAGES:
    error("Only disk.img or uefi.img are allowed!")

# ------------------------------------------------------------
# Validate ROM
# ------------------------------------------------------------

try:
    rom_stat = os.lstat(rom_name)

    if not stat.S_ISREG(rom_stat.st_mode):
        error("ROM must be a regular file.")

    with open(rom_name, "rb") as f:
        rom_data = f.read(ROM_CAPACITY + 1)

except OSError as exc:
    error(f"Cannot read ROM: {exc}")

rom_size = len(rom_data)

if rom_size == 0 or rom_size > ROM_CAPACITY:
    error("Invalid ROM size. Maximum is 6 MiB.")

if rom_size % 2:
    error("Genesis ROM size must be even.")

# ------------------------------------------------------------
# Create exactly 6 MiB padded ROM
# ------------------------------------------------------------

padded_rom = rom_data + bytes(ROM_CAPACITY - rom_size)

assert len(padded_rom) == ROM_CAPACITY

# ------------------------------------------------------------
# Safely open image
# ------------------------------------------------------------

try:
    image_lstat = os.lstat(image_name)

    if not stat.S_ISREG(image_lstat.st_mode):
        error("Image must be a regular file.")

    if image_lstat.st_nlink != 1:
        error("Hard-linked images are forbidden.")

    if image_lstat.st_size < ROM_OFFSET + ROM_CAPACITY:
        error("Image is too small.")

    if os.path.islink(image_name):
        error("Symbolic links are forbidden.")

    flags = os.O_RDWR
    flags |= getattr(os, "O_NOFOLLOW", 0)
    flags |= getattr(os, "O_BINARY", 0)
    flags |= getattr(os, "O_CLOEXEC", 0)

    image_fd = os.open(image_name, flags)

except OSError as exc:
    error(f"Cannot safely open image: {exc}")

try:

    image_stat = os.fstat(image_fd)

    if not stat.S_ISREG(image_stat.st_mode):
        error("Opened target is not a regular file.")

    if image_stat.st_nlink != 1:
        error("Hard-linked image rejected.")

    if image_stat.st_size < ROM_OFFSET + ROM_CAPACITY:
        error("Image size changed unexpectedly.")

    # --------------------------------------------------------
    # Information
    # --------------------------------------------------------

    print()
    print("========================================")
    print(" Penetrizzel ROM Replacement")
    print("========================================")

    print(f"[IMAGE] {image_name}")
    print(f"[ROM]   {rom_name}")
    print(f"[SIZE]  {rom_size} bytes")
    print(f"[PAD]   {ROM_CAPACITY} bytes")
    print(f"[LBA]   {ROM_START_LBA}")
    print(f"[OFFSET] {ROM_OFFSET}")

    print()
    print("The entire 6 MiB ROM region will be replaced.")
    print("Unused ROM space will be zero-filled.")

    confirm = input("\nContinue? [y/N] ")

    if confirm not in ("y", "Y"):
        print("[ABORT] Nothing changed.")
        sys.exit(0)

    # --------------------------------------------------------
    # Write exactly 6 MiB
    # --------------------------------------------------------

    print("\n[WRITE] Writing padded ROM...")

    os.lseek(image_fd, ROM_OFFSET, os.SEEK_SET)

    written = 0

    while written < ROM_CAPACITY:

        count = os.write(
            image_fd,
            padded_rom[written:]
        )

        if count <= 0:
            error("Write failed.")

        written += count

    os.fsync(image_fd)

    # --------------------------------------------------------
    # Verify entire region
    # --------------------------------------------------------

    print("[VERIFY] Checking complete ROM region...")

    os.lseek(image_fd, ROM_OFFSET, os.SEEK_SET)

    verified = bytearray()

    while len(verified) < ROM_CAPACITY:

        chunk = os.read(
            image_fd,
            min(1024 * 1024, ROM_CAPACITY - len(verified))
        )

        if not chunk:
            error("Unexpected end of image during verification.")

        verified.extend(chunk)

    if bytes(verified) != padded_rom:
        error("ROM verification failed!")

    print("[OK] Entire 6 MiB ROM region verified!")

    print()
    print("========================================")
    print(" Penetrizzel ROM replacement complete")
    print("========================================")

finally:
    os.close(image_fd)

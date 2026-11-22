#!/bin/bash
set -euo pipefail

# WARNING:
# Use at ur own risk. The author takes no responsibility for
# data loss, corrupted disk images, or any damage caused by using
# these scripts. Always keep backups of ur original images.

# ============================================================
# Penetrizzel - Genesis ROM Replacement
#
# Usage:
#   ./replace_rom.sh disk.img ROM.bin
#   ./replace_rom.sh uefi.img ROM.bin
#
# ALWAYS writes exactly 6 MiB.
# ============================================================

IMAGE="${1:-}"
ROM="${2:-}"

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

ROM_START_MIB=77
ROM_END_MIB=83

SECTOR_SIZE=512

ROM_START_LBA=$((ROM_START_MIB * 2048))
ROM_REGION_SECTORS=$(((ROM_END_MIB - ROM_START_MIB) * 2048))

ROM_OFFSET=$((ROM_START_LBA * SECTOR_SIZE))
ROM_CAPACITY=$((ROM_REGION_SECTORS * SECTOR_SIZE))

# ------------------------------------------------------------
# Validate target
# ------------------------------------------------------------

if [[ "$IMAGE" != "disk.img" && "$IMAGE" != "uefi.img" ]]; then
    echo "[ERROR] Only disk.img or uefi.img are allowed!"
    exit 1
fi

if [[ ! -f "$IMAGE" || -L "$IMAGE" ]]; then
    echo "[ERROR] Image missing, invalid, or symbolic link."
    exit 1
fi

if [[ "$(stat -c '%h' "$IMAGE")" -ne 1 ]]; then
    echo "[ERROR] Hard-linked images are forbidden."
    exit 1
fi

if [[ ! -f "$ROM" || -L "$ROM" ]]; then
    echo "[ERROR] ROM missing or invalid."
    exit 1
fi

# ------------------------------------------------------------
# Validate ROM
# ------------------------------------------------------------

ROM_SIZE=$(stat -c%s "$ROM")

if (( ROM_SIZE <= 0 || ROM_SIZE > ROM_CAPACITY )); then
    echo "[ERROR] Invalid ROM size."
    exit 1
fi

if (( ROM_SIZE % 2 != 0 )); then
    echo "[ERROR] Genesis ROM size must be even."
    exit 1
fi

IMAGE_SIZE=$(stat -c%s "$IMAGE")

if (( IMAGE_SIZE < ROM_OFFSET + ROM_CAPACITY )); then
    echo "[ERROR] Image too small."
    exit 1
fi

# ------------------------------------------------------------
# Create padded temporary ROM
# ------------------------------------------------------------

TEMP_ROM=$(mktemp)

trap 'rm -f "$TEMP_ROM"' EXIT

truncate -s "$ROM_CAPACITY" "$TEMP_ROM"

dd if="$ROM" \
   of="$TEMP_ROM" \
   bs=1M \
   conv=notrunc \
   status=none

PADDED_SIZE=$(stat -c%s "$TEMP_ROM")

if (( PADDED_SIZE != ROM_CAPACITY )); then
    echo "[ERROR] Temporary ROM has incorrect size."
    exit 1
fi

# ------------------------------------------------------------
# Information
# ------------------------------------------------------------

echo
echo "======================================"
echo " Penetrizzel ROM Replacement"
echo "======================================"

echo "[IMAGE] $IMAGE"
echo "[ROM]   $ROM"
echo "[SIZE]  $ROM_SIZE bytes"
echo "[PAD]   $ROM_CAPACITY bytes"
echo "[LBA]   $ROM_START_LBA"
echo "[OFFSET] $ROM_OFFSET"

echo
echo "The entire 6 MiB ROM region will be replaced."
echo "Unused ROM space will be zero-filled."

read -r -p "Continue? [y/N] " CONFIRM

if [[ "$CONFIRM" != "y" && "$CONFIRM" != "Y" ]]; then
    echo "[ABORT]"
    exit 0
fi

# ------------------------------------------------------------
# Write exactly 6 MiB
# ------------------------------------------------------------

echo
echo "[WRITE] Writing padded ROM..."

dd if="$TEMP_ROM" \
   of="$IMAGE" \
   bs=512 \
   seek="$ROM_START_LBA" \
   count="$ROM_REGION_SECTORS" \
   conv=notrunc \
   status=progress

# ------------------------------------------------------------
# Verify entire 6 MiB
# ------------------------------------------------------------

echo
echo "[VERIFY] Checking complete ROM region..."

dd if="$IMAGE" \
   bs=512 \
   skip="$ROM_START_LBA" \
   count="$ROM_REGION_SECTORS" \
   status=none |
cmp - "$TEMP_ROM"

echo "[OK] Entire 6 MiB ROM region verified!"

echo
echo "======================================"
echo " Penetrizzel ROM replacement complete"
echo "======================================"

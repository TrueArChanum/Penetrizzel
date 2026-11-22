
# ⛧ Penetrizzel — Installation Guide

This document explains how to install and run Penetrizzel using a physical USB storage device.

Both Legacy BIOS and UEFI boot modes are supported.

**IMPORTANT:** Penetrizzel uses Flat-Address Storage (FAS). The USB device is not simply a read-only boot medium. Persistent game data is written to the boot device.

**Back up your data before proceeding\!**

---

## 📦 Available Images

Only ONE image is required.

| Image | Boot mode | Size |
| --- | --- | --- |
| `disk.img` | Legacy BIOS / CSM | 84 MiB |
| `uefi.img` | UEFI | 84 MiB |

Choose the image matching your computer's firmware configuration.

---

## ⚠️ Important Safety Information

Writing an image to a USB storage device overwrites existing data.

Penetrizzel also uses the boot device for persistent storage.

Existing data in the relevant storage area may be overwritten.

**Never use a USB stick containing valuable data without making a backup.**

The installation commands below operate on the entire USB device, not an individual partition.

For example:

- `/dev/sdb` = entire USB device.
- `/dev/sdb1` = first partition.

Use the entire device.

---

## 🐧 Linux Installation

### 1\. Identify your USB device

Connect your USB stick and execute:

bash

```bash
lsblk -o NAME,SIZE,MODEL,TRAN,MOUNTPOINTS
```

Example:

text

```text
NAME   SIZE MODEL           TRAN MOUNTPOINTS
sda    1.0T Samsung SSD     sata
├─sda1 512M
└─sda2 999G
sdb   16.0G SanDisk USB     usb
└─sdb1 16.0G
```

In this example, the USB device is `/dev/sdb`.

**Your device name will probably be different\!**

Never blindly copy `/dev/sdb` from this example.

### 2\. Unmount the USB partitions

Replace `sdX` with your actual device identifier.

bash

```bash
sudo umount /dev/sdX?*
```

If no partitions are mounted, this step may not be necessary.

### 3\. Optional: Clear the first 90 MiB

Penetrizzel uses approximately 84 MiB of storage.

If you want to explicitly clear the first 90 MiB before installation:

bash

```bash
sudo dd if=/dev/zero of=/dev/sdX bs=1M count=90 status=progress conv=fsync
```

**Explanation:**

- `if=/dev/zero`: Source of zero bytes.
- `of=/dev/sdX`: Target USB device.
- `bs=1M`: Block size of 1 MiB.
- `count=90`: Write 90 blocks.
- `status=progress`: Display progress.
- `conv=fsync`: Flush output before completion.

This operation clears the first 90 MiB (94,371,840 bytes).

**This step is optional.** Writing the complete image already overwrites the corresponding area.

### 4\. Write the boot image

Choose the appropriate command.

#### Legacy BIOS / CSM

bash

```bash
sudo dd if=disk.img of=/dev/sdX bs=4M status=progress conv=fsync
```

#### UEFI

bash

```bash
sudo dd if=uefi.img of=/dev/sdX bs=4M status=progress conv=fsync
```

Replace `/dev/sdX` with your actual USB device.

Wait until `dd` finishes.

Do not disconnect the USB stick while writing.

### 5\. Boot Penetrizzel

1. Safely disconnect the USB device.
2. Insert it into the target computer.
3. Enter the computer's boot menu.
4. Select the USB storage device.

For `disk.img`, enable Legacy BIOS / CSM boot if necessary.

For `uefi.img`, use UEFI boot mode.

Enjoy Penetrizzel\! ⛧

---

## 🪟 Windows 11 Installation

The easiest method is using balenaEtcher.

Download:

https://etcher.balena.io/

### 1\. Prepare the USB device

Connect your USB stick.

**WARNING:** All existing data on the selected target device may be destroyed.

### 2\. Select the image

Open balenaEtcher.

Choose one of the following:

- `disk.img` — Legacy BIOS.
- `uefi.img` — UEFI.

### 3\. Select the target device

Carefully select your USB storage device.

Verify its model and capacity before proceeding.

### 4\. Flash the image

Click **Flash\!**

Wait until the writing and verification process has completed.

### 5\. Boot

Safely eject the USB device.

Insert it into the target computer and select it through the firmware boot menu.

**Note:** Windows may display a message asking you to format the USB device after writing the image. Do NOT format it. This is expected when Windows encounters an unfamiliar disk layout.

---

## 🖥️ Running Penetrizzel without a physical USB stick

You can also run Penetrizzel through QEMU.

See QEMU.md for the recommended BIOS and UEFI configurations.

---

## 🔐 Persistent Storage & Game Saves

Penetrizzel uses Flat-Address Storage (FAS).

The boot device also stores persistent game data, including:

- Hyper-Wurm high scores.
- SEGA Genesis game data and SRAM.
- Other persistent application data.

Persistent data is written during:

- Reboot.
- Shutdown.
- F2 Kernel Soft-reset.

**IMPORTANT:** This is not a harmless read-only boot image.

Please back up your USB device regularly.

---

⛧ Best regards,

ArChanum


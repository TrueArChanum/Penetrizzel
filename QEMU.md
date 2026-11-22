# ⛧ Penetrizzel — QEMU Configuration

Penetrizzel can run inside QEMU without requiring a physical USB stick.

Both Legacy BIOS and UEFI configurations are supported.

The supplied disk images are used as virtual USB mass-storage devices.

---

## Requirements

- QEMU.
- SDL display support.
- OpenGL support (for `gl=on`).
- Audio backend support.
- OVMF firmware for UEFI testing.

On Arch Linux:

bash

```bash
sudo pacman -S qemu-full edk2-ovmf
```

Check the QEMU executables:

bash

```bash
qemu-system-i386 --version
qemu-system-x86_64 --version
```

Check the OVMF firmware location:

bash

```bash
ls /usr/share/edk2/x64/OVMF.4m.fd
```

Firmware paths may differ between distributions.

---

## 🖥️ Legacy BIOS

Start Penetrizzel using the Legacy BIOS image:

bash

```bash
qemu-system-i386 \
    -cpu max \
    -m 128M \
    -display sdl,gl=on \
    -audiodev sdl,id=snd0 \
    -machine pcspk-audiodev=snd0 \
    -audiodev pa,id=audio0 \
    -device ich9-intel-hda \
    -device hda-duplex,audiodev=audio0 \
    -device qemu-xhci,id=xhci \
    -drive if=none,id=usbdisk,format=raw,file=disk.img \
    -device usb-storage,bus=xhci.0,drive=usbdisk,bootindex=1
```

### Configuration

| Parameter | Value |
| --- | --- |
| Architecture | x86 / i386 |
| CPU | max |
| RAM | 128 MiB |
| Firmware | Legacy BIOS |
| Display | SDL + OpenGL |
| USB controller | xHCI |
| Storage | USB mass storage |
| Audio | PC Beeper + HDA |

---

## 🖥️ UEFI

Start Penetrizzel using the UEFI image:

bash

```bash
qemu-system-x86_64 \
    -machine q35,pcspk-audiodev=snd0 \
    -cpu max,+mtrr \
    -m 256M \
    -display sdl,gl=on \
    -audiodev sdl,id=snd0 \
    -audiodev pa,id=audio0 \
    -device ich9-intel-hda \
    -device hda-duplex,audiodev=audio0 \
    -bios /usr/share/edk2/x64/OVMF.4m.fd \
    -fw_cfg name=opt/ovmf/X-PciMmio64Mb,string=0 \
    -device qemu-xhci,id=xhci \
    -drive if=none,id=usbdisk,format=raw,file=uefi.img \
    -device usb-storage,bus=xhci.0,drive=usbdisk,bootindex=1
```

### Configuration

| Parameter | Value |
| --- | --- |
| Architecture | x86-64 |
| CPU | max, +mtrr |
| RAM | 256 MiB |
| Firmware | OVMF / UEFI |
| Machine | Q35 |
| Display | SDL + OpenGL |
| USB controller | xHCI |
| Storage | USB mass storage |
| Audio | PC Beeper + HDA |

---

## 🔊 Audio

The supplied configurations use two audio backends:

- SDL for the PC Beeper.
- PulseAudio (`pa`) for HDA audio.

Depending on your Linux distribution and installed audio infrastructure, you may need to adjust the audio backend.

The UEFI configuration currently has a known limitation: the PC Beeper fallback does not seem to work reliably in QEMU.

---

## ⚠️ Known QEMU Issues

QEMU has some BUGs, especially with UEFI, which I haven't encountered on real hardware.

### Boot hangs

Sometimes QEMU hangs at the red boot splash screen.

**Workaround:** Simply execute the QEMU command again.

### PC Beeper

UEFI + QEMU does not seem to support the PC Beeper aka Earbleeder fallback audio backend reliably.

This might be a BUG. xD

### HDA audio

When switching applications quickly and frequently, HDA audio sometimes dies and falls back to the PC Beeper or produces no sound.

---

## 💾 Persistent Storage

QEMU directly uses the supplied image file as its virtual USB storage device.

Penetrizzel's FAS operations and game saves therefore modify the image file on your host computer.

**IMPORTANT:** The virtual disk image is writable.

If you want to preserve the original image, make a backup before running Penetrizzel.

For example:

bash

```bash
cp disk.img disk_backup.img
cp uefi.img uefi_backup.img
```

You can restore the original image by replacing the modified image with its backup.

---

⛧ Best regards,

ArChanum

# ⛧ Penetrizzel

**A "TOY OS" with a slightly unhealthy obsession with USB sticks.**

---

## 💾 What is Penetrizzel?

Penetrizzel is a self-contained, retro-inspired toy operating system for developing and running games and applications, including SEGA Genesis games.

Written in freestanding C99 and Assembly (NASM), without relying on external frameworks or dependencies such as Godot, Unity, Unreal Engine, SDL3, or Raylib.

Inspired by the era of MS-DOS 💾 floppy disk games, but modernized around USB sticks.

Simply boot from a USB stick (or use QEMU instead). No hard drive (HDD or SSD) access is implemented or supported by the OS itself.

The idea is simple:

**Less bloat. Less waiting. More control.**

And hopefully fewer gray hairs.

---

## 📑 Table of Contents

- What is Penetrizzel?
- Why Penetrizzel?
- Features
- SEGA Genesis Emulation
- Performance & Architecture
- Download & Installation
- Security & Privacy
- Known Limitations & Bugs
- Videos & Demonstrations
- Screenshots
- About Me
- Support the Development

---

## Why Penetrizzel?

Because I despise stuff like "2 Faktor Auth" with a passion.

"Modern" PC security seems to head in a rather questionable direction too, IMHO. More authentication layers, more dependencies, more inconvenience for the USER.

And whether all this actually makes things safer in practice? Well, "no"... if u believe the news, which I don't; so I guess we don't know for sure...

I want to explore alternative ideas.

**Future plans, maybe...**

For example, instead of relying exclusively on digital authentication mechanisms, why not use physical USB sticks as keys?

Imagine separating ur data into different physical devices:

| Device | Purpose |
| --- | --- |
| Boot Stick | Contains the operating system (currently also the games). |
| Vault Stick | Contains valuable data. |
| App Stick | Contains applications, games n' stuff. |
| Key Stick | Could contain a cryptographic key. Without it, data on the Tresor Stick remains unreadable. |
| Personal Data Stick | Contains encrypted personal user data. Cannot be read without the Key Stick; could be used for online stuff (authentication, age verification, etc.). |

And we could also mess around with the TPM 2.0 chip.

It's my hardware, I paid for it, so I wanna use this damn chip too\!

The basic idea: Instead of real physical keys, we use USB sticks to unlock stuff. No other security layer needed (hopefully).

**NOTE:** These are exploratory ideas of a madman, NOT currently implemented security guarantees.

---

## 🖥️ Features

- x86 32-bit protected mode.
- Freestanding C99 + Assembly (NASM).
- Legacy BIOS boot (CSM / Legacy Boot enabled).
- UEFI 64-bit boot support.
- No paging.
- No multithreading / SMP.
- VGA MODE-X, 320 × 240, 8-bit framebuffer.
- Four memory planes and page flipping.
- PS/2 keyboard support via IRQ1.
- Basic PCI support.
- xHCI / USB support (Mouse, Keyboard, Mass Storage).
- PIT timing.
- PC Beeper (AKA Earbleeder) sound.
- HDA audio support.
- Flat-Address Storage (FAS), designed for USB sticks.
- Custom executable format: PNO (Penetrizzel Native Object).
- SIMD software rendering (AVX2, SSE2, or scalar fallback).
- CRT effects and custom rendering effects.
- Built-in application management.
- Automatic game save storage.

### Included applications

| Application | Description |
| --- | --- |
| Hyper-Wurm | 🪱 Arcade game |
| Twilight Chess | ♟️ Chess |
| clownMDEmu | 🤡 SEGA Genesis emulator |

PNO applications are launched through the console using the `RUN` command.

Examples:

text

```text
RUN CHESS
RUN CLOWN
RUN WURM
```

**PNO = Penetrizzel Native Object.**

Pronunciation: `[piˈno]`

The PNO format is still relatively new. A lot of things are hardcoded right now, so u cannot (yet) simply add ur own custom `.pno` applications and launch them.

---

## 🎮 SEGA Genesis Emulation

Penetrizzel includes an optional PNO implementation of ClownMDEmu, based on Clownacy's awesome emulator.

Features:

- SEGA Genesis / Mega Drive ROM support.
- ROM size up to 6 MiB.
- Game data and SRAM storage through FAS.
- Integrated into Penetrizzel's application manager.

The included ROM is **FAT TOFU**.

Original project / example:

[https://github.com/Clownacy/clownmdsdk/tree/master/example/tofu](<https://github.com/Clownacy/clownmdsdk/tree/master/example/tofu>)

### ⚠️ FAT TOFU workaround

There seems to be a BUG involving FAT TOFU which can cause the emulator to hang.

For the currently distributed version, after executing:

text

```text
RUN CLOWN
```

Switch back to the console using the **F1 key**, then execute:

text

```text
ROM WRITE 2CAC 0000
```

Now switch back to TOFU and play.

Apparently, ROM editing is sometimes part of the gaming experience. xD

### Replacing the Genesis ROM

The uploaded version uses FAT TOFU as its SEGA Genesis ROM.

If u have another ROM in `.bin` format (**NOT** the de-interleaved `.smd` format), u can replace the existing ROM data directly.

Reference / ROM.smd fix:

[https://sfmods.com/threads/shining-force-alternate.252/page-6](<https://sfmods.com/threads/shining-force-alternate.252/page-6>)

**Requirements:**

- Raw `.bin` ROM.
- Maximum size: 6 MiB.
- Write it to the same LBA as the original ROM.

On Arch Linux (btw), use the supplied helper:

bash

```bash
./replace_rom.sh ROM.bin
```

Alternatively:

bash

```bash
python replace_rom.py ROM.bin
```

See the accompanying `README.txt` for additional details.

---

## 🚀 Performance & Architecture

One of the main intentions behind Penetrizzel is to explore how far a 32-bit system can be pushed on modern PCs.

**No GPU support. 💩**

Instead, Penetrizzel uses SIMD software rendering:

- AVX2 🚂
- SSE2 🚚
- Scalar fallback 🐢

The FPS target is adjustable from 1 to 1000.

On "real" hardware (e.g. my Ryzen 9 5900X), it easily hits **1000 FPS at approximately 9% CPU load** for Console / Hyperwurm / Twilight\_Chess.

### Some statistics

**Version 1.37**

#### Lines of Code — Penetrizzel

bash

```bash
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 3 -type f -name "*.h" -exec cat {} + | wc -l
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 3 -type f -name "*.c" -exec cat {} + | wc -l
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 3 -type f -name "*.asm" -exec cat {} + | wc -l
```

| File type | Lines |
| --- | --- |
| `.h` | 7,764 |
| `.c` | 72,173 |
| `.asm` | 5,506 |
| **Total** | **85,443** |

#### Lines of Code — Including clownMDEmu

bash

```bash
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 20 -type f -name "*.h" -exec cat {} + | wc -l
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 20 -type f -name "*.c" -exec cat {} + | wc -l
find ~/Desktop/Penetrizzel/Kernel/ -maxdepth 20 -type f -name "*.asm" -exec cat {} + | wc -l
```

| File type | Lines |
| --- | --- |
| `.h` | 12,267 |
| `.c` | 85,793 |
| `.asm` | 5,506 |
| **Total** | **103,566** |

---

## 📦 Download & Installation

### 📜 Licensing & Development Philosophy

This project is currently developed as proprietary, closed-source software, cause of "entropy" - as soon as i made the decision to "open source" everything, i cannot take it back; that's why i wanna back "small steps" first.

Open-sourcing parts or potentially all of the project remains a possibility for the future.

### Available images

Only ONE of the following images is needed to run Penetrizzel.

| Image | Boot mode | Size |
| --- | --- | --- |
| `disk.img` | Legacy BIOS | 84 MiB |
| `uefi.img` | UEFI | 84 MiB |

### Requirements

- A compatible x86 PC.
- USB storage device for physical booting.
- Legacy BIOS / CSM or UEFI firmware, depending on the image.
- QEMU for virtualized testing.

### Creating a bootable USB stick

Download the appropriate image from the Releases section.

**Linux (Arch Linux):**

bash

```bash
chmod +x build.sh
./build.sh
```

**Windows:**

bat

```bat
build.bat
```

⚠️ **WARNING:** Writing a disk image to a USB stick destroys existing data on the selected target device.

Always verify the target device before proceeding.

**Back up ur data first\!**

Detailed installation instructions: INSTALLATION.md

### QEMU

You can also run Penetrizzel using QEMU.

See QEMU.md for the recommended command line and configuration.

---

## 🔐 Security & Privacy

There is:

- No telemetry.
- No local nor online account.
- No background data collection.
- No HDD/SSD storage driver.
- No network communication.

It's not Windows. xD

_Note: Firmware-level components such as Intel Management Engine and similar technologies are outside the scope of these OS-level claims._

### Storage access

Penetrizzel currently uses the USB storage device from which it boots.

The OS itself does not implement an HDD/SSD driver.

**QEMU exception:** When running the supplied `uefi.img` / `disk.img` through QEMU, the image is backed by a file on ur host computer. Therefore, the host HDD/SSD is indirectly involved in the virtualized setup.

This does not mean Penetrizzel has direct access to ur host filesystem.

### ⚠️ Important: USB storage and game saves

Penetrizzel uses Flat-Address Storage (FAS).

The boot device also stores persistent game data, including:

- Hyper-Wurm high scores.
- Genesis game data / SRAM (e.g. Shining Force savegames).

The OS uses approximately 84 MiB of storage.

U can use the `MEMORY` command to show ur RAM usage.

**I recommend backing up ur USB stick and clearing at least the first 90 MiB before using it.**

Example on Arch Linux:

bash

```bash
sudo dd if=/dev/zero of=/dev/sdX bs=1M count=90 status=progress conv=fsync
```

⚠️ **WARNING:** Always verify which device (e.g. `/dev/sdX`) u are messing up before executing such a command\!

Replace `/dev/sdX` with the correct device identifier. Selecting the wrong device can destroy data.

**IMPORTANT: This is not a harmless read-only boot image.**

The relevant storage area is used for FAS and game data. Existing data will be overwritten\!

Persistent data is written during:

- Reboot.
- Shutdown.
- F2 Kernel Soft-reset.

If u want to prevent the OS from writing persistent data, u can physically remove the USB stick after booting, although this also means that subsequent storage operations will fail.

See SECURITY.md for further details.

---

## ⚠️ Known Limitations & Bugs

There are quite a few... 🙉

### General limitations

- One CPU core only.
- No paging.
- No multithreading.
- No GPU acceleration.
- USB support is not reliable yet.
- xHCI implementation is still rather buggy. SEND HELP\!
- HDA audio support is incomplete.
- Only a limited number of sound controllers are supported.
- PNO is still under development.
- Custom PNO applications cannot yet be added conveniently.
- File reads and Chess AI calculations block the entire CPU.

### QEMU-specific issues

QEMU has some BUGs (especially with UEFI) which I haven't encountered on real hardware.

- Sometimes it doesn't boot and simply hangs at the "red boot splash screen".**FIX:** Just run the QEMU command again.
- UEFI + QEMU does not seem to support the "PC Beeper" aka "Earbleeder" fallback audio backend... but, this might be a BUG. xD
- When switching applications quickly and frequently, HDA audio sometimes dies and falls back to the PC Beeper or produces no sound.

### Real hardware

Things that can go wrong:

- CPU crashes + reboots.
- High CPU load (on one core).U can monitor this via the console command:
  text
  ```text
  STATUS ON
  ```
- Freezes and hangs.
- USB devices not recognized (no mouse, keyboard control, nor storage in this case).
- No HDA sound.
- Various other bugs.

All these issues should be fixed by "turning OFF" the power of the computer, though.

### Data safety

Although Penetrizzel does not implement an HDD/SSD driver, USB storage operations can still overwrite existing data on the boot device.

**Please don't use a USB stick containing valuable data without making a backup.**

---

## 🎥 Videos & Demonstrations

### FAT TOFU

Demonstration of ROM editing to work around a glitch in the SEGA Genesis game TOFU.

Original example:

[https://github.com/Clownacy/clownmdsdk/tree/master/example/tofu](<https://github.com/Clownacy/clownmdsdk/tree/master/example/tofu>)

▶ Watch FAT TOFU demonstration

### Penetrizzel boot performance

Boot-speed and performance testing on my Ryzen 9 5900X.

The FPS target is adjustable from 1 to 1000.

▶ Watch Penetrizzel performance demonstration

---

## 📸 Screenshots

See the `screenshots/` directory for additional screenshots.

---

## 👨‍💻 About Me

For people who care...

I dislike long loading times, unresponsive menus, and 99% of modern EDV "Tools".

MS Word, EXCEL, u name it, I hate it.

I did like MS Paint & Editor though... 🎨

Modern software (especially on the WEB) is ridiculously bloated.

I cannot even understand how people can do web design. Huge respect for the people who suffer through this ... aaand u have my pity too\!

### Where did these bad experiences come from?

I tried to port "Hyperwurm is Back\!" to WASM via Electron.

...It actually worked at some point...

...after pegging the AI on cooldown, that is...

...until the next major update. -.-

Messing around with dozens of dependencies (which seem to change or shift with every update) was among the least fun experiences I've ever had while doing Vibecoding.

Even worse than xHCI.c kernel driver development.

I simply want to revert to simpler software, get less gray hair in the future, and maybe make something useful along the way.

50/50 Chance (4+ 🎲)

For everybody else who embraces all this new stuff: have fun with ur (future) three-factor authentication\! 🖕

Another thing:

I am just a braindead person trying to fix stuff.

But recently, I want to ask CS students, developers, and other people working in the industry:

**What are u actually doing?**

I challenge u\! ⚔️

**Please fix modern software\!**

P.S. I don't like smartphones either.

---

## ☕ Support the Development

If u want to support me, or buy me a Coffee...

Well...

I actually do not drink a lot of Coffee.

More of a Tea guy.

But I would genuinely appreciate it if u bought a valuable copy of my game:

### 🪱 Hyperwurm is back\!

Available on Steam.

Play it, try to enjoy it, and share ur high scores\!

That would mean more to me than a random donation.

Arigatou Gozaimasu (ありがとうございます)\! 🙏

---

⛧ **Best regards,**

**ArChanum**

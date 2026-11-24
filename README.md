# ⛧ Penetrizzel

---

## 💾 What is Penetrizzel?

Penetrizzel is a self-contained, retro-inspired toy operating system for developing and running games and applications, including SEGA Genesis games.

Written in freestanding C99 and Assembly (NASM), without relying on external frameworks or dependencies such as Godot, Unity, Unreal Engine, SDL3, or Raylib.

Inspired by the era of MS-DOS 💾 floppy disk games, but modernized around USB sticks.

Simply boot from a USB stick (or use QEMU instead). No hard drive (HDD or SSD) access is implemented or supported by the OS itself.

The idea is simple:

**Less bloat. Less waiting. More control.**

And hopefully fewer gray hairs, while waiting for MS Outlook to boot for half a minute, only to show some text...

https://github.com/user-attachments/assets/242446cf-a66e-444b-886d-2a7d05680523

---

### Source Code

Curious about what's under the hood?

Take a peek at the [kernel.c](kernel.c) * the heart of Penetrizzel.

---


## 📑 Table of Contents

- [What is Penetrizzel?](#-what-is-penetrizzel)
- [Why Penetrizzel?](#why-penetrizzel)
- [Features](#-features)
- [SEGA Genesis Emulation](#-sega-genesis-emulation)
- [Performance & Architecture](#-performance--architecture)
- [Download & Installation](#-download--installation)
- [Security & Privacy](#-security--privacy)
- [Known Limitations & Bugs](#-known-limitations--bugs)
- [About Me](#-about-me)
- [Support the Development](#-support-the-development)

---

## Why Penetrizzel?

Making even simple Games is rly hard; i want to deliver a platform which makes it easy to develop a simple 2D "retro-like" Game.

More Examples: Not a fan, when i try to shutdown my Computer, but Windows 11 decides to do "do stuff" for 10 more minutes... 
// I rly did like Win 7 - i wish id' come back! xD

🔮 Future Ideas of a madman (Unimplemented)

<details>
<summary>🧠 Brainrot ahead... *click to expand*</summary>

# Rethinking PC Security.

I despise stuff like "Two-factor authentication (2FA)" with a passion. Do not love mandatory local nor Online-Logins 
(squinting to Win11 -.- ) either to access my OS.

"Modern" PC security seems to head in a rather questionable direction too, IMHO. 
More authentication layers, more dependencies, more inconvenience for the USER.

And whether all this actually makes things safer in practice? 
Well, "no"... if u believe the news, which I don't; so I guess we don't know for sure...

Finally I want to explore alternative ideas. // WARNING if u proceed to read! i did not yet took all my pills!

**Future plans, maybe...**

For example, instead of relying exclusively on digital authentication mechanisms, why not use physical 
# USB sticks as keys?

Imagine separating ur data into different physical devices:

| Device | Purpose |
| --- | --- |
| Boot Stick | Contains the operating system (currently also the games). |
| Vault Stick | Contains valuable data. |
| App Stick | Contains applications, games n' stuff. |
| Key Stick | Could contain a cryptographic key. Without it, data on the Tresor Stick remains unreadable. |
| Personal Data Stick | Contains encrypted personal user data. Cannot be read without the Key Stick; could be used for online stuff (authentication, age verification, etc.). |

<img width="800" height="600" alt="USB_banner" src="https://github.com/user-attachments/assets/d903a3d5-325c-4d87-9025-f8678772453b" />

And we could also mess around with the TPM 2.0 chip. // It's my hardware, I paid for it, so I wanna use this damn chip too\!

The basic idea: Instead of real physical keys, we use USB sticks to unlock stuff. No other security layer needed (hopefully).

</details>

<details>
<summary>💭 fever dream...</summary>

# Virtual CPUs (V_CPU) aka CPU emulation.

"V_CPU" - A 64-bit x86 Computer should be able to "execute all Code", no matter the CPU architecture. 

For example:

Create a "Virtual CPU" (short: V_CPU), e.g. M68000 and Z80; together, they could act as a SEGA Genesis emulator.

Create another "Virtual CPU" which could e.g. run Amiga, ARM (JIT?), etc.

Create Trampolines to switch between CPU execution modes: 32-bit (baseline), 16-bit (Real Mode), or 64-bit (with Paging). Each could act as a separate "Virtual CPU".

Questions to ask: Is this possible? And does it even make sense..? xD

</details>

<details>
<summary>🔥 ...this...is Heresy!</summary>

# The Internet should deliver information, not an entire operating environment.

No web browser as a second layer (basically another OS on top of mine). No HTML, no WWW, no HTTPS(e.g. avoid the conventional Web stack), no JavaScript, etc. Just the freestanding C99 OS.

"Websites" only transmit very little "data" – NO ASSETS! (no images, no fonts).

These assets are already stored in the OS itself (e.g. basic fonts). The "website" only stores text strings, locations (where these strings should be displayed on the screen), etc.

We use plain IP addresses (as a starting point) to access a "Penetrizzel-hosted Website".

To access a certain website, we simply enter a COMMAND in the Console (which basically acts as a text-based web browser).

We use TCP/IP.

bash
BROWSE 8.8.8.8        // BROWSE = COMMAND

In the end...

What is a Website..?

...usually?

Text (ok... with fancy CSS; but no1 rly needs that);

sometimes Images;

sometimes embedded videos;

//we could handle these two later.

and 98% of the "other" stuff is not rly "wanted" by "us" users!

</details>

<details>
<summary> 🦾 🤖 > 💥 Max Copium!</summary>

# "True" Agentic OS

Implement a 32-to-64-bit trampoline( = controlled CPU mode transitions), to switch CPU Mode on demand (we need more Memory!); 
then implement a fully Agentic OS to support a local AI (LLM).

# Make no mistakes!    --sandbox danger-full-access

"Access" the GPU command buffers n' stuff (..somehow): Load a local LLM into GPU memory and run it (prompt it via Console).

Hook as many Kernel "functions" as possible to Commands the LLM can execute (to control stuff - complete Chaos? No problem!).

~100% of GPU resources belong to the LLM; we use Software Rendering for all the "other stuff".
The LLM is the operating system's administrator, not its guest.
No artificial permission hierarchy inside Penetrizzel. 
The AI can modify, destroy, restart or completely break its own environment.

Why should an AI running inside an operating system be treated like an app, rather than a first-class system component?

https://github.com/user-attachments/assets/53c9cac3-6f7c-419e-833e-53b37a8182fa

CHAOS IS MY MASTER!
</details>

---

## 🖥️ Features

- x86 32-bit protected mode.
- Freestanding C99 + Assembly (NASM).
- Legacy BIOS boot (CSM / Legacy Boot enabled).
- UEFI 64-bit boot support.
- No paging.
- No multithreading / SMP.
- VGA MODE-X, 320 × 240, 8-bit framebuffer.  (640 x 480 in UEFI)
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

**PNO = Penetrizzel Native Object.**  // Pronunciation: `[piˈno]`

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

<details>
<summary> **Update:** Clownacy has fixed this issue in a newer version. The workaround described below is only relevant to older versions. </summary>

~~ ### 🍔 FAT TOFU workaround ~~

~~ There seems to be a Glitch involving FAT TOFU which can cause the emulator to hang. ~~

~~ For the currently distributed version, after executing: ~~

text

```text
RUN CLOWN
```

~~ Switch back to the console using the **F1 key**, then execute: ~~

text

```text
ROM WRITE 2CAC 0000
```

~~ Now switch back to TOFU and play. ~~

~~ Apparently, ROM editing is sometimes part of the gaming experience. xD ~~

</details>

### 💽 Replacing the Genesis ROM

The uploaded version uses FAT TOFU as its SEGA Genesis ROM.

If u have another ROM in `.bin` format (**NOT** the de-interleaved `.smd` format), u can replace the existing ROM data directly.

Reference to convert a ROM.smd into a ROM.bin:

[https://sfmods.com/threads/shining-force-alternate.252/page-6](<https://sfmods.com/threads/shining-force-alternate.252/page-6>)

**Requirements:**

- Raw `.bin` ROM.
- Maximum size: 6 MiB.
- Write it to the same LBA as the original ROM.

On Arch Linux (btw), use the supplied helper:

bash

```bash
./replace_rom.sh disk.img ROM.bin
```

or

```bash
./replace_rom.sh uefi.img ROM.bin
```

Alternatively:

bash

```bash
python replace_rom.py disk.img ROM.bin
```

or

```bash
python replace_rom.py uefi.img ROM.bin
```

See the accompanying `replace_rom_README.txt` for additional details.

---

## 🚀 Performance & Architecture

One of the main intentions behind Penetrizzel is to explore how far a 32-bit system can be pushed on modern PCs.

**No GPU support. 💩**

Instead, Penetrizzel uses SIMD software rendering:

- AVX2 🚂
- SSE2 🚚
- Scalar fallback 🐢

The FPS target is adjustable from 1 to 1000.

On "real" hardware (e.g. my Ryzen 9 5900X), it easily hits **1000 FPS at approximately ~12% CPU load** for Console / Hyper_Wurm / Twilight\_Chess. clownMDEmu hits around 800 FPS.

### Some statistics

**Version 1.37**

#### Lines of Code * Penetrizzel

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

#### Lines of Code * Including clownMDEmu (via .pno), all custom libs, everything.

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

This project is currently developed as proprietary, closed-source software, cause of "entropy" - as soon as i made the decision to "open source" everything, 
i cannot take it back; that's why i wanna back "small steps" first.

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

Detailed installation instructions: [INSTALLATION.md](INSTALLATION.md)

### QEMU

You can also run Penetrizzel using QEMU.

See [QEMU.md](QEMU.md) for the recommended command line and configuration.

---

## 🔐 Security & Privacy

There is:

- No telemetry.
- No local nor online account.
- No background data collection.
- No HDD/SSD storage driver.
- No network communication.

It's not Windows 11      // Dear Microsoft, pls fix Windows!

_NOTE: Firmware-level components such as Intel Management Engine and similar "spy" technologies are outside the scope of these OS-level claims._

### Storage access

Penetrizzel currently uses the USB storage device from which it boots.

Although Penetrizzel does not implement an HDD/SSD driver, USB storage operations can still overwrite existing data on the boot device.

**QEMU exception:** When running the supplied `uefi.img` / `disk.img` through QEMU, the image is backed by a file on ur host computer. Therefore, the host HDD/SSD is indirectly involved in the virtualized setup.

This does not mean Penetrizzel has direct access to ur host filesystem.


### 💾 USB storage and game saves

Penetrizzel uses Flat-Address Storage (FAS).

The boot device also stores persistent game data, including:

- Hyper_Wurm high scores.
- Genesis game data / SRAM (e.g. Shining Force savegames).

The OS uses approximately 84 MiB of storage.

U can use the `MEMORY` command to show ur RAM usage.

The relevant storage area is used for FAS and game data. Existing data will be overwritten\!

Persistent data is written during:

- Reboot.
- Shutdown.
- F2 Kernel Soft-reset.

If u want to prevent the OS from writing persistent data, u can physically remove the USB stick after booting, although this also means that subsequent storage operations will fail.

---

## 🐞 Known Limitations & Bugs

There are quite a few... 🙉

### General limitations

- One CPU core only (file reads and Chess AI calculations block the entire CPU.)
- No GPU acceleration.
- USB support is not reliable yet.
- xHCI implementation is still rather buggy. // SEND HELP\!
- HDA audio pin support is incomplete (only works with "Loudspeaker Boxes")
- Only a limited number of sound controllers are supported.
- PNO is still under development and custom PNO applications cannot yet be added conveniently.
- QEMU has some BUGs (especially with UEFI) which I haven't encountered on real hardware.

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

---

## 👨‍💻 About Me

For people who care...

I dislike long loading times, unresponsive menus, and 95% of modern EDV "Tools".

MS Word, Adobe Acrobat, u name it, I hate it.

I did like MS Paint, TaskManager & Editor though... 🎨
...and I am a big fan of KDE Plasma "KATE" (which is my Code Editor basically) - so... it's not all "bad".

On the Contrast, i rly like MS Copilot and even think an agentic OS could be good, if the LLM agent is PURE local, ofc...

Modern software (especially on the WEB) is ridiculously bloated. // = understatement!

I cannot even understand how people can do web design. Huge respect for the people who suffer through this ... aaand u have my pity too\!

### Where did these bad experiences come from?

I tried to port "Hyperwurm is back\!" to WASM via Electron.

...It actually worked at some point...

...after pegging the AI on cooldown, that is...

...until the next major update. -.-

Messing around with dozens of dependencies (which seem to change or shift with every update) was among the least fun experiences I've ever had while doing Vibecoding.

Even worse than xHCI.c kernel driver development.

I actually want to revert to simpler software and maybe make something useful along the way. // 50/50 Chance (4+ 🎲)

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

### [🚂 Get "Hyperwurm is back!" on Steam!](https://store.steampowered.com/app/4664970/Hyperwurm_is_back/)

Play it, try to enjoy it, and share ur high scores\!

That would mean more to me than a random donation.

Arigatou Gozaimasu (ありがとうございます)\! 🙏

---

⛧ **Best regards,**

**ArChanum**

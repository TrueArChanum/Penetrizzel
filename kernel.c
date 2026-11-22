// SPDX-License-Identifier: LicenseRef-proprietary-license
/*
 *
 * Copyright (c) 2026 "Whyvna Züafgre" a.k.a. "ArChanum" or "TrueArChanum" (...'cause some1 stole my Name on GitHub -.- )
 * This file is part of Penetrizzel.
 *
 * Proprietary License Notice
 * ----------------------
 *
 * This source code is proprietary and confidential.
 * Unauthorized copying, modification, distribution, or use of this file,
 * via any medium, is strictly prohibited without prior written permission
 * from the author.
 *
 * Permission is granted to use this software in its compiled form only,
 * under the terms provided by the author. Access to the source code is
 * restricted and may not be shared or disclosed.
 *
 * All of Penetrizzel's source code, binaries, and related materials
 * are provided "as is" without warranty of any kind, either
 * express or implied, including without limitation any implied
 * warranties of condition, uninterrupted use, merchantability,
 * fitness for a particular purpose, or non-infringement.
 *
 * The author assumes no responsibility for any damage or loss caused by
 * the use of this project.
 *
 * All rights reserved.
 *
 * See LICENSE.txt for details.
 *
 * This project can optionally include or use the following third-party software:
 *
 * • 🤡 clownMDEmu
 *  Copyright (c) 2021-2023 Clownacy;
 *  see emulator/clownmdemu-core/LICENCE.txt , emulator/clownmdemu-core/licence.txt)
 *  , emulator/clownmdemu-core/THIRD_PARTY_LICENSES.txt , /emulator/rom/LICENSE.txt)
 *
 * NOTE: u might need a litte patch thou, see: emulator/clownmdemu-core/CHANGELOG.txt
 *
 * ----------------------
 *
 * "Penetrizzel" is a self-contained platform for developing and running retro-style games, including "SEGA Genesis" games, using nothing but freestanding C and Assembly
 * ... without relying on external frameworks or dependencies such as Godot, Unity, Unreal Engine, or SDL3 or Raylib.
 *
 * Inspired by the era of MS-DOS 💾 floppy disk games
 * ... but modernized around USB sticks. Simply boot from a USB stick (or use QEMU instead), no "Hard Drive" (e.g. HDD or SSD) access needed nor supported.
 *
 * Context: TOY OS called "Penetrizzel":
 * - x86 32-bit protected mode, freestanding C99 + Assembly (NASM). I don't want Rust in my 🚗 car, nor in my kernel! xD
 * - supports legacy BIOS (assumes: CSM / Legacy Boot enabled) and UEFI (64-bit) boot.
 * - No paging, nor Multithreading. No SMP 😞 "One CPU core to rule them all..." // me too dumb to implement (, yet)...
 * - VGA MODE-X 320x240 8-bit framebuffer. 4 Memory Planes. Page flipping.
 * - PS/2 keyboard support via IRQ1. USB Mouse & KBD if u are lucky! 🎲
 * - PIT timing.
 * - basic PCI, xHCI, USB (Mouse, Keyboard, Mass Storage).
 * - "PC Beeper" (AKA Earbleeder) Sound and HDA Sound support. // no Volume Control -> WARNING: lobotomizing sound amplitude! U have been warned...
 * - uses Flat-Address Storage = FAS, for USB-Sticks only.
 * - uses PNO or .pno "Penetrizzel Native Object" as executable applications. Pronunciation: [piˈno].
 * - clownMDEmu-core 🤡 as optional .pno emulator (Genesis ROM supported up to 6 MiB).
 * - includes Hyper-Wurm 🪱 & Twilight Chess ♟️ (PNO Games made for the Kernel).
 * - automatically saves Hyper-Wurm Score & Genesis Game Data, stored in "FAS objects" on the USB-Stick used to boot.
 * - no GPU support 💩... uses SIMD software rendering instead (AVX2 🚂, SSE2 🚚, or scalar fallback 🐢).
 * - 💾 Kernel-Size 1168752 bytes, 🎮 ROM 6291456 bytes, 🎶 (compressed) Audio_Blob (for Twilight Chess) 56604160 bytes. // hm... it's like 88% Audio Data xD
 *
 * Limitations:
 * - xHCI (USB) stuff is a nightmare 🆘, and still pretty buggy ... (USB support is not reliable yet).
 * - HDA not fully implemented (only a few "Sound Cards" are supported).
 * - PNO is pretty new; a lot of stuff is hardcoded right now, so u cannot (yet) add new custom .pno apps nor launch em...
 * - Only one CPU without threads means that file reads (reading Game-Data from USB-Stick) or Chess-AI "thinking" will block the "whole CPU".
 */








// kernel.c

#include <stdint.h>

#include "kernel.h"
#include "ACPI.h"
#include "CRT_shader.h"
#include "FAS_32.h"
#include "GLASS_shader.h"
#include "VGA_MTRR.h"
#include "asset_loading.h"
#include "credits.h"
#include "DMA.h"
#include "PCI.h"
#include "SIMD.h"
#include "USB.h"
#include "cpu.h"
#include "framebuffer.h"
#include "game_save.h"
#include "i8042.h"
#include "keyboard.h"
#include "kprintf.h"
#include "memmap.h"
#include "heap.h"
#include "idt.h"
#include "mouse.h"
#include "performance.h"
#include "pic.h"
#include "pit.h"
#include "backbuffer.h"
#include "HD_audio.h"
#include "vga_palette.h"
#include "pool.h"
#include "renderer.h"
#include "speaker.h"
#include "libc_compat.h" // IWYU pragma: keep
#include "kernel_ram_alias.h"

// for ROM_FILE_SIZE_BYTES
#include "ROM/ROM.h"

// for CHESS AUDIO_BLOB
#include "games/Twilight_Chess/generated_audio_map.h"
#include "games/Twilight_Chess/pzaudio_blob.h"
#include "audio.h"
#include "audio_backend.h"

//BEGIN UEFI
#include <stddef.h>
#include "GOP_MTRR.h"
#include "UEFI/BOOT_INFO.h"

extern uint32_t boot_info_ptr;
//END UEFI

static const char *watermark __attribute__((used)) = "💾 Penetrizzel – Built by ArChanum with assistance of 🧮 ChatGPT – 2026";

#if CONSOLE_ACTIVE
#include "console.h"
#include "PNOConsoleCommands.h"
#endif

static bool kernel_reset_requested = false;

volatile bool usb_work_pending; // 🔌 -> 🖱️ + ⌨️ + 🎮

//BEGIN HACK to fix USB issues on high FPS
#define USB_SERVICE_HACK_MIN_FPS 666u
void usb_service_pending_HACK(void)
{
    bool poll_usb;

    if (pit_get_fps_target() < USB_SERVICE_HACK_MIN_FPS) return;

    cpu_cli();

    poll_usb = usb_work_pending;
    usb_work_pending = false;

    cpu_sti();

    if (poll_usb)
        usb_poll();
}
//END HACK

//BEGIN asset_loading
#define ASSET_RAW_DEVICE_ID     0u
#define ASSET_ROM_LBA           157696ull
#define ASSET_AUDIO_LBA         16384ull       //TODO ye..spam magic numbers all over the place - good idea... -.-
//END asset_loading

//BEGIN CRT COLLAPSE
typedef enum
{
    KERNEL_TRANSITION_NONE = 0,
    KERNEL_TRANSITION_RESET,
    KERNEL_TRANSITION_REBOOT,
    KERNEL_TRANSITION_SHUTDOWN

} KernelTransition;

static KernelTransition g_kernel_transition = KERNEL_TRANSITION_NONE;
static bool g_transition_frame_captured = false;
static uint8_t g_crt_palette_rgb[256 * 3];

static void kernel_request_transition(KernelTransition transition)
{
    if (g_kernel_transition != KERNEL_TRANSITION_NONE)   return;

    hda_pcm_stop();

    g_kernel_transition = transition;
    g_transition_frame_captured = false;

    crt_start_collapse();
}
//END CRT COLLAPSE

//BEGIN PROGRAM SELECTION
//BEGIN HD Audio init
//BEGIN SOUND toggle
static bool g_sound_enabled = true;

bool sound_is_enabled(void)
{
    return g_sound_enabled;
}

void sound_toggle(void)
{
    sound_set_enabled(!g_sound_enabled);
}
//END SOUND toggle

//BEGIN Ownership stuff
//PNO audio adapter

//HDA acquisition
static int32_t kernel_pno_audio_pcm_start(void *context, PNO_AudioPCMFillFn callback, void *user_data)
{
    (void)context;

    #if BLASTMASTER_ENABLED // WARNING: Never enable this! "We're So Loud You Wanna Die Go Forth and Amplify!" 🔊 🎵 🎶 -> 🪦
    (void)callback;
    (void)user_data;
    return -1;
    #else
    if (callback == NULL)
        return -1;

    return hda_audio_switch_owner(HDA_AUDIO_OWNER_PNO, callback, user_data) ? 0 : -1;
    #endif
}

//HDA release
static int32_t kernel_pno_audio_pcm_stop(void *context)
{
    (void)context;

    if (hda_audio_get_owner() != HDA_AUDIO_OWNER_PNO)
        return -1;

    hda_audio_release(HDA_AUDIO_OWNER_PNO);
    return hda_audio_get_owner() == HDA_AUDIO_OWNER_NONE ? 0 : -1;
}

//PC Beeper
static int32_t kernel_pno_audio_tone_play(void *context, uint32_t frequency, uint32_t duration_ticks)
{
    (void)context;

    if (!g_sound_enabled || frequency == 0u || duration_ticks == 0u)
        return -1;

    speaker_play(frequency, duration_ticks);
    return 0;
}

static int32_t kernel_pno_audio_tone_set_frequency(void *context, uint32_t frequency)
{
    (void)context;

    if (!g_sound_enabled || frequency == 0u)
        return -1;

    speaker_set_frequency(frequency);
    return 0;
}

static int32_t kernel_pno_audio_tone_stop(void *context)
{
    (void)context;
    speaker_disable();
    return 0;
}
//END Ownership stuff

static void *kernel_map(uint64_t physical_address, uint32_t size)
{
    if (physical_address > 0xFFFFFFFFull)
    {
        kprintf("kernel_map: > 0xFFFFFFFFull");
        return 0;
    }

    if (size == 0u)
    {
        kprintf("kernel_map: size == 0u");
        return 0;
    }

    if (physical_address + (uint64_t)size > 0x100000000ull)
    {
        kprintf("kernel_map: > 0x100000000ull");
        return 0;
    }

    return (void *)(uintptr_t)(uint32_t)physical_address;
}

static void kernel_unmap(void *virtual_address, uint32_t size)
{
    (void)virtual_address;
    (void)size;
}

static bool kernel_audio_init(void)
{
    if (!hda_init_target_pcm(kernel_map, kernel_unmap))
    {
        kprintf("kernel_audio_init: no usable HDA controller\n");
        return false;
    }

    return true;
}
//END HD Audio init

//BEGIN KernelServices table
typedef struct
{
    uint32_t fas_device_id;

} KernelPNOContext;

static KernelPNOContext g_pno_context;
static KernelServices g_pno_services;

static uint32_t g_pno_fas_device_id = FAS_DEVICE_ID_INVALID;

static void *kernel_pno_mem_alloc(void *context, uint32_t size, uint32_t alignment)
{
    (void)context;
    return mem_alloc(size, alignment);
}

static void kernel_pno_mem_free(void *context, void *ptr)
{
    (void)context;
    (void)mem_free(ptr);
}

static void kernel_pno_log(void *context, uint32_t level, const char *message)
{
    (void)context;
    kprintf("PNO[%u]: %s\n", level, message != NULL ? message : "");
}

static uint32_t kernel_pno_get_ticks(void *context)
{
    (void)context;
    return g_ticks;
}

static int32_t kernel_pno_storage_find(void *context, const char *name, PNO_ObjectInfo *info)
{
    KernelPNOContext *storage = (KernelPNOContext *)context;
    FAS_ObjectInfo fas_info;

    if (storage == NULL || name == NULL || info == NULL)
        return -1;

    if (fas_get_info(storage->fas_device_id, name, &fas_info) != FAS_OK)
        return -1;

    memset(info, 0, sizeof(*info));

    info->object_id = fas_info.object_id;
    info->size_bytes = fas_info.size_bytes;
    info->generation = fas_info.generation;
    info->flags = fas_info.flags;

    memcpy(info->name, fas_info.name, sizeof(info->name));

    return 0;
}

static int32_t kernel_pno_storage_read(void *context, uint32_t object_id, uint64_t offset, uint32_t length, void *buffer)
{
    KernelPNOContext *storage = (KernelPNOContext *)context;

    if (storage == NULL)
        return -1;

    return fas_read(storage->fas_device_id, object_id, offset, length, buffer) == FAS_OK ? 0 : -1;
}

static int32_t kernel_pno_storage_write(void *context, uint32_t object_id, uint64_t offset, uint32_t length, const void *buffer)
{
    KernelPNOContext *storage = (KernelPNOContext *)context;

    if (storage == NULL)
        return -1;

    return fas_write(storage->fas_device_id, object_id, offset, length, buffer) == FAS_OK ? 0 : -1;
}

static int32_t kernel_pno_storage_sync(void *context)
{
    KernelPNOContext *storage = (KernelPNOContext *)context;

    if (storage == NULL)
        return -1;

    return fas_sync(storage->fas_device_id) == FAS_OK ? 0 : -1;
}

static int32_t kernel_pno_video_get_surface(void *context, PNO_Surface *surface)
{
    (void)context;

    if (surface == NULL || g_backbuffer.pixels == NULL)
        return -1;

    memset(surface, 0, sizeof(*surface));

    surface->struct_size = sizeof(*surface);
    surface->width = FB_WIDTH;
    surface->height = FB_HEIGHT;
    surface->pitch = FB_WIDTH;
    surface->pixel_format = PNO_PIXEL_INDEX8;
    surface->pixels = g_backbuffer.pixels;

    return 0;
}

static void kernel_pno_video_clear(void *context, uint8_t color)
{
    (void)context;
    clear_back_buffer(color);
}

// two callbacks to expose and modify the kernel's sound preference
static int32_t kernel_pno_audio_get_enabled(void *context)
{
    (void)context;
    return g_sound_enabled ? 1 : 0;
}

static int32_t kernel_pno_audio_set_enabled(void *context, bool enabled)
{
    (void)context;
    sound_set_enabled(enabled);
    return 0;
}

void sound_set_enabled(bool enabled)
{
    g_sound_enabled = enabled;

    if (!enabled)
        speaker_disable();
}

static void kernel_pno_video_set_palette(void *context, uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    (void)context;
    vga_palette_set(index, red, green, blue);
}

static int32_t PNO_ABI_CALL kernel_game_save_register(void *context, const char *object_name, uint32_t max_payload_size, PNO_SaveExportFn export_data, PNO_SaveImportFn import_data, void *provider_context)
{
    GameSaveProvider provider;

    (void)context;

    if (object_name == NULL || export_data == NULL || import_data == NULL)
        return -1;

    provider.object_name = object_name;
    provider.max_payload_size = max_payload_size;
    provider.context = provider_context;
    provider.export_data = export_data;
    provider.import_data = import_data;

    return game_save_register_provider(&provider) ? 0 : -1;
}

static int32_t PNO_ABI_CALL kernel_game_save_store(void *context)
{
    (void)context;
    return game_save_store() ? 0 : -1;
}

static int32_t PNO_ABI_CALL kernel_game_save_unregister(void *context)
{
    (void)context;
    return game_save_unregister_provider() ? 0 : -1;
}

//ROM loading callback
static bool g_genesis_rom_loaded;
static uint32_t g_genesis_rom_size;

static int32_t PNO_ABI_CALL kernel_pno_genesis_rom_acquire(void *context, uint8_t **rom, uint32_t *size)
{
    AssetLoadStatus status;
    uint8_t *active = (uint8_t *)(uintptr_t)RAM_ROM_BASE;
    uint8_t *backup = (uint8_t *)(uintptr_t)RAM_ROM_BACKUP_BASE;

    (void)context;

    if (rom == NULL || size == NULL)
        return -1;

    if (!g_genesis_rom_loaded)
    {
        if (ROM_FILE_SIZE_BYTES == 0u || ROM_FILE_SIZE_BYTES > RAM_ROM_MAX_SIZE || (ROM_FILE_SIZE_BYTES & 1u) != 0u)
            return -1;

        status = asset_load_raw(
            g_pno_fas_device_id,
            ASSET_ROM_LBA,
            ROM_FILE_SIZE_BYTES,
            active,
            RAM_ROM_MAX_SIZE);

        if (status != ASSET_LOAD_OK)
        {
            kprintf("GenesisROM: loading failed: %s\n", asset_load_status_string(status));
            return -1;
        }

        g_genesis_rom_size = (uint32_t)ROM_FILE_SIZE_BYTES;

        memcpy(backup, active, g_genesis_rom_size);

        g_genesis_rom_loaded = true;
    }
    else
    {
        memcpy(active, backup, g_genesis_rom_size);
    }

    *rom = active;
    *size = g_genesis_rom_size;

    return 0;
}

// AUDIO_BLOB callback
static bool g_audio_blob_loaded;
static bool g_audio_blob_decompressed;

static int32_t PNO_ABI_CALL kernel_pno_audio_blob_acquire(void *context, const uint8_t **blob, uint32_t *size)
{
    AssetLoadStatus status;

    (void)context;

    if (blob == NULL || size == NULL)
        return -1;

    if (!g_audio_blob_loaded)
    {
        status = asset_load_raw(    //TODO this is slow af...
            ASSET_RAW_DEVICE_ID,
            ASSET_AUDIO_LBA,
            AUDIO_COMPRESSED_SIZE,
            (void *)(uintptr_t)RAM_AUDIO_BASE,
            RAM_AUDIO_MAX_SIZE);

        if (status != ASSET_LOAD_OK)
            return -2;

        g_audio_blob_loaded = true;
    }

    if (!g_audio_blob_decompressed)
    {
        if (!pzaudio_blob_load(AUDIO_COMPRESSED_SIZE))
            return -3;

        g_audio_blob_decompressed = true;
    }

    *blob = (const uint8_t *)(uintptr_t)RAM_AUDIO_BASE;
    *size = AUDIO_BLOB_SIZE;

    return 0;
}

static bool g_pno_audio_backend_ready = false;

static int32_t PNO_ABI_CALL kernel_pno_audio_asset_play(void *context, const char *name, uint32_t flags)
{
    const uint8_t *blob = NULL;
    uint32_t size = 0u;
    int32_t rc;

    if (name == NULL || name[0] == '\0' || !g_sound_enabled)
        return -1;

    if (!g_pno_audio_backend_ready)
    {
        rc = kernel_pno_audio_blob_acquire(context, &blob, &size);
        if (rc != 0)
        {
            kprintf("PNO audio: blob acquire failed (%d)\n", rc);
            return rc;
        }

        if (blob == NULL || size != AUDIO_BLOB_SIZE)
            return -2;

        if (!audio_init())
        {
            kprintf("PNO audio: audio_init failed\n");
            return -3;
        }
        g_pno_audio_backend_ready = true;
    }

    if (flags & PNO_AUDIO_ASSET_MUSIC)
    {
        if (flags & PNO_AUDIO_ASSET_LOOP)
            audio_play_music_looping(name);
        else
            audio_play_music(name);
    }
    else
    {
        if (flags & PNO_AUDIO_ASSET_LOOP)
            audio_play_sound_looping(name);
        else
            audio_play_sound(name);
    }

    return 0;
}

static int32_t PNO_ABI_CALL kernel_pno_audio_asset_stop(void *context, const char *name, uint32_t flags)
{
    (void)context;

    if (flags & PNO_AUDIO_ASSET_MUSIC)  audio_stop_music();
    else if (name != NULL)              audio_stop_sound(name);

    return 0;
}

static void kernel_audio_shutdown(void)
{
    if (g_pno_audio_backend_ready)
    {
        audio_shutdown();
        g_pno_audio_backend_ready = false;
    }
}

static void kernel_pno_services_init(void)
{
    memset(&g_pno_services, 0, sizeof(g_pno_services));

    g_pno_services.struct_size  = sizeof(g_pno_services);
    g_pno_services.abi_major    = PNO_ABI_VERSION_MAJOR;
    g_pno_services.abi_minor    = PNO_ABI_VERSION_MINOR;
    g_pno_services.context      = &g_pno_context;

    g_pno_services.mem_alloc                = kernel_pno_mem_alloc;
    g_pno_services.mem_free                 = kernel_pno_mem_free;

    g_pno_services.log                      = kernel_pno_log;
    g_pno_services.get_ticks                = kernel_pno_get_ticks;

    g_pno_services.storage_find             = kernel_pno_storage_find;
    g_pno_services.storage_read             = kernel_pno_storage_read;
    g_pno_services.storage_write            = kernel_pno_storage_write;
    g_pno_services.storage_sync             = kernel_pno_storage_sync;

    g_pno_services.video_set_palette        = kernel_pno_video_set_palette;
    g_pno_services.video_get_surface        = kernel_pno_video_get_surface;
    g_pno_services.video_clear              = kernel_pno_video_clear;

    g_pno_services.audio_get_enabled        = kernel_pno_audio_get_enabled;
    g_pno_services.audio_set_enabled        = kernel_pno_audio_set_enabled;
    g_pno_services.audio_pcm_start          = kernel_pno_audio_pcm_start;
    g_pno_services.audio_pcm_stop           = kernel_pno_audio_pcm_stop;
    g_pno_services.audio_tone_play          = kernel_pno_audio_tone_play;
    g_pno_services.audio_tone_set_frequency = kernel_pno_audio_tone_set_frequency;
    g_pno_services.audio_tone_stop          = kernel_pno_audio_tone_stop;

    g_pno_services.game_save_register       = kernel_game_save_register;
    g_pno_services.game_save_store          = kernel_game_save_store;
    g_pno_services.game_save_unregister     = kernel_game_save_unregister;

    g_pno_services.genesis_rom_acquire      = kernel_pno_genesis_rom_acquire;

    g_pno_services.audio_blob_acquire       = kernel_pno_audio_blob_acquire;
    g_pno_services.audio_asset_play         = kernel_pno_audio_asset_play;
    g_pno_services.audio_asset_stop         = kernel_pno_audio_asset_stop;

    g_pno_services.capabilities =
    PNO_CAP_MEMORY |
    PNO_CAP_LOG |
    PNO_CAP_TIME |
    PNO_CAP_STORAGE_READ |
    PNO_CAP_STORAGE_WRITE |
    PNO_CAP_STORAGE_SYNC |
    PNO_CAP_VIDEO_PALETTE |
    PNO_CAP_VIDEO |
    PNO_CAP_AUDIO |
    PNO_CAP_GAME_SAVE;
}

//BEGIN generic kernel callbacks
static void kernel_console_update(void *context)
{
    (void)context;

    #if CONSOLE_ACTIVE
    console_update();
    console_draw();
    #endif
}

static bool kernel_program_can_switch(void *context)
{
    (void)context;
    return !glass_is_active();
}

static bool g_pno_key_state[KEY_COUNT];
static bool g_pno_key_state_valid = false;
static MouseState g_pno_previous_mouse_state;
static bool g_pno_mouse_state_valid = false;

static bool kernel_program_name_equal(const char *a, const char *b)
{
    if (a == NULL || b == NULL)     return false;

    while (*a && *b)
    {
        char ca = *a++;
        char cb = *b++;

        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + ('a' - 'A'));

        if (ca != cb)               return false;
    }

    return *a == *b;
}

static void kernel_program_active_changed(void *context, const char *old_name, const char *new_name)
{
    (void)context;

    g_pno_key_state_valid = false;
    g_pno_mouse_state_valid = false;

    // HACK
    if (kernel_program_name_equal(old_name, "chess.pno"))   audio_backend_deactivate();
    if (kernel_program_name_equal(new_name, "chess.pno"))   (void)audio_backend_activate();

    glass_start(&g_crt.work);
    clear_back_buffer(0);

    #if CONSOLE_ACTIVE
    credits_show_application(new_name);
    if (strcmp(new_name, "CONSOLE") == 0)   console_request_full_redraw();
    #endif
}

static void kernel_program_runtime_update(uint32_t delta_ticks)
{
    KeyEvent key_event;
    MouseState mouse_state;
    bool console_active;

    keyboard_update();

    console_active = ProgramManager_IsConsoleActive();

    while (keyboard_pop_event(&key_event))
    {
        if (!console_active || !key_event.pressed)
            continue;

        if (key_event.key == KEY_F1)
        {
            (void)ProgramManager_SwitchNext();
            continue;
        }

        if (key_event.key == KEY_F2)
        {
            kernel_request_transition(KERNEL_TRANSITION_RESET);
            continue;
        }

        if (key_event.key == KEY_F3)
        {
            kernel_request_transition(KERNEL_TRANSITION_REBOOT);
            continue;
        }

        if (key_event.key == KEY_F4)
        {
            kernel_request_transition(KERNEL_TRANSITION_SHUTDOWN);
            continue;
        }
        #if CONSOLE_ACTIVE
        console_handle_event(&key_event);
        #endif
    }

    if (!console_active)
    {
        for (uint32_t key = 1u; key < KEY_COUNT; ++key)
        {
            bool down = keyboard_is_down((KeyCode)key);
            bool was_down = g_pno_key_state_valid ? g_pno_key_state[key] : false;

            g_pno_key_state[key] = down;

            if (down == was_down && g_pno_key_state_valid)
                continue;

            if (down)
            {
                if (key == KEY_F1)
                {
                    (void)ProgramManager_SwitchNext();
                    continue;
                }

                if (key == KEY_F2)
                {
                    kernel_request_transition(KERNEL_TRANSITION_RESET);
                    continue;
                }

                if (key == KEY_F3)
                {
                    kernel_request_transition(KERNEL_TRANSITION_REBOOT);
                    continue;
                }

                if (key == KEY_F4)
                {
                    kernel_request_transition(KERNEL_TRANSITION_SHUTDOWN);
                    continue;
                }
            }

            PNO_Event event = {0};

            event.type = down ? PNO_EVENT_KEY_DOWN : PNO_EVENT_KEY_UP;
            event.code = key;
            event.value = down ? 1 : 0;
            event.ticks = g_ticks;

            (void)ProgramManager_DispatchEvent(&event);
        }
    }

    g_pno_key_state_valid = true;

    mouse_get_state(&mouse_state);

    if (!ProgramManager_IsConsoleActive())
    {
        PNO_Event event = {0};

        if (!g_pno_mouse_state_valid || mouse_state.x != g_pno_previous_mouse_state.x || mouse_state.y != g_pno_previous_mouse_state.y)
        {
            event.type = PNO_EVENT_POINTER_MOVE;
            event.x = mouse_state.x;
            event.y = mouse_state.y;
            event.ticks = g_ticks;
            (void)ProgramManager_DispatchEvent(&event);
        }

        if (!g_pno_mouse_state_valid || mouse_state.left != g_pno_previous_mouse_state.left)
        {
            event = (PNO_Event){0};
            event.type = PNO_EVENT_POINTER_BUTTON;
            event.code = 1u;
            event.value = mouse_state.left ? 1 : 0;
            event.x = mouse_state.x;
            event.y = mouse_state.y;
            event.ticks = g_ticks;
            (void)ProgramManager_DispatchEvent(&event);
        }

        if (!g_pno_mouse_state_valid || mouse_state.right != g_pno_previous_mouse_state.right)
        {
            event = (PNO_Event){0};
            event.type = PNO_EVENT_POINTER_BUTTON;
            event.code = 2u;
            event.value = mouse_state.right ? 1 : 0;
            event.x = mouse_state.x;
            event.y = mouse_state.y;
            event.ticks = g_ticks;
            (void)ProgramManager_DispatchEvent(&event);
        }

        if (!g_pno_mouse_state_valid || mouse_state.middle != g_pno_previous_mouse_state.middle)
        {
            event = (PNO_Event){0};
            event.type = PNO_EVENT_POINTER_BUTTON;
            event.code = 3u;
            event.value = mouse_state.middle ? 1 : 0;
            event.x = mouse_state.x;
            event.y = mouse_state.y;
            event.ticks = g_ticks;
            (void)ProgramManager_DispatchEvent(&event);
        }

        g_pno_previous_mouse_state = mouse_state;
        g_pno_mouse_state_valid = true;
    }
    else
    {
        g_pno_mouse_state_valid = false;
    }

    (void)ProgramManager_Update(delta_ticks);
}
//END generic kernel callbacks
//END KernelServices table
//END PROGRAM SELECTION

//BEGIN PNO Importer
#define PNO_MAX_BYTES          (1024u * 1024u)
#define WURM_PNO_RAW_LBA       153600ull
#define CLOWN_PNO_RAW_LBA      155648ull
#define CHESS_PNO_RAW_LBA      (83 * 2048)

static void kernel_import_pno(const char *label, const char *object_name, uint64_t raw_lba)
{
    AssetLoadStatus load_status;
    FAS_Status fas_status;
    FAS_ObjectInfo info;
    PNO_Header header;
    PNO_ValidationLimits limits;
    uint8_t *import_buffer = NULL;
    uint32_t object_id = FAS_OBJECT_ID_INVALID;
    uint32_t file_size;

    import_buffer = pool_alloc(PNO_MAX_BYTES);

    if (import_buffer == NULL)
    {
        kprintf("%s import: insufficient memory\n", label);
        return;
    }

    load_status = asset_load_raw(
        GAME_SAVE_DEVICE_ID,
        raw_lba,
        PNO_HEADER_BYTES,
        import_buffer,
        PNO_MAX_BYTES);

    if (load_status != ASSET_LOAD_OK)
    {
        kprintf("%s import: header read failed\n", label);
        goto cleanup;
    }

    if (PNO_DecodeHeader(
        import_buffer,
        PNO_HEADER_BYTES,
        &header) != PNO_OK)
    {
        kprintf("%s import: invalid header\n", label);
        goto cleanup;
    }

    file_size = header.file_size;

    if (file_size < PNO_HEADER_BYTES || file_size > PNO_MAX_BYTES)
    {
        kprintf("%s import: invalid file size: %u\n", label, file_size);
        goto cleanup;
    }

    load_status = asset_load_raw(
        GAME_SAVE_DEVICE_ID,
        raw_lba,
        file_size,
        import_buffer,
        PNO_MAX_BYTES);

    if (load_status != ASSET_LOAD_OK)
    {
        kprintf("%s import: binary read failed\n", label);
        goto cleanup;
    }

    PNO_DefaultValidationLimits(&limits);
    limits.max_file_size = PNO_MAX_BYTES;

    if (PNO_ValidateFile(
        import_buffer,
        file_size,
        &limits,
        &header) != PNO_OK)
    {
        kprintf("%s import: validation failed\n", label);
        goto cleanup;
    }

    fas_status = fas_get_info(
        GAME_SAVE_DEVICE_ID,
        object_name,
        &info);

    if (fas_status == FAS_ERR_NOT_FOUND)
    {
        fas_status = fas_create(
            GAME_SAVE_DEVICE_ID,
            object_name,
            file_size,
            &object_id);
    }
    else if (fas_status == FAS_OK)
    {
        if (info.size_bytes != file_size)
        {
            fas_status = fas_delete(GAME_SAVE_DEVICE_ID, object_name);

            if (fas_status != FAS_OK)
            {
                kprintf("%s import: old object deletion failed: %s\n", label, fas_status_string(fas_status));
                goto cleanup;
            }

            fas_status = fas_create(
                GAME_SAVE_DEVICE_ID,
                object_name,
                file_size,
                &object_id);
        }
        else
        {
            object_id = info.object_id;
        }
    }

    if (fas_status != FAS_OK)
    {
        kprintf("%s import: object creation/lookup failed: %s\n", label, fas_status_string(fas_status));
        goto cleanup;
    }

    fas_status = fas_write(
        GAME_SAVE_DEVICE_ID,
        object_id,
        0u,
        file_size,
        import_buffer);

    if (fas_status != FAS_OK)
    {
        kprintf("%s import: write failed: %s\n", label, fas_status_string(fas_status));
        goto cleanup;
    }

    fas_status = fas_sync(GAME_SAVE_DEVICE_ID);

    if (fas_status != FAS_OK)
    {
        kprintf("%s import: sync failed: %s\n", label, fas_status_string(fas_status));
        goto cleanup;
    }

    kprintf("%s: %s imported (%u bytes, ID %u)\n", label, object_name, file_size, object_id);

cleanup:
    pool_free(import_buffer);
}
//END PNO Importer

extern void kernel_gdt_init(void); // HACK to fix GDT & IDT and all the other UEFI crap.

//BEGIN 32-bit KERNEL INIT
void kernel_start(void)
{

    // // for (;;) //TEST HALT
    // //     __asm__ volatile ("cli; hlt");

    KERNEL_RESET:

    cpu_cli();

    //BEGIN UEFI
    BOOT_INFO *BootInfo = NULL;
    uint32_t boot_source = BOOT_SOURCE_BIOS;

    if (boot_info_ptr != 0)
    {
        BootInfo = (BOOT_INFO *)(uintptr_t)boot_info_ptr;

        if (BootInfo->version != BOOT_INFO_VERSION || BootInfo->size < sizeof(BOOT_INFO))
        {
            BootInfo = NULL;
        }
        else
        {
            boot_source = BootInfo->boot_source;
        }

        if (boot_source == BOOT_SOURCE_UEFI && BootInfo == NULL)    shutdown();
    }
    //END UEFI

    //BEGIN Kernel-owned CPU infrastructure
    kernel_gdt_init();  // NOTE: Establish kernel-owned GDT.
    init_idt();
    init_pic();
    init_pit();
    //END Kernel-owned CPU infrastructure

    //BEGIN Pointer madness
    if (boot_source == BOOT_SOURCE_UEFI)
    {
        g_ram_heap_start      = (uintptr_t)BootInfo->heap_address;
        g_ram_pool_base       = (uintptr_t)BootInfo->pool_address;
        g_ram_rom_base        = (uintptr_t)BootInfo->rom_address;
        g_ram_rom_backup_base = (uintptr_t)BootInfo->rom_backup_address;
        g_ram_dma_base        = (uintptr_t)BootInfo->dma_address;
        g_ram_audio_base      = (uintptr_t)BootInfo->audio_address;
    }
    else
    {
        g_ram_heap_start      = RAM_HEAP_START_FIXED;
        g_ram_pool_base       = RAM_POOL_BASE_FIXED;
        g_ram_rom_base        = RAM_ROM_BASE_FIXED;
        g_ram_rom_backup_base = RAM_ROM_BACKUP_FIXED;
        g_ram_dma_base        = RAM_DMA_BASE_FIXED;
        g_ram_audio_base      = RAM_AUDIO_BASE_FIXED;
    }
    if (g_ram_heap_start      == 0 ||
        g_ram_pool_base       == 0 ||
        g_ram_rom_base        == 0 ||
        g_ram_rom_backup_base == 0 ||
        g_ram_dma_base        == 0 ||
        g_ram_audio_base      == 0)
    {
        shutdown();
    }
    //END Pointer madness

    //BEGIN memmap_is_usable()
    if (boot_source == BOOT_SOURCE_UEFI)
    {
        if (BootInfo == NULL)                                                                   shutdown();
        if (!memmap_is_usable(RAM_KERNEL_LOAD, RAM_KERNEL_MAX_END - RAM_KERNEL_LOAD))           shutdown();
        if (!memmap_is_usable(RAM_STACK_BOTTOM, RAM_STACK_SIZE))                                shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->rom_address, RAM_ROM_MAX_SIZE))              shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->rom_backup_address, RAM_ROM_BACKUP_SIZE))    shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->dma_address, RAM_DMA_SIZE))                  shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->pool_address, RAM_POOL_SIZE))                shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->heap_address, RAM_HEAP_SIZE))                shutdown();
        if (!memmap_is_usable((uintptr_t)BootInfo->audio_address, AUDIO_BLOB_SIZE))             shutdown();
    }
    else if (boot_source == BOOT_SOURCE_BIOS)
    {
        uintptr_t kernel_start_addr = RAM_KERNEL_LOAD;
        uintptr_t kernel_end_addr = (uintptr_t)&kernel_end;

        if (kernel_end_addr <= kernel_start_addr)                        shutdown();

        uintptr_t kernel_size = kernel_end_addr - kernel_start_addr;

        if (!memmap_is_usable(kernel_start_addr, kernel_size))           shutdown();
        if (!memmap_is_usable(RAM_HEAP_START, RAM_HEAP_SIZE))            shutdown();
        if (!memmap_is_usable(RAM_ROM_BASE, RAM_ROM_MAX_SIZE))           shutdown();
        if (!memmap_is_usable(RAM_ROM_BACKUP_BASE, RAM_ROM_BACKUP_SIZE)) shutdown();
        if (!memmap_is_usable(RAM_DMA_BASE, RAM_DMA_SIZE))               shutdown();
        if (!memmap_is_usable(RAM_POOL_BASE, RAM_POOL_SIZE))             shutdown();
        if (!memmap_is_usable(RAM_AUDIO_BASE, AUDIO_BLOB_SIZE))          shutdown();
    }
    else                                                                 shutdown();
    //END memmap_is_usable()

    // WARNING: RAM_* addresses are runtime aliases (kernel_ram_alias.h), initialized for BIOS or UEFI above.

    //BEGIN memory init
    kmem_init_dynamic(RAM_HEAP_START, RAM_HEAP_SIZE);
    if (!pool_init(RAM_POOL_BASE, RAM_POOL_SIZE))                        shutdown();
    //END memory init

    #if CONSOLE_ACTIVE
    console_init();
    //BEGIN kprintf() is useable now
    #endif

    simd_init();    // NOTE: AVX2 and SSE2 support only ... cause, my Ryzen 9 5900X does not support AVX-512 nor AVX10 :(

    //BEGIN Renderer init
    uint8_t *pixels = kmalloc(FB_SIZE);
    if (pixels == NULL) shutdown();

    if (!vga_mtrr_init())    kprintf("VGA MTRR: Write-combining unavailable\n");

    backbuffer_init(pixels, FB_WIDTH, FB_HEIGHT);

    //BEGIN BIOS & UEFI framebuffer backends
    if (boot_source == BOOT_SOURCE_UEFI)
    {
        if (!framebuffer_init_gop(&g_framebuffer, BootInfo))
        {
            kprintf("FRAMEBUFFER GOP FAILED\n");
        }
        else
        {
            uint64_t framebuffer_size = BootInfo->framebuffer_size;

            if (!gop_mtrr_init(BootInfo->framebuffer_address, framebuffer_size))
            {
                kprintf("GOP MTRR FAILED: %u\n", (uint32_t)gop_mtrr_get_status());
            }
            else
            {
                kprintf("  GOP MTRR: Write-combining ENABLED\n");
            }
        }
    }
    else
    {
        framebuffer_init(&g_framebuffer);
    }
    //END BIOS & UEFI framebuffer backends

    renderer_init(&g_renderer);

    renderer_set_target(&g_renderer, &g_backbuffer);
    //END Renderer init

    //TODO Set up new loading splash screen here? ..nah, just make boot fasta...

    //BEGIN ACPI
    if (boot_source == BOOT_SOURCE_UEFI)
    {
        if (BootInfo->acpi_rsdp_address == 0)
        {
            kprintf("ACPI: UEFI RSDP unavailable\n");
        }
        else if (!acpi_init_from_rsdp(BootInfo->acpi_rsdp_address))
        {
            kprintf("ACPI: UEFI initialization failed\n");
        }
    }
    else
    {
        if (!acpi_init())   kprintf("ACPI: BIOS initialization failed\n");
    }
    //END ACPI

    pci_init();

    if (!dma_init())    shutdown();

    //BEGIN FAS
    FAS_Status status;
    status = fas_init();
    if (status != FAS_OK)   kprintf("Flat-Address Storage initialization failed: %s\n", fas_status_string(status));
    //END FAS

    usb_init(); // NOTE: does xhci_init()

    //BEGIN Initialize the generic manager
    g_pno_fas_device_id = GAME_SAVE_DEVICE_ID; // TODO #define GAME_SAVE_DEVICE_ID         0u        -> hardcoded block device registered as ID 0
    //END Initialize the generic manager

    //BEGIN Import
    status = game_save_init();

    if (status != FAS_OK)
    {
        kprintf("GameSave: persistent storage unavailable\n");
    }
    else
    {
        kernel_import_pno("Hyper-Wurm",     "wurm.pno",  WURM_PNO_RAW_LBA);
        kernel_import_pno("clownMDEmu",     "clown.pno", CLOWN_PNO_RAW_LBA);
        kernel_import_pno("Twilight Chess", "chess.pno", CHESS_PNO_RAW_LBA);
    }
    //END Import

    if (!kernel_audio_init())   kprintf("HDA: N/A - it is 'Earbleeder time'!\n");

    //BEGIN input init
    i8042_init();

    keyboard_init();

    mouse_init();

    if (usb_keyboard_present()) i8042_disable_legacy_keyboard();
    if (usb_mouse_present())    i8042_disable_legacy_mouse();
    //END Input init

    //BEGIN Program init
    ProgramManagerConfig program_config = {0};
    ProgramManagerStatus program_status;

    g_pno_context.fas_device_id = g_pno_fas_device_id;

    kernel_pno_services_init();

    program_config.fas_device_id = g_pno_fas_device_id;
    program_config.services      = &g_pno_services;
    program_config.max_loaded    = PROGRAM_MANAGER_DEFAULT_MAX_LOADED;

    program_config.console_update = kernel_console_update;
    program_config.can_switch     = kernel_program_can_switch;
    program_config.active_changed = kernel_program_active_changed;

    program_status = ProgramManager_Init(&program_config);

    if (program_status != PM_OK)
    {
        kprintf("ProgramManager initialization failed: %s\n", ProgramManager_StatusString(program_status));
    }
    else
    {
        program_status = ProgramManager_Discover();
        if (program_status != PM_OK)    kprintf("PNO discovery failed: %s\n", ProgramManager_StatusString(program_status));
        else                            kprintf("PNO: discovered %u applications\n", ProgramManager_GetDiscoveredCount());
    }
    #if CONSOLE_ACTIVE
    PNOConsoleCommands_Init(console_write_line);
    #endif

    //BEGIN credits
    credits_init(); // 💳
    credits_palette_init();
    credits_show_application(ProgramManager_GetActiveName());
    //END credits
    //END Program init

    cpu_sti();

    clear_back_buffer(0);   // obsolete? cause kernel_pno_services_init() ?

    kernel_reset_requested = false;

    kprintf("kernel footprint: %u bytes\n", (uint32_t)(&kernel_end - &__kernel_start)); // 💾 1163088 bytes, ROM 6291456 bytes, Audio_Blob (for Twilight Chess) 56604160

    kernel_main();

    //BEGIN reset path
    glass_shutdown();
    crt_shutdown();

    cpu_cli();

    (void)ProgramManager_Shutdown();

    kernel_audio_shutdown();

    // TODO: BUG only QEMU (real hardware works fine): HDA PCM produces continuous buzzing ("BRRRRRrr") after F2 soft reset.

    hda_shutdown();

    (void)game_save_shutdown();
    (void)usb_shutdown();       // 🔌 NOTE: DMA arena does get reset via dma_init()

    speaker_disable();          // 🪓 Kill the Earbleeder!

    goto KERNEL_RESET;          // 🍝 reset();
}
//END 32-bit KERNEL INIT

//BEGIN MAIN LOOP
void kernel_main(void)
{
    uint32_t last_tick;
    float kernel_time_seconds = 0.0f;

    performance_init(); // 🏁

    if (!crt_init())    shutdown(); // 📺
    if (!glass_init())  kprintf("GLASS: initialization FAILED\n");

    vga_palette_copy_rgb(g_crt_palette_rgb);
    crt_set_palette(g_crt_palette_rgb);

    last_tick = g_ticks;

    // ➰ Main loop
    while (!kernel_reset_requested)
    {
        uint32_t now;
        uint32_t delta_ticks;
        bool collapse_finished;

        //BEGIN idle loop
        while (g_ticks == last_tick)
        {
            bool poll_usb;

            cpu_cli();

            poll_usb = usb_work_pending;
            usb_work_pending = false;

            if (g_ticks != last_tick)
            {
                cpu_sti();
                break;
            }

            if (poll_usb)
            {
                cpu_sti();
                usb_poll(); // ⚡ 1000 hz
                continue;
            }

            cpu_sti_hlt();
        }
        //END idle loop

        now = g_ticks;  // ⏰ default = 60 Hz (variable 1-1000 Hz)
        delta_ticks = now - last_tick;
        last_tick = now;

        float delta_seconds = (float)delta_ticks / (float)pit_get_fps_target();
        kernel_time_seconds += delta_seconds;
        crt_update_time(kernel_time_seconds);

        if (g_kernel_transition != KERNEL_TRANSITION_NONE)
        {
            performance_frame_begin();

            if (!g_transition_frame_captured)
            {
                vga_palette_copy_rgb(g_crt_palette_rgb);
                crt_set_palette(g_crt_palette_rgb);
                crt_apply(&g_backbuffer, &g_crt.work);
                simd_memcpy(g_backbuffer.pixels, g_crt.work.pixels, FB_SIZE);

                g_transition_frame_captured = true;
            }

            collapse_finished = crt_update_collapse(delta_seconds);
            crt_apply_collapse(&g_backbuffer, &g_crt.work, g_crt.collapse_progress);

            framebuffer_present(&g_framebuffer, &g_crt.work);

            performance_frame_end();

            if (collapse_finished)
            {
                KernelTransition transition;

                transition = g_kernel_transition;
                g_kernel_transition = KERNEL_TRANSITION_NONE;
                g_transition_frame_captured = false;

                if (transition == KERNEL_TRANSITION_RESET)          kernel_reset_requested = true;
                else if (transition == KERNEL_TRANSITION_REBOOT)    reboot();
                else if (transition == KERNEL_TRANSITION_SHUTDOWN)  shutdown();
            }
            continue;
        }

        performance_frame_begin();

        usb_service_pending_HACK(); // HACK kicks in at 666 FPS+

        kernel_program_runtime_update(delta_ticks); // 🖥️ > 🪱 > ♟️ > 🤡

        if (g_kernel_transition != KERNEL_TRANSITION_NONE)
        {
            performance_frame_end();
            continue;
        }

        credits_update();

        speaker_update();

        performance_draw_hud();

        credits_draw();

        //BEGIN 🎵
        if (hda_pcm_is_running())
        {

            if (!hda_pcm_service())
            {
                kprintf(
                    "HDA: PCM service FAILED, underruns=%u, owner=%u, position=%u\n",
                    hda_pcm_get_underruns(),
                    (unsigned int)hda_audio_get_owner(),
                    hda_pcm_get_position());

                hda_audio_release(hda_audio_get_owner());
            }
        }
        //END 🎵

        const Surface *present_surface;
        vga_palette_copy_rgb(g_crt_palette_rgb);
        crt_set_palette(g_crt_palette_rgb);
        crt_apply(&g_backbuffer, &g_crt.work);
        glass_update(delta_seconds);
        present_surface = glass_render(&g_crt.work);
        framebuffer_present(&g_framebuffer, present_surface);   // 🖥️

        performance_frame_end();

    }
}
//END MAIN LOOP

Penetrizzel – Genesis ROM Replacement

//WARNING:
Use at ur own risk. The author takes no responsibility for
data loss, corrupted disk images, or any damage caused by using
these scripts. Always keep backups of ur original images.





//NOTE: info:
I did alrdy create a "FAT TOFU" ROM.bin (via build.sh to compile the whole kernel)
This pads TOFU original 34.1 KiB to exactly 6 MiB using 0xFF.

Why? The kernel uses a compile-time ROM_FILE_SIZE_BYTES configuration.
Padding TOFU ensures the reserved ROM region is consistently 6 MiB,
allowing ROM replacements (ur new ROM) of different sizes without rebuilding the kernel.





//TODO:

Replace a ROM (Accepts Genesis ROMs up to 6 MiB)


Modify ur uefi.img or disk.img by replacing its "FAT TOFU" ROM with the ROM u wanna play next:


UEFI bootloader:
python replace_rom.py uefi.img ROM.bin
(or use replace_rom.sh instead, if u dislike Python...)


Or:


BIOS bootloader:
python replace_rom.py disk.img ROM.bin

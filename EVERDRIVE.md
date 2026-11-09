# EverDrive GBCOS testing

Build the Linux desktop emulator with `make -C platforms/linux -j4`. Set
`EverDriveSDPath` to the directory representing the SD root, then open
`GBCSYS/GBCOS.BIN` from that directory. The stock v1.06 image is 81,920 bytes
(MD5 `f200a1c6417323274e30ddf6ce5ec263`); it does not need a modified ROM header
or padding to 128 KiB. Keep mapper selection on Auto.

Stock initialization requires these behaviors:

- Identify the fixed `GBXOS`/`KRKZ` header fields even when the title is padded
  with `$FF`. Otherwise it selects ROM Only and `$BD0C` reads return `$FF`,
  leaving the OS in its I2C busy loop.
- Preserve CMD18 multi-block reads across chip-select changes; CMD12 stops them.
- Present two mirrored FATs in the folder-backed virtual disk. GBCOS computes
  the start of the data area using two FATs.
- Support CMD25 writes with `$FC` block tokens and `$FD` stop tokens, in addition
  to CMD24 single-block writes.
- Retain I2C EEPROM bytes written through `$BD0C/$BD0D`. Preparing a game writes
  its identifier into the settings record; launching compares it with a readback.

The EEPROM implementation models byte transactions for the `$A0` and `$A2`
devices, including their two-byte and one-byte addresses and repeated START for
reads. Transfers complete immediately. Contents remain for the lifetime of the
emulator instance, including cartridge resets; they are not persisted to disk or
included in save states. RTC clock progression and complete FPGA behavior are
not implemented by this change.

A fresh emulated save RAM can produce GBCOS's “Battery has run dry” warning.
Press a button to let the OS initialize its marker. This warning cleared on
subsequent boots in testing with the same emulated save RAM.

Run component regressions with:

```
make -C tests everdrive
```

These use synthetic ROM headers and a temporary host directory. They check
stock/replacement OS detection, FAT layout and mirroring, SPI reads across chip
select, multi-sector write/readback, single-block writes, and EEPROM readback
across bus resets. They require no GBCOS binary or physical card.

For interactive debugging, launch with `--mcp-stdio --headless --portable` from
an isolated executable directory. Set `GEARBOY_SD_DIR` to a temporary copy of
the SD directory and, if needed, `SDL_AUDIO_DRIVER=dummy`. The environment SD
path takes precedence over `EverDriveSDPath`. Folder-backed SD writes can update
host files, so a copy keeps debugging separate from hardware testing.

Integration verified through the stdio MCP server with unmodified stock v1.06:
initialization, folder navigation, opening the game menu, and “Select and Start”
loading the 32 KiB DMG Test Suite from the copied SD directory into game PSRAM.
The resulting Game Boy test menu rendered correctly. In-game menu/snapshot
behavior and battery-backed game save round trips have not been verified here.

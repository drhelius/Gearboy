
.:: Gearboy ::.
-----------------------------------------------------
Instructions and tips at:
https://github.com/drhelius/Gearboy
-----------------------------------------------------
Gearboy is a cross-platform Game Boy / Game Boy Color emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch.
Please, consider sponsoring (https://github.com/sponsors/drhelius) and following me on Twitter (http://twitter.com/drhelius) for updates.
-----------------------------------------------------
Features:
    - Accurate CPU emulation, passes cpu_instrs.gb from blargg's tests.
    - Accurate instruction and memory timing, passes instr_timing.gb and mem_timing.gb from blargg's tests.
    - Supported cartridges: ROM, ROM + RAM, MBC1, MBC2, MBC3 + RTC, MBC5, HuC-1 and MBC1M (multicart).
    - Accurate LCD controller emulation with correct timings and priorities including mid-scanline effects.
    - Game Boy Color support.
    - LCD screen ghosting effect as seen in the original Game Boy.
    - LCD dot matrix effect.
    - Battery powered RAM save support.
    - Save states.
    - Compressed rom support (ZIP).
    - Bootrom (BIOS) support.
    - Game Genie and GameShark cheat support.
    - Supported platforms (standalone): Windows, Linux, BSD and macOS.
    - Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
    - Full debugger with just-in-time disassembler, cpu breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, memory editor, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
    - Windows and Linux Portable Mode.
    - Rom loading from the command line by adding the rom path as an argument.
    - Support for modern game controllers through gamecontrollerdb.txt file located in the same directory as the application binary.
-----------------------------------------------------
Gearboy is licensed under the GNU General Public License v3.0 License:

Copyright (C) 2012 Ignacio Sanchez
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/

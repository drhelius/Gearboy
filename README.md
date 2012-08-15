Gearboy
=======

Nintendo Gameboy emulator core written in C++.

Its main focus is readability of source code, but it has good compatibility.

Compiles on Windows (Visual Studio 2010) Linux and on Mac OSX.

Features:
- Full CPU emulation, passes cpu_instrs.gb from blargg tests.
- Accurate instruction timing, passes instr_timing.gb from blargg tests.
- Full support for most common Memory Bank Controllers (ROM, MBC1, MBC2, MBC3, MBC5)
- Accurate emulation of LCD controller. Full support for backgrounds, window and sprites.

TODO:
- Add sound support
- Improve compatibility
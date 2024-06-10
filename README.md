# Gearboy

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/drhelius/Gearboy/gearboy.yml)](https://github.com/drhelius/Gearboy/actions/workflows/gearboy.yml)
[![GitHub Releases)](https://img.shields.io/github/v/tag/drhelius/Gearboy?label=version)](https://github.com/drhelius/Gearboy/releases)
[![commits)](https://img.shields.io/github/commit-activity/t/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/graphs/contributors)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/drhelius)](https://github.com/sponsors/drhelius)
[![GitHub](https://img.shields.io/github/license/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/blob/master/LICENSE)
[![Twitter Follow](https://img.shields.io/twitter/follow/drhelius)](https://x.com/drhelius)

Gearboy is a cross-platform Game Boy / Game Boy Color emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch.

This is an open source project with its ongoing development made possible thanks to the support by these awesome [backers](backers.md). If you find it useful, please, consider [sponsoring](https://github.com/sponsors/drhelius).

Don't hesitate to report bugs or ask for new features by [openning an issue](https://github.com/drhelius/Gearboy/issues). 

----------

## Downloads

- **Windows**:
  - [Gearboy-3.5.0-windows.zip](https://github.com/drhelius/Gearboy/releases/download/3.5.0/Gearboy-3.5.0-windows.zip)
  - NOTE: If you have errors you may need to install:
    - [Microsoft Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572)
    - [OpenGL Compatibility Pack](https://apps.microsoft.com/detail/9nqpsl29bfff)
- **macOS**:
  - [Gearboy-3.5.0-macos-arm.zip](https://github.com/drhelius/Gearboy/releases/download/3.5.0/Gearboy-3.5.0-macos-arm.zip)
  - [Gearboy-3.5.0-macos-intel.zip](https://github.com/drhelius/Gearboy/releases/download/3.5.0/Gearboy-3.5.0-macos-intel.zip)
- **Linux**:
  - [Gearboy-3.5.0-ubuntu-22.04.zip](https://github.com/drhelius/Gearboy/releases/download/3.5.0/Gearboy-3.5.0-ubuntu-22.04.zip)
  - [Gearboy-3.5.0-ubuntu-20.04.zip](https://github.com/drhelius/Gearboy/releases/download/3.5.0/Gearboy-3.5.0-ubuntu-20.04.zip) 
  - NOTE: You may need to install `libsdl2` and `libglew`
- **RetroArch**: [Libretro core documentation](https://docs.libretro.com/library/gearcoleco/).

## Features

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
- *Game Genie* and *GameShark* cheat support.
- Supported platforms (standalone): Windows, Linux, BSD and macOS.
- Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
- Full debugger with just-in-time disassembler, cpu breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, memory editor, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
- Windows and Linux *Portable Mode*.
- Rom loading from the command line by adding the rom path as an argument.
- Support for modern game controllers through [gamecontrollerdb.txt](https://github.com/gabomdq/SDL_GameControllerDB) file located in the same directory as the application binary.

<img src="http://www.geardome.com/files/gearboy/gearboy_debug_01.png" width="880" height="455">

## Tips

- *Boot ROM*: Gearboy can run with or without a Boot ROM. You can optionally load a Boot ROM and enable it.
- *Mouse Cursor*: Automatically hides when hovering main output window or when Main Menu is disabled.
- *Portable Mode*: Create an empty file named `portable.ini` in the same directory as the application binary to enable portable mode.
- *Debug Symbols*: The emulator always tries to load a symbol file at the same time a rom is being loaded. For example, for ```path_to_rom_file.gb``` it tries to load ```path_to_rom_file.sym```. It is also possible to load a symbol file using the GUI or using the CLI.
- *Command Line Usage*: ```gearboy [rom_file] [symbol_file]```
  
## Build Instructions

### Windows

- Install Microsoft Visual Studio Community 2022 or later.
- Open the Gearboy Visual Studio solution `platforms/windows/Gearboy.sln` and build.
- You may want to use the `platforms/windows/Makefile` to build the application using MinGW.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run these commands to generate a Mac *app* bundle:

``` shell
brew install sdl2
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian / Raspberry Pi (Raspbian):

``` shell
sudo apt-get install build-essential libsdl2-dev libglew-dev libgtk-3-dev
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL2-devel glew-devel gtk3-devel
cd platforms/linux
make
```

### BSD

- FreeBSD:

``` shell
su root -c "pkg install -y git gmake pkgconf SDL2 glew lang/gcc gtk3"
cd platforms/bsd
gmake
```

- NetBSD:

``` shell
su root -c "pkgin install gmake pkgconf SDL2 glew lang/gcc gtk3"
cd platforms/bsd
gmake
```

### Libretro

- Ubuntu / Debian / Raspberry Pi (Raspbian):

``` shell
sudo apt-get install build-essential
cd platforms/libretro
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++
cd platforms/libretro
make
```

## Accuracy Tests

Compared to other emulators: [see here](http://tasvideos.org/EmulatorResources/GBAccuracyTests.html).

Tests from [blargg's test roms](https://github.com/retrio/gb-test-roms):

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/gearboy_001.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/gearboy_002.png)![lcd_sync.gb](http://www.geardome.com/files/gearboy/gearboy_003.png)![dmg_sound.gb](http://www.geardome.com/files/gearboy/gearboy_032.png)![cgb_sound.gb](http://www.geardome.com/files/gearboy/gearboy_033.png)![mem_timing.gb](http://www.geardome.com/files/gearboy/gearboy_memtiming2.png)

## Screenshots

![Screenshot](http://www.geardome.com/files/gearboy/gearboy_004.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_006.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_008.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_022.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_013.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_023.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_015.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_029.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_011.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_024.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_017.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_016.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_034.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_026.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_018.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_025.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_021.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_027.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_019.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_020.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_031.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_028.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_007.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_009.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_010.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_005.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_012.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_014.png)

## Contributors

Thank you to all the people who have already contributed to Gearboy!

[![Contributors](https://contrib.rocks/image?repo=drhelius/gearboy)]("https://github.com/drhelius/gearboy/graphs/contributors)

## License

Gearboy is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.

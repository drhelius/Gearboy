# Gearboy

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/drhelius/Gearboy/gearboy.yml)](https://github.com/drhelius/Gearboy/actions/workflows/gearboy.yml)
[![GitHub Releases)](https://img.shields.io/github/v/tag/drhelius/Gearboy?label=version)](https://github.com/drhelius/Gearboy/releases)
[![commits)](https://img.shields.io/github/commit-activity/t/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/graphs/contributors)
[![GitHub Sponsors](https://img.shields.io/github/sponsors/drhelius)](https://github.com/sponsors/drhelius)
[![License](https://img.shields.io/github/license/drhelius/Gearboy)](https://github.com/drhelius/Gearboy/blob/master/LICENSE)
[![Twitter Follow](https://img.shields.io/twitter/follow/drhelius)](https://x.com/drhelius)

Gearboy is a cross-platform Game Boy / Game Boy Color emulator written in C++ that runs on Windows, macOS, Linux, BSD and RetroArch, with an embedded MCP server for debugging and tooling.

This is an open source project with its ongoing development made possible thanks to the support by these awesome [backers](backers.md). If you find it useful, please consider [sponsoring](https://github.com/sponsors/drhelius).

Don't hesitate to report bugs or ask for new features by [opening an issue](https://github.com/drhelius/Gearboy/issues).

<img src="http://www.geardome.com/files/gearboy/gearboy_debug_03.png">

## Downloads

<table>
  <thead>
    <tr>
      <th>Platform</th>
      <th>Architecture</th>
      <th>Download Link</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td rowspan="4"><strong>Windows</strong></td>
      <td>Desktop x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-windows-x64.zip">Gearboy-3.7.5-desktop-windows-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-windows-arm64.zip">Gearboy-3.7.5-desktop-windows-arm64.zip</a></td>
    </tr>
    <tr>
      <td>MCPB x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-windows-x64.mcpb">Gearboy-3.7.5-mcpb-windows-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>MCPB ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-windows-arm64.mcpb">Gearboy-3.7.5-mcpb-windows-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="5"><strong>macOS</strong></td>
      <td>Homebrew</td>
      <td><code>brew install --cask drhelius/geardome/gearboy</code></td>
    </tr>
    <tr>
      <td>Desktop Apple Silicon</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-macos-arm64.zip">Gearboy-3.7.5-desktop-macos-arm64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Intel</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-macos-intel.zip">Gearboy-3.7.5-desktop-macos-intel.zip</a></td>
    </tr>
    <tr>
      <td>MCPB arm64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-macos-arm64.mcpb">Gearboy-3.7.5-mcpb-macos-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td>MCPB x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-macos-x64.mcpb">Gearboy-3.7.5-mcpb-macos-x64.mcpb</a></td>
    </tr>
    <tr>
      <td rowspan="5"><strong>Linux</strong></td>
      <td>Desktop Ubuntu 24.04 x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-ubuntu24.04-x64.zip">Gearboy-3.7.5-desktop-ubuntu24.04-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Ubuntu 22.04 x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-ubuntu22.04-x64.zip">Gearboy-3.7.5-desktop-ubuntu22.04-x64.zip</a></td>
    </tr>
    <tr>
      <td>Desktop Ubuntu 24.04 ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-desktop-ubuntu24.04-arm64.zip">Gearboy-3.7.5-desktop-ubuntu24.04-arm64.zip</a></td>
    </tr>
    <tr>
      <td>MCPB x64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-linux-x64.mcpb">Gearboy-3.7.5-mcpb-linux-x64.mcpb</a></td>
    </tr>
    <tr>
      <td>MCPB ARM64</td>
      <td><a href="https://github.com/drhelius/Gearboy/releases/download/3.7.5/Gearboy-3.7.5-mcpb-linux-arm64.mcpb">Gearboy-3.7.5-mcpb-linux-arm64.mcpb</a></td>
    </tr>
    <tr>
      <td><strong>RetroArch</strong></td>
      <td>All platforms</td>
      <td><a href="https://docs.libretro.com/library/gearboy/">Libretro core documentation</a></td>
    </tr>
    <tr>
      <td><strong>Dev Builds</strong></td>
      <td>All platforms</td>
      <td><a href="https://github.com/drhelius/Gearboy/actions/workflows/gearboy.yml">GitHub Actions</a></td>
    </tr>
  </tbody>
</table>

**Notes:**
- **Windows**: May need [Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572) and [OpenGL Compatibility Pack](https://apps.microsoft.com/detail/9nqpsl29bfff)
- **Linux**: May need `libsdl3`

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
- VGM recorder.
- Supported platforms (standalone): Windows, Linux, BSD and macOS.
- Supported platforms (libretro): Windows, Linux, macOS, Raspberry Pi, Android, iOS, tvOS, PlayStation Vita, PlayStation 3, Nintendo 3DS, Nintendo GameCube, Nintendo Wii, Nintendo WiiU, Nintendo Switch, Emscripten, Classic Mini systems (NES, SNES, C64, ...), OpenDingux, RetroFW and QNX.
- Full debugger with just-in-time disassembler, CPU breakpoints, memory access breakpoints, code navigation (goto address, JP JR and CALL double clicking), debug symbols, automatic labels, memory editor, trace logger, IO inspector and VRAM viewer including tiles, sprites, backgrounds and palettes.
- MCP server for AI-assisted debugging with GitHub Copilot, Claude, ChatGPT and similar, exposing tools for execution control, memory inspection, hardware status, and more.
- Windows and Linux *Portable Mode*.
- ROM loading from the command line by adding the ROM path as an argument.
- ROM loading using drag & drop.
- Support for modern game controllers through [gamecontrollerdb.txt](https://github.com/mdqinc/SDL_GameControllerDB) file located in the same directory as the application binary.

## Tips

### Basic Usage
- **Boot ROM**: Gearboy can run with or without a Boot ROM. You can optionally load a Boot ROM and enable it.
- **Mouse Cursor**: Automatically hides when hovering over the main output window or when Main Menu is disabled.
- **Portable Mode**: Create an empty file named `portable.ini` in the same directory as the application binary to enable portable mode.

### Debugging Features
- **Docking Windows**: In debug mode, you can dock windows together by pressing SHIFT and dragging a window onto another.
- **Multi-viewport**: In Windows or macOS, you can enable "multi-viewport" in the debug menu. You must restart the emulator for the change to take effect. Once enabled, you can drag debugger windows outside the main window.
- **Single Instance**: You can enable "Single Instance" in the ```Emulator``` menu. When enabled, opening a ROM while another instance is running will send the ROM to the running instance instead of starting a new one.
- **Debug Symbols**: The emulator automatically tries to load a symbol file when loading a ROM (.sym, .noi). For example, for ```path_to_rom_file.gb``` it tries to load ```path_to_rom_file.sym```. You can also load a symbol file using the GUI or the CLI. It supports RGBDS, GBDK-2020, WLA-DX, no$gmb, SDCC/NoICE (.noi), EQU and generic file formats.

### Command Line Usage
```
gearboy [options] [rom_file] [symbol_file]

Arguments:
  [rom_file]               ROM file: accepts ROMs (.gb, .dmg, .gbc, .cgb, .sgb) or ZIP (.zip)
  [symbol_file]            Optional symbol file for debugging

Options:
  -f, --fullscreen         Start in fullscreen mode
  -w, --windowed           Start in windowed mode with menu visible
      --mcp-stdio          Auto-start MCP server with stdio transport
      --mcp-http           Auto-start MCP server with HTTP transport
      --mcp-http-port N    HTTP port for MCP server (default: 7777)
  -v, --version            Display version information
  -h, --help               Display this help message
```

### MCP Server

Gearboy includes a [Model Context Protocol](https://modelcontextprotocol.io/introduction) (MCP) server that enables AI-assisted debugging through AI agents like GitHub Copilot, Claude, ChatGPT and similar. The server provides tools for execution control, memory inspection, breakpoints, disassembly, hardware status, and more.

For complete setup instructions and tool documentation, see [MCP_README.md](MCP_README.md).

## Build Instructions

### Windows

- Install Microsoft Visual Studio Community 2022 or later.
- Open the Gearboy Visual Studio solution `platforms/windows/Gearboy.sln` and build.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run these commands to generate a Mac *app* bundle:

``` shell
brew install sdl3
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian / Raspberry Pi (Raspbian):

If you are using Ubuntu 25.04 or later, you can install SDL3 directly. Use the following commands to build:

``` shell
sudo apt install build-essential libsdl3-dev libgtk-3-dev
cd platforms/linux
make
```

For older Ubuntu versions (22.04, 24.04), you need to build SDL3 from source first. Use the following commands to build both SDL3 and Gearboy:

``` shell
sudo apt install build-essential cmake libgtk-3-dev
git clone --depth 1 --branch release-3.4.x https://github.com/libsdl-org/SDL.git /tmp/SDL3
cmake -S /tmp/SDL3 -B /tmp/SDL3/build -DCMAKE_INSTALL_PREFIX=/usr -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
cmake --build /tmp/SDL3/build -j$(nproc)
sudo cmake --install /tmp/SDL3/build
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL3-devel gtk3-devel
cd platforms/linux
make
```

- Arch Linux:

``` shell
sudo pacman -S base-devel sdl3 gtk3
cd platforms/linux
make
```

### BSD

- FreeBSD:

``` shell
su root -c "pkg install -y git gmake pkgconf SDL3 lang/gcc gtk3"
cd platforms/bsd
gmake
```

- NetBSD:

``` shell
su root -c "pkgin install gmake pkgconf SDL3 lang/gcc gtk3"
cd platforms/bsd
gmake
```

### Libretro

- Ubuntu / Debian / Raspberry Pi (Raspbian):

``` shell
sudo apt install build-essential
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

[![Contributors](https://contrib.rocks/image?repo=drhelius/gearboy)](https://github.com/drhelius/gearboy/graphs/contributors)

## License

Gearboy is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.

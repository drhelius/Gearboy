# Gearboy

[![Build Status](https://travis-ci.org/drhelius/Gearboy.svg?branch=master)](https://travis-ci.org/drhelius/Gearboy)

Gearboy is a cross-platform Game Boy / GameBoy Color emulator written in C++ that runs on iOS, Raspberry Pi, macOS, Windows, Linux and RetroArch.

This is an open source project with its ongoing development made possible thanks to the support by these awesome [backers](backers.md).

Please, consider [sponsoring](https://github.com/sponsors/drhelius) and following me on [Twitter](https://twitter.com/drhelius) for updates.

----------

## Downloads

- **Windows**: [Gearboy-3.0.4-Windows.zip](https://github.com/drhelius/Gearboy/releases/download/gearboy-3.0.4/Gearboy-3.0.4-Windows.zip)
  + NOTE: You may need to install the [Microsoft Visual C++ Redistributable](https://go.microsoft.com/fwlink/?LinkId=746572)
- **macOS**:
  + `brew cask install gearboy`
  + Or install manually: [Gearboy-3.0.4-macOS.zip](https://github.com/drhelius/Gearboy/releases/download/gearboy-3.0.4/Gearboy-3.0.4-macOS.zip)
- **Linux**: [Gearboy-3.0.4-Linux.tar.xz](https://github.com/drhelius/Gearboy/releases/download/gearboy-3.0.4/Gearboy-3.0.4-Linux.tar.xz)
- **iOS**: Build Gearboy with Xcode and transfer it to your device. You can open rom files from other apps like Safari or Dropbox, or use your iCloud Drive.
- **RetroArch**: [Libretro core documentation](https://docs.libretro.com/library/gearboy/).
- **Raspberry Pi**: Build Gearboy from sources. Optimized projects are provided for Raspberry Pi 1, 2, 3 and 4.

## Features

- Accurate CPU emulation, passes cpu_instrs.gb from blargg's tests.
- Accurate instruction and memory timing, passes instr_timing.gb and mem_timing.gb from blargg's tests.
- Supported cartridges: ROM, ROM + RAM, MBC1, MBC2, MBC3 + RTC, MBC5, HuC-1 and MBC1M (multicart).
- Accurate LCD controller emulation with correct timings and priorities including mid-scanline effects.
- Game Boy Color support.
- LCD screen ghosting effect as seen in the original Game Boy.
- LCD dot matrix effect.
- Sound emulation using SDL Audio and [Gb_Snd_Emu library](http://blargg.8bitalley.com/libs/audio.html#Gb_Snd_Emu).
- Save battery powered RAM cartridges to file.
- Save states.
- Compressed rom support (ZIP).
- Game Genie and GameShark cheat support.
- Supported platforms: Windows, Linux, macOS, Raspberry Pi, iOS and RetroArch (libretro).

## Build Instructions

### Windows

- Install Microsoft Visual Studio Community 2019 or later.
- Open the Gearboy Visual Studio solution `platforms/windows/Gearboy.sln` and build.
- You may want to use the `platforms/windows/Makefile` to build the application using MinGW.

### macOS

- Install Xcode and run `xcode-select --install` in the terminal for the compiler to be available on the command line.
- Run this commands to generate a Mac *app* bundle:

``` shell
brew install sdl2
cd platforms/macos
make dist
```

### Linux

- Ubuntu / Debian:

``` shell
sudo apt-get install build-essential libsdl2-dev libglew-dev
cd platforms/linux
make
```

- Fedora:

``` shell
sudo dnf install @development-tools gcc-c++ SDL2-devel glew-devel
cd platforms/linux
make
```

### iOS

- Install Xcode for macOS. You need iOS 13 SDK or later.
- Build the project `platforms/ios/Gearboy.xcodeproj`
- Run it on real hardware using your iOS developer certificate. Make sure it builds on *Release* for better performance.

### Libretro

- Ubuntu / Debian:

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

### Raspberry Pi 4 - Raspbian (Desktop)

``` shell
sudo apt install build-essential libsdl2-dev libglew-dev
cd platforms/raspberrypi4
make
```

### Raspberry Pi 2 & 3 - Raspbian (CLI)

- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development:

``` shell
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libfreeimage-dev libopenal-dev libpango1.0-dev libsndfile-dev libudev-dev libasound2-dev libjpeg-dev libtiff5-dev libwebp-dev automake
cd ~
wget https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
tar zxvf SDL2-2.0.12.tar.gz
cd SDL2-2.0.12 && mkdir build && cd build
../configure --disable-pulseaudio --disable-esd --disable-video-mir --disable-video-wayland --disable-video-x11 --disable-video-opengl --host=armv7l-raspberry-linux-gnueabihf
make -j 4
sudo make install
```

- Install libconfig library dependencies for development: `sudo apt-get install libconfig++-dev`
- Use `make -j 4` in the `platforms/raspberrypi3/x64/` folder to build the project.
- Use `export SDL_AUDIODRIVER=ALSA` before running the emulator for the best performance.
- Gearboy generates a `gearboy.cfg` configuration file where you can customize keyboard and gamepads. Key codes are from [SDL](https://wiki.libsdl.org/SDL_Keycode).

## Accuracy Tests

Compared to other emulators: [see here](http://tasvideos.org/EmulatorResources/GBAccuracyTests.html).

Tests from [blargg's test roms](https://github.com/retrio/gb-test-roms):

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/gearboy_001.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/gearboy_002.png)![lcd_sync.gb](http://www.geardome.com/files/gearboy/gearboy_003.png)![dmg_sound.gb](http://www.geardome.com/files/gearboy/gearboy_032.png)![cgb_sound.gb](http://www.geardome.com/files/gearboy/gearboy_033.png)![mem_timing.gb](http://www.geardome.com/files/gearboy/gearboy_memtiming2.png)

## Screenshots

![Screenshot](http://www.geardome.com/files/gearboy/gearboy_004.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_006.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_008.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_022.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_013.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_023.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_015.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_029.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_011.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_024.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_017.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_016.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_034.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_026.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_018.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_025.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_021.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_027.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_019.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_020.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_031.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_028.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_007.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_009.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_010.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_005.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_012.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_014.png)

## License

Gearboy is licensed under the GNU General Public License v3.0 License, see [LICENSE](LICENSE) for more information.

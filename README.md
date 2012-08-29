Gearboy
=======
<b>Copyright &copy; 2012 by Ignacio Sanchez</b>

----------

Gearboy is a Nintendo Game Boy emulator written in C++.

The emulator is focused on readability of source code, but nevertheless it has good compatibility.

A lot of effort has gone into this in order to follow OOP and keep it as simple as possible.

----------

Features
--------

- Full CPU emulation, passes cpu_instrs.gb from blargg's tests.
- Accurate instruction timing, passes instr_timing.gb from blargg's tests.
- Full support for most common Memory Bank Controllers (MBC1, MBC2, MBC3, MBC5) and ROM + RAM cartridges.
- Accurate emulation of LCD controller. Full support for background, window and sprites, with correct timings and priorities.
- Sound emulation using SDL Audio and [Gb_Snd_Emu library](http://www.slack.net/~ant/libs/audio.html#Gb_Snd_Emu).
- Basic Game Boy Color support.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Uses Qt framework for GUI and OpenGL for rendering.
- Multi platform. Compiles and runs on Windows, Linux and Mac OS X.
- Visual Studio 2010 project provided for Windows. Netbeans 7.2 project provided for Linux and Mac OS X.

Things left
-----------

- Improve GUI.
- Improve Game Boy Color support.
- Save RAM battery to disk.
- Add RTC support.
- Improve timing and compatibility.
- Cross device iOS version.
- Support for compressed roms.

Compiling Instructions
----------------------

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all platforms you will need SDL and Qt Framework SDKs installed and configured.

There is a nice Netbeans + Qt tutorial [here](http://netbeans.org/kb/docs/cnd/qt-applications.html).

### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt SDK for Windows](http://qt.nokia.com/downloads/sdk-windows-cpp).
- Install the Qt Visual Studio Add-in and point it to the [Qt SDK](http://qt.nokia.com/downloads/visual-studio-add-in).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearboy Visual Studio project and build.

### Linux
- You need at least Netbeans 7.2 for C++.
- Install Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL development dependencies (Ubuntu: <code>sudo apt-get install libsdl1.2-dev</code>).
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make</code> to build the project.
- In Ubuntu 12.04 I had to <code>set export SDL_AUDIODRIVER=ALSA</code> for the sound to work properly.

### Mac OS X
- You need at least Netbeans 7.2 for C++.
- Install Xcode for the compiler to be available on the command line.
- Install the [Qt SDK for Mac OS](http://qt.nokia.com/downloads/sdk-mac-os-cpp).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearboy Netbeans project and build. This project is configured for using <code>clang</code>.
- Alternatively you can use <code>make</code> to build the project.

Passed Tests
------------

This tests are from [blargg's test roms](http://blargg.parodius.com/gb-tests/).

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/gearboy_1.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/gearboy_2.png)![lcd_sync.gb](http://www.geardome.com/files/gearboy/gearboy_3.png)

Screenshots
-----------

![Screenshot](http://www.geardome.com/files/gearboy/gearboy_4.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_5.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_6.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_7.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_8.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_9.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_10.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_11.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_12.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_13.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_14.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_15.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_16.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_17.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_18.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_19.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_20.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_21.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_22.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_23.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_24.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_25.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_26.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_27.png)
![Screenshot](http://www.geardome.com/files/gearboy/gearboy_28.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_29.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_31.png)

License
-------

<i>Gearboy - Nintendo Game Boy Emulator</i>

<i>Copyright (C) 2012  Ignacio Sanchez</i>

<i>This program is free software: you can redistribute it and/or modify</i>
<i>it under the terms of the GNU General Public License as published by</i>
<i>the Free Software Foundation, either version 3 of the License, or</i>
<i>any later version.</i>

<i>This program is distributed in the hope that it will be useful,</i>
<i>but WITHOUT ANY WARRANTY; without even the implied warranty of</i>
<i>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the</i>
<i>GNU General Public License for more details.</i>

<i>You should have received a copy of the GNU General Public License</i>
<i>along with this program.  If not, see http://www.gnu.org/licenses/</i>

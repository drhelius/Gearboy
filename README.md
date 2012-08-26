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
- Basic Game Boy Color support.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Uses Qt framework for GUI and OpenGL for rendering.
- Multi platform. Compiles and runs on Windows, Linux and Mac OS.
- Visual Studio 2010 project provided for Windows. Netbeans 7.2 project provided for Linux and Mac OS.

Things left
-----------

- Improve GUI.
- Add sound support.
- Improve Game Boy Color support.
- Save RAM battery to disk.
- Add RTC support.
- Improve timing and compatibility.
- Cross device iOS version.

Compiling Instructions
----------------------

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all platforms you will need the Qt Framework SDK installed and configured.

There is a nice Netbeans + Qt tutorial here: http://netbeans.org/kb/docs/cnd/qt-applications.html

### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the Qt SDK for Windows (http://qt.nokia.com/downloads/sdk-windows-cpp).
- Install the Qt Visual Studio Add-in and point it to the Qt SDK (http://qt.nokia.com/downloads/visual-studio-add-in).
- Open the Gearboy Visual Studio project and build.

### Linux
- You need at least Netbeans 7.2 for C++.
- Install the Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install the OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make</code> to build the project.

### Mac OS
- You need at least Netbeans 7.2 for C++.
- Install Xcode for the compiler to be available on the command line.
- Install the Qt SDK for Mac OS (http://qt.nokia.com/downloads/sdk-mac-os-cpp).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed). 
- Open the Gearboy Netbeans project and build. This project is configured for using <code>clang</code>.
- Alternatively you can use <code>make</code> to build the project.

Passed Tests
------------

This tests are from blargg's test roms (http://blargg.parodius.com/gb-tests/)

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/12.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/13.png) 
![lcd_sync.gb](http://www.geardome.com/files/gearboy/14.png)

Screenshots
-----------

![Screenshot](http://www.geardome.com/files/gearboy/1.png)![Screenshot](http://www.geardome.com/files/gearboy/2.png)
![Screenshot](http://www.geardome.com/files/gearboy/3.png)![Screenshot](http://www.geardome.com/files/gearboy/4.png)
![Screenshot](http://www.geardome.com/files/gearboy/5.png)![Screenshot](http://www.geardome.com/files/gearboy/6.png)
![Screenshot](http://www.geardome.com/files/gearboy/7.png)![Screenshot](http://www.geardome.com/files/gearboy/8.png)
![Screenshot](http://www.geardome.com/files/gearboy/9.png)![Screenshot](http://www.geardome.com/files/gearboy/10.png)
![Screenshot](http://www.geardome.com/files/gearboy/11.png)

License
-------

<i>
Gearboy - Nintendo Game Boy Emulator
Copyright (C) 2012  Ignacio Sanchez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/ 
</i>
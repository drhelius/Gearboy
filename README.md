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
- Full support for most common Memory Bank Controllers (ROM, MBC1, MBC2, MBC3, MBC5)
- Accurate emulation of LCD controller. Full support for background, window and sprites with correct timings and priorities.
- Integrated disassembler. It can dump the full disassembled memory to a text file.
- Uses Qt framework for GUI. Same appearance in Windows, Linux and Mac OS.
- Multi platform. Compiles and runs on Windows, Linux and Mac OS X.
- Visual Studio 2010 project provided for Windows. Netbeans 7.2 project provided for Linux and Mac OS.

Things left
-----------

- Add sound support.
- Add Game Boy Color support
- Save RAM battery to disk.
- Add RTC support.
- Improve timing and compatibility.
- Use QT for GUI.
- Cross device iOS version

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

<i>This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation. The full license is available at http://www.gnu.org/licenses/gpl.html This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</i>
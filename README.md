Gearboy
=======
<b>Copyright &copy; 2012 by Ignacio Sanchez</b>

----------

Gearboy is a Nintendo Game Boy / GameBoy Color emulator written in C++ that runs on iOS, Raspberry Pi, Mac, Windows and Linux.

The main focus of this emulator is readability of source code with very high compatibility.

Follow me on Twitter for updates: http://twitter.com/drhelius

If you want new features ask for them but don't forget donating, thanks :)

[![PayPal - The safer, easier way to pay online!](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=28YUTJVAH7JH8 "PayPal - The safer, easier way to pay online!")

----------

Downloads
--------
- Gearboy 2.0 for Jailbroken iOS: [Cydia](http://modmyi.com/info/gearboygameboy.d.php). You can open rom files from other apps like Safari or Dropbox. They can be placed in <code>/var/mobile/Media/ROMs/GAMEBOY</code> too. Save files are placed in <code>/var/mobile/Library/Gearboy</code>
- Gearboy 2.0 for Non-Jailbroken iOS: You can open rom files from other apps like Safari or Dropbox, or use [iTunes file sharing](http://support.apple.com/kb/ht4094). 
- Gearboy 1.6 for Windows: [Gearboy-1.6-Windows.zip](http://www.geardome.com/files/gearboy/Gearboy-1.6-Windows.zip) (NOTE: You may need to install the [Microsoft Visual C++ Redistributable](http://www.microsoft.com/en-us/download/details.aspx?id=40784))
- Gearboy 1.6 for Linux: [Gearboy-1.6-Linux.tar.gz](http://www.geardome.com/files/gearboy/Gearboy-0.8-Linux.tar.gz)

Features
--------
- Highly accurate CPU emulation, passes cpu_instrs.gb from blargg's tests.
- Accurate instruction timing, passes instr_timing.gb from blargg's tests.
- Memory Bank Controllers (MBC1, MBC2, MBC3 with RTC, MBC5), ROM + RAM and multicart cartridges.
- Accurate LCD controller emulation. Background, window and sprites, with correct timings and priorities.
- Mix frames: Mimics the LCD ghosting effect seen in the original Game Boy.
- Sound emulation using SDL Audio and [Gb_Snd_Emu library](http://slack.net/~ant/libs/audio.html#Gb_Snd_Emu).
- Game Boy Color support.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Saves battery powered RAM cartridges to file.
- Compressed rom support (ZIP deflate).
- Multi platform. Runs on Windows, Linux, Mac OS X, Raspberry Pi and iOS.

Todo List
---------
- Saving and loading game states (only desktop).
- Pixel precision scan line timing (https://gist.github.com/3730564).
- Debugger.

Save Files Note
---------------
If you don't have access to an Apple developer account and are using older versions of Gearboy (such as from emu4ios), you can use a hex editor to convert between a .gearboy file and a .sav file. 
In particular, a .gearboy file has about 40 extra bytes at the beginning of the file. If you remove these 40 bytes and rename to a .sav extension the file should be compatible with other emulators. You can also reinsert the 40 bytes to convert a .sav file to .gearboy. This should work for Pokemon RBY. 
If you are still having problems, see if you can find an SRAM map for your game and double check that the offset matches.

Compiling Instructions
----------------------

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all desktop platforms you will need SDL 2 and Qt 5 SDKs installed and configured.

### iOS
- Install Xcode for Mac OS X. You need iOS SDK 8 or later. 
- Build the project. 
- Run it on real hardware using your iOS developer certificate. Be sure to compile it on Release for extra optimizations. For jailbroken devices use the jailbreak branch.

### Raspberry Pi - Raspbian
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development.
- Use <code>make</code> to build the project.
- Sound emulation in the Pi is awfully slow. Use <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator and over clock your Raspberry as much as you can for the best performance.

### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt 5 SDK for Windows](http://qt-project.org/downloads).
- Install the [Qt 5 Visual Studio Add-in](http://qt-project.org/downloads) and point it to the Qt SDK.
- Install and configure [SDL 2](http://www.libsdl.org/download-2.0.php) for development.
- In order to use OpenGL extensions I used [GLEW](http://glew.sourceforge.net/). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions. Make sure the GLEW headers and libs are configured within VC++.
- Open the Gearboy Visual Studio project and build.

### Mac OS X
- You need Qt Creator, included in the Qt 5 SDK.
- Install Xcode and run <code>xcode-select --install</code> in the terminal for the compiler to be available on the command line.
- Install the [Qt 5 SDK for Mac OS](http://qt-project.org/downloads).
- Download [SDL 2](http://www.libsdl.org/download-2.0.php) source code. Then run <code>.configure</code> <code>make</code> <code>sudo make install</code> on the terminal.
- Open the Gearboy Qt project and build.

### Linux
- You need Netbeans 7.3 or later.
- Install Qt 5 development dependencies (Ubuntu: <code>sudo apt-get install qt5-default qttools5-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL 2 development dependencies (Ubuntu: <code>sudo apt-get install libsdl2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.8-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 13.10 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

Accuracy Tests
------------
Compared to other emulators: [see here](http://tasvideos.org/EmulatorResources/GBAccuracyTests.html). 

Tests from [blargg's test roms](http://slack.net/~ant/old/gb-tests/):

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/gearboy_001.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/gearboy_002.png)![lcd_sync.gb](http://www.geardome.com/files/gearboy/gearboy_003.png)![dmg_sound.gb](http://www.geardome.com/files/gearboy/gearboy_032.png)![cgb_sound.gb](http://www.geardome.com/files/gearboy/gearboy_033.png)

Screenshots
-----------

![Screenshot](http://www.geardome.com/files/gearboy/gearboy_004.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_006.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_008.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_022.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_013.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_023.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_015.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_029.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_011.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_024.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_017.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_016.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_034.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_026.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_018.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_025.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_021.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_027.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_019.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_020.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_031.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_028.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_007.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_009.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_010.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_005.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_012.png)![Screenshot](http://www.geardome.com/files/gearboy/gearboy_014.png)

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


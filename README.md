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
- Gearboy 1.4 for Jailbroken iOS: [Cydia](http://modmyi.com/info/gearboygameboy.d.php). You can open rom files from other apps like Safari or Dropbox.
- Gearboy 1.4 for Non-Jailbroken iOS: Use your developer certificate to compile and install it, then you can open rom files from other apps or use [iTunes file sharing](http://support.apple.com/kb/ht4094). 
- Gearboy 0.8 for Windows: [Gearboy-0.8-Windows.zip](http://www.geardome.com/files/gearboy/Gearboy-0.8-Windows.zip)
- Gearboy 0.8 for Linux: [Gearboy-0.8-Linux.tar.gz](http://www.geardome.com/files/gearboy/Gearboy-0.8-Linux.tar.gz)

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
-----------
- Saving and loading game states (only desktop).
- Pixel precision scan line timing (https://gist.github.com/3730564).
- Debugger.

Compiling Instructions
----------------------

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all desktop platforms you will need SDL and Qt 4 SDKs installed and configured. SDL is provided as a framework for iOS.

There is a nice Netbeans + Qt tutorial [here](http://netbeans.org/kb/docs/cnd/qt-applications.html).

### iOS
- Install Xcode for Mac OS X. You need iOS SDK 5.1 or later. 
- Open the Gearboy Xcode project and build.
- Run it on real hardware using your iOS developer certificate. For jailbroken devices use the jailbreak branch.

### Raspberry Pi - Raspbian
- Install SDL development dependencies (<code>sudo apt-get install libsdl1.2-dev</code>).
- Use <code>make</code> to build the project.

### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt 4 SDK for Windows](http://qt-project.org/downloads).
- Install the [Qt 4 Visual Studio Add-in](http://qt-project.org/downloads) and point it to the Qt SDK.
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- In order to use OpenGL extensions I used [GLEW](http://glew.sourceforge.net/). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions. Make sure the GLEW headers and libs are configured within VC++.
- Open the Gearboy Visual Studio project and build.

### Mac OS X
- You need Netbeans 7.3 or later.
- Install Xcode for the compiler to be available on the command line.
- Install the [Qt 4 SDK for Mac OS](http://qt-project.org/downloads).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearboy Netbeans project and build. The project will use <code>clang</code>.
- Alternatively, you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.

### Linux
- You need Netbeans 7.3 or later.
- Install Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL development dependencies (Ubuntu: <code>sudo apt-get install libsdl1.2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.6-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 12.04 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

Accuracy Tests
------------
Compared to other emulators: [see here](http://tasvideos.org/EmulatorResources/GBAccuracyTests.html). 

Tests from [blargg's test roms](http://slack.net/~ant/old/gb-tests/):

![cpu_instrs.gb](http://www.geardome.com/files/gearboy/gearboy_1.png)![insrt_timing.gb](http://www.geardome.com/files/gearboy/gearboy_2.png)![lcd_sync.gb](http://www.geardome.com/files/gearboy/gearboy_3.png)
![dmg_sound.gb](http://www.geardome.com/files/gearboy/gearboy_32.png)![cgb_sound.gb](http://www.geardome.com/files/gearboy/gearboy_33.png)

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

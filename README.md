Gearboy
=======
<b>Copyright &copy; 2012 by Ignacio Sanchez</b>

----------

Gearboy is a Nintendo Game Boy / Game Boy Color emulator written in C++.

The main focus of this emulator is readability of source code. Nevertheless, it has a high compatibility ratio.

A lot of effort has gone into this in order to follow OOP and keep it as simple and efficient as possible.

Don't forget sending me your comments or questions at: http://twitter.com/drhelius


----------

Donations
--------
Gearboy is free but it's a lot of work, donations are really appreciated, thanks! :)

[![PayPal - The safer, easier way to pay online!](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=28YUTJVAH7JH8 "PayPal - The safer, easier way to pay online!")

Downloads
--------
- Windows: [Gearboy-0.5-Windows.zip](http://www.geardome.com/files/gearboy/Gearboy-0.5-Windows.zip)
- Linux: [Gearboy-0.5-Linux.tar.gz](http://www.geardome.com/files/gearboy/Gearboy-0.5-Linux.tar.gz)
- iOS (jailbroken): [Cydia](http://modmyi.com/info/gearboygameboy.d.php)
- iOS (non-jailbroken): [Install Now](http://macbuildserver.com/project/github/build/?xcode_project=platforms%2Fios%2FGearboy.xcodeproj&amp;target=Gearboy&amp;repo_url=git%3A%2F%2Fgithub.com%2Fdrhelius%2FGearboy.git&amp;build_conf=Release)

Features
--------
- Accurate CPU emulation, passes cpu_instrs.gb from blargg's tests.
- Accurate instruction timing, passes instr_timing.gb from blargg's tests.
- Memory Bank Controllers (MBC1, MBC2, MBC3 with RTC, MBC5), ROM + RAM and multicart cartridges.
- Accurate LCD controller emulation. Background, window and sprites, with correct timings and priorities.
- Mix frames: Mimics the LCD ghosting effect seen in the original Game Boy.
- Sound emulation using SDL Audio and [Gb_Snd_Emu library](http://slack.net/~ant/libs/audio.html#Gb_Snd_Emu).
- Game Boy Color support.
- Integrated disassembler. It can dump the full disassembled memory to a text file or access it in real time.
- Saves battery powered RAM cartridges to file.
- Compressed rom support (ZIP deflate).
- Multi platform. Compiles and runs on Windows, Linux, Mac OS X, Raspberry Pi and iOS.
- Uses OpenGL for rendering on all platforms.
- Uses Qt framework for Mac, Windows and Linux. Uses Cocoa Touch for iPad, iPhone and iPod touch.
- Visual Studio 2010 project provided for Windows. Netbeans 7.2 project provided for Linux and Mac OS X. Xcode project for iOS.

Todo List
-----------
- iOS landscape mode.
- Saving and loading game states.
- Pixel precision scan line timing (https://gist.github.com/3730564).
- Debugger.

Compiling Instructions
----------------------

The best way of compiling Gearboy is by using one of the IDE projects provided for each platform.

For all desktop platforms you will need SDL and Qt Framework SDKs installed and configured. SDL is provided as a framework for iOS project.

There is a nice Netbeans + Qt tutorial [here](http://netbeans.org/kb/docs/cnd/qt-applications.html).

### Windows
- You need Visual Studio 2010 (Express Edition will do but you won't be able to install the Qt Add-in).
- Install the [Qt SDK for Windows](http://qt-project.org/downloads).
- Install the [Qt Visual Studio Add-in](http://qt-project.org/downloads) and point it to the Qt SDK.
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- In order to use OpenGL extensions I used [GLEW](http://glew.sourceforge.net/). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions. Make sure the GLEW headers and libs are configured within VC++.
- Open the Gearboy Visual Studio project and build.

### Linux
- You need at least Netbeans 7.2 for C++.
- Install Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL development dependencies (Ubuntu: <code>sudo apt-get install libsdl1.2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.6-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 12.04 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

### Mac OS X
- You need at least Netbeans 7.2 for C++.
- Install Xcode for the compiler to be available on the command line.
- Install the [Qt SDK for Mac OS](http://qt-project.org/downloads).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearboy Netbeans project and build. This project is configured for using <code>clang</code>.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.

### iOS
- Install Xcode for Mac OS X. iOS SDK 5.1 or later is needed. 
- Open the Gearboy Xcode project and build.
- In order to run it on real hardware you will need an iOS developer certificate. For jailbroken devices use the jailbreak branch.

### Raspberry Pi - Raspbian
- Install SDL development dependencies (<code>sudo apt-get install libsdl1.2-dev</code>).
- Use <code>make</code> to build the project.

Passed Tests
------------

This tests are from [blargg's test roms](http://slack.net/~ant/old/gb-tests/).

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

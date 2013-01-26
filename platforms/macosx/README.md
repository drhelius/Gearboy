Gearboy
=======
<b>Copyright &copy; 2012 by Ignacio Sanchez</b>

----------

Gearboy is a Nintendo Game Boy / Game Boy Color emulator written in C++.

The emulator is focused on readability of source code, but nevertheless it has good compatibility.

A lot of effort has gone into this in order to follow OOP and keep it as simple as possible.

For any comments or questions I can be reached at: http://twitter.com/drhelius

----------

Compiling Instructions
----------------------

### Mac OS X
- You need at least Netbeans 7.2 for C++.
- Install Xcode for the compiler to be available on the command line.
- Install the [Qt SDK for Mac OS](http://qt-project.org/downloads).
- Add <code>qmake</code> to the PATH (You can find qmake in the bin directory where you have Qt SDK installed).
- Install and configure [SDL](http://www.libsdl.org/download-1.2.php) for development.
- Open the Gearboy Netbeans project and build. This project is configured for using <code>clang</code>.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.

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

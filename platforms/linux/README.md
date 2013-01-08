Gearboy
=======
<b>Copyright &copy; 2012 by Ignacio Sanchez</b>

----------

Gearboy is a Nintendo Game Boy emulator written in C++.

The emulator is focused on readability of source code, but nevertheless it has good compatibility.

A lot of effort has gone into this in order to follow OOP and keep it as simple as possible.

----------

Compiling Instructions
----------------------

### Linux
- You need at least Netbeans 7.2 for C++.
- Install Qt development dependencies (Ubuntu: <code>sudo apt-get install qt4-dev-tools</code>).
- Install OpenGL development dependencies (Ubuntu: <code>sudo apt-get install freeglut3-dev</code>).
- Install SDL development dependencies (Ubuntu: <code>sudo apt-get install libsdl1.2-dev</code>).
- In order to use OpenGL extensions I used GLEW dependencies (Ubuntu: <code>sudo apt-get install libglew1.6-dev</code>). This is because of a [bug](http://stackoverflow.com/questions/11845230/glgenbuffers-crashes-in-release-build) in QGLFunctions.
- Open the Gearboy Netbeans project and build.
- Alternatively you can use <code>make -f nbproject/Makefile-Release.mk SUBPROJECTS= .build-conf</code> to build the project.
- In Ubuntu 12.04 I had to <code>export SDL_AUDIODRIVER=ALSA</code> before running the emulator for the sound to work properly.

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

Gb_Snd_Emu 0.1.4: Game Boy Sound Emulator
-----------------------------------------
This is a portable Nintendo Game Boy PAPU sound chip emulator library for use
in a Game Boy emulator. Its main features are good sound quality and
efficiency.

Licensed under the GNU Lesser General Public License (LGPL); see LGPL.TXT.
Copyright (C) 2003-2005 Shay Green.

Website: http://www.slack.net/~ant/libs/
Forum  : http://groups-beta.google.com/group/blargg-sound-libs
Contact: Shay Green <hotpop.com@blargg> (swap to e-mail)


Getting Started
---------------
Build a program consisting of demo.cpp, Basic_Gb_Apu.cpp, and all source files
in the gb_apu/ directory. Running the program should generate a WAVE sound file
"out.wav" of random tones.

See notes.txt for more information, and respective header (.h) files for
reference. Visit the discussion forum to get assistance.


Files
-----
notes.txt               General notes about the library
changes.txt             Changes since previous releases
LGPL.TXT                GNU Lesser General Public License

demo.cpp                How to use Basic_Gb_Apu
Basic_Gb_Apu.h          Simplified Game Boy sound chip emulator
Basic_Gb_Apu.cpp

demo_sdl.cpp            How to use Basic_Gb_Apu with SDL sound
Sound_Queue.h           Makes SDL sound act like file you can write to
Sound_Queue.cpp

usage.txt               How to use full Gb_Apu in an emulator

gb_apu/
  Gb_Apu.h              Game Boy sound chip emulator
  Gb_Apu.cpp
  Gb_Oscs.cpp
  Gb_Oscs.h
  Multi_Buffer.h        Stereo_Buffer
  Multi_Buffer.cpp
  blargg_common.h       Common services
  blargg_source.h
  Blip_Synth.h          Sound synthesis buffer
  Blip_Buffer.h
  Blip_Buffer.cpp

boost/                  Substitute for boost library if it's unavailable

Wave_Writer.h           WAVE sound file writer used for demo output
Wave_Writer.cpp

-- 
Shay Green <hotpop.com@blargg> (swap to e-mail)

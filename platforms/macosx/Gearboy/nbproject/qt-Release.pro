# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Release/CLang-MacOSX
TARGET = Gearboy
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += release 
PKGCONFIG +=
QT = core gui opengl
SOURCES += ../../../src/Audio.cpp ../../../src/Cartridge.cpp ../../../src/CommonMemoryRule.cpp ../../../src/GearboyCore.cpp ../../../src/IORegistersMemoryRule.cpp ../../../src/Input.cpp ../../../src/MBC1MemoryRule.cpp ../../../src/MBC2MemoryRule.cpp ../../../src/MBC3MemoryRule.cpp ../../../src/MBC5MemoryRule.cpp ../../../src/Memory.cpp ../../../src/MemoryRule.cpp ../../../src/MultiMBC1MemoryRule.cpp ../../../src/Processor.cpp ../../../src/RomOnlyMemoryRule.cpp ../../../src/Video.cpp ../../../src/audio/Blip_Buffer.cpp ../../../src/audio/Effects_Buffer.cpp ../../../src/audio/Gb_Apu.cpp ../../../src/audio/Gb_Apu_State.cpp ../../../src/audio/Gb_Oscs.cpp ../../../src/audio/Multi_Buffer.cpp ../../../src/audio/Sound_Queue.cpp ../../../src/opcodes.cpp ../../../src/opcodes_cb.cpp ../../qt-shared/About.cpp ../../qt-shared/Emulator.cpp ../../qt-shared/GLFrame.cpp ../../qt-shared/InputSettings.cpp ../../qt-shared/MainWindow.cpp ../../qt-shared/RenderThread.cpp ../../qt-shared/SoundSettings.cpp ../../qt-shared/VideoSettings.cpp ../../qt-shared/main.cpp
HEADERS += ../../../src/Audio.h ../../../src/Cartridge.h ../../../src/CommonMemoryRule.h ../../../src/EightBitRegister.h ../../../src/GearboyCore.h ../../../src/IORegistersMemoryRule.h ../../../src/Input.h ../../../src/MBC1MemoryRule.h ../../../src/MBC2MemoryRule.h ../../../src/MBC3MemoryRule.h ../../../src/MBC5MemoryRule.h ../../../src/Memory.h ../../../src/MemoryRule.h ../../../src/Memory_inline.h ../../../src/MultiMBC1MemoryRule.h ../../../src/Processor.h ../../../src/Processor_inline.h ../../../src/RomOnlyMemoryRule.h ../../../src/SixteenBitRegister.h ../../../src/Video.h ../../../src/audio/Blip_Buffer.h ../../../src/audio/Blip_Synth.h ../../../src/audio/Effects_Buffer.h ../../../src/audio/Gb_Apu.h ../../../src/audio/Gb_Oscs.h ../../../src/audio/Multi_Buffer.h ../../../src/audio/Sound_Queue.h ../../../src/audio/blargg_common.h ../../../src/audio/blargg_config.h ../../../src/audio/blargg_source.h ../../../src/boot_roms.h ../../../src/definitions.h ../../../src/gearboy.h ../../../src/opcode_names.h ../../../src/opcode_timing.h ../../qt-shared/About.h ../../qt-shared/Emulator.h ../../qt-shared/GLFrame.h ../../qt-shared/InputSettings.h ../../qt-shared/MainWindow.h ../../qt-shared/RenderThread.h ../../qt-shared/SoundSettings.h ../../qt-shared/VideoSettings.h
FORMS += ../../qt-shared/About.ui ../../qt-shared/InputSettings.ui ../../qt-shared/MainWindow.ui ../../qt-shared/SoundSettings.ui ../../qt-shared/VideoSettings.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Release/CLang-MacOSX
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = clang
QMAKE_CXX = clang++
DEFINES += 
INCLUDEPATH += 
LIBS += -lSDLmain -lSDL -Wl,-framework,Cocoa  

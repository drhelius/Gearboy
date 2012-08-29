# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Debug/GNU-Linux-x86
TARGET = Gearboy
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
PKGCONFIG +=
QT = core gui opengl
SOURCES += ../../../src/MBC2MemoryRule.cpp ../../../src/Audio.cpp ../../../src/MBC1MemoryRule.cpp ../../../src/IORegistersMemoryRule.cpp ../../../src/audio/Gb_Apu.cpp ../../qt-shared/RenderThread.cpp ../../qt-shared/SoundSettings.cpp ../../../src/GearboyCore.cpp ../../../src/MemoryCell.cpp ../../qt-shared/GLFrame.cpp ../../../src/audio/Multi_Buffer.cpp ../../../src/audio/Effects_Buffer.cpp ../../../src/MBC5MemoryRule.cpp ../../../src/audio/Gb_Apu_State.cpp ../../qt-shared/MainWindow.cpp ../../../src/SixteenBitRegister.cpp ../../../src/audio/Blip_Buffer.cpp ../../../src/EightBitRegister.cpp ../../qt-shared/main.cpp ../../../src/MemoryRule.cpp ../../../src/Input.cpp ../../../src/Processor.cpp ../../../src/Memory.cpp ../../../src/Cartridge.cpp ../../../src/MBC3MemoryRule.cpp ../../../src/Video.cpp ../../../src/RomOnlyMemoryRule.cpp ../../qt-shared/VideoSettings.cpp ../../qt-shared/Emulator.cpp ../../../src/audio/Sound_Queue.cpp ../../../src/audio/Gb_Oscs.cpp ../../../src/opcodes.cpp ../../../src/opcodes_cb.cpp ../../qt-shared/InputSettings.cpp
HEADERS += ../../../src/definitions.h ../../../src/IORegistersMemoryRule.h ../../../src/MemoryCell.h ../../../src/EightBitRegister.h ../../../src/Memory.h ../../qt-shared/VideoSettings.h ../../../src/Input.h ../../../src/Video.h ../../../src/MemoryRule.h ../../../src/MBC2MemoryRule.h ../../qt-shared/SoundSettings.h ../../../src/SixteenBitRegister.h ../../../src/MBC1MemoryRule.h ../../../src/GearboyCore.h ../../../src/gearboy.h ../../../src/audio/Effects_Buffer.h ../../../src/RomOnlyMemoryRule.h ../../../src/audio/Multi_Buffer.h ../../../src/audio/Blip_Synth.h ../../../src/Processor.h ../../../src/audio/Gb_Oscs.h ../../../src/audio/Blip_Buffer.h ../../qt-shared/Emulator.h ../../../src/MBC5MemoryRule.h ../../../src/MBC3MemoryRule.h ../../../src/audio/blargg_common.h ../../../src/audio/Sound_Queue.h ../../../src/Cartridge.h ../../../src/audio/blargg_source.h ../../../src/opcode_names.h ../../../src/opcode_timing.h ../../../src/Audio.h ../Gearboy/ui_MainWindow.h ../../../src/boot_roms.h ../../../src/audio/Gb_Apu.h ../../qt-shared/MainWindow.h ../../qt-shared/RenderThread.h ../../../src/audio/blargg_config.h ../../qt-shared/InputSettings.h ../../qt-shared/GLFrame.h
FORMS += ../../qt-shared/VideoSettings.ui ../../qt-shared/InputSettings.ui ../../qt-shared/SoundSettings.ui ../../qt-shared/MainWindow.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Debug/GNU-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += 
LIBS += -lGLU -lSDL  

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
SOURCES += ../../../src/MBC2MemoryRule.cpp ../../../src/Audio.cpp ../../../src/MBC1MemoryRule.cpp ../../../src/IORegistersMemoryRule.cpp ../../qt-shared/RenderThread.cpp ../../../src/GearboyCore.cpp ../../../src/MemoryCell.cpp ../../qt-shared/GLFrame.cpp ../../../src/MBC5MemoryRule.cpp ../../qt-shared/MainWindow.cpp ../../../src/SixteenBitRegister.cpp ../../qt-shared/main.cpp ../../../src/EightBitRegister.cpp ../../../src/MemoryRule.cpp ../../../src/Input.cpp ../../../src/Processor.cpp ../../../src/Video.cpp ../../../src/Memory.cpp ../../../src/Cartridge.cpp ../../../src/MBC3MemoryRule.cpp ../../../src/RomOnlyMemoryRule.cpp ../../qt-shared/Emulator.cpp ../../../src/opcodes.cpp ../../../src/opcodes_cb.cpp
HEADERS += ../../../src/definitions.h ../../../src/IORegistersMemoryRule.h ../../../src/MemoryCell.h ../../../src/EightBitRegister.h ../../../src/Memory.h ../../../src/Input.h ../../../src/Video.h ../../../src/MemoryRule.h ../../../src/MBC2MemoryRule.h ../../../src/SixteenBitRegister.h ../../../src/MBC1MemoryRule.h ../../../src/GearboyCore.h ../../../src/gearboy.h ../../../src/RomOnlyMemoryRule.h ../../../src/Processor.h ../../qt-shared/Emulator.h ../../../src/MBC5MemoryRule.h ../../../src/MBC3MemoryRule.h ../../../src/opcode_names.h ../../../src/Cartridge.h ../../../src/opcode_timing.h ../../../src/Audio.h ../../../src/boot_roms.h ../../qt-shared/MainWindow.h ../../qt-shared/RenderThread.h ../../qt-shared/GLFrame.h
FORMS += ../../qt-shared/MainWindow.ui
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
LIBS += 

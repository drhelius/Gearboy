#-------------------------------------------------
#
# Project created by QtCreator 2014-08-15T03:33:23
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += opengl

TARGET = Gearboy
TEMPLATE = app

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/lib

LIBS += -stdlib=libc++ -L/usr/local/lib -lSDL2

QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    ../../../src/audio/Blip_Buffer.cpp \
    ../../../src/audio/Effects_Buffer.cpp \
    ../../../src/audio/Gb_Apu_State.cpp \
    ../../../src/audio/Gb_Apu.cpp \
    ../../../src/audio/Gb_Oscs.cpp \
    ../../../src/audio/Multi_Buffer.cpp \
    ../../audio-shared/Sound_Queue.cpp \
    ../../../src/Audio.cpp \
    ../../../src/Cartridge.cpp \
    ../../../src/CommonMemoryRule.cpp \
    ../../../src/GearboyCore.cpp \
    ../../../src/Input.cpp \
    ../../../src/IORegistersMemoryRule.cpp \
    ../../../src/MBC1MemoryRule.cpp \
    ../../../src/MBC2MemoryRule.cpp \
    ../../../src/MBC3MemoryRule.cpp \
    ../../../src/MBC5MemoryRule.cpp \
    ../../../src/Memory.cpp \
    ../../../src/MemoryRule.cpp \
    ../../../src/MultiMBC1MemoryRule.cpp \
    ../../../src/opcodes_cb.cpp \
    ../../../src/opcodes.cpp \
    ../../../src/Processor.cpp \
    ../../../src/RomOnlyMemoryRule.cpp \
    ../../../src/Video.cpp \
    ../../../src/miniz/miniz.c \
    ../../qt-shared/About.cpp \
    ../../qt-shared/Emulator.cpp \
    ../../qt-shared/GLFrame.cpp \
    ../../qt-shared/InputSettings.cpp \
    ../../qt-shared/main.cpp \
    ../../qt-shared/MainWindow.cpp \
    ../../qt-shared/RenderThread.cpp \
    ../../qt-shared/SoundSettings.cpp \
    ../../qt-shared/VideoSettings.cpp

HEADERS  += \
    ../../../src/audio/blargg_common.h \
    ../../../src/audio/blargg_config.h \
    ../../../src/audio/blargg_source.h \
    ../../../src/audio/Blip_Buffer.h \
    ../../../src/audio/Blip_Synth.h \
    ../../../src/audio/Effects_Buffer.h \
    ../../../src/audio/Gb_Apu.h \
    ../../../src/audio/Gb_Oscs.h \
    ../../../src/audio/Multi_Buffer.h \
    ../../audio-shared/Sound_Queue.h \
    ../../../src/Audio.h \
    ../../../src/Cartridge.h \
    ../../../src/CommonMemoryRule.h \
    ../../../src/definitions.h \
    ../../../src/EightBitRegister.h \
    ../../../src/gearboy.h \
    ../../../src/GearboyCore.h \
    ../../../src/Input.h \
    ../../../src/IORegistersMemoryRule.h \
    ../../../src/MBC1MemoryRule.h \
    ../../../src/MBC2MemoryRule.h \
    ../../../src/MBC3MemoryRule.h \
    ../../../src/MBC5MemoryRule.h \
    ../../../src/Memory_inline.h \
    ../../../src/Memory.h \
    ../../../src/MemoryRule.h \
    ../../../src/MultiMBC1MemoryRule.h \
    ../../../src/opcode_names.h \
    ../../../src/opcode_timing.h \
    ../../../src/Processor_inline.h \
    ../../../src/Processor.h \
    ../../../src/RomOnlyMemoryRule.h \
    ../../../src/SixteenBitRegister.h \
    ../../../src/Video.h \
    ../../qt-shared/About.h \
    ../../qt-shared/Emulator.h \
    ../../qt-shared/GLFrame.h \
    ../../qt-shared/InputSettings.h \
    ../../qt-shared/MainWindow.h \
    ../../qt-shared/RenderThread.h \
    ../../qt-shared/SoundSettings.h \
    ../../qt-shared/VideoSettings.h

FORMS    += \
    ../../qt-shared/About.ui \
    ../../qt-shared/InputSettings.ui \
    ../../qt-shared/MainWindow.ui \
    ../../qt-shared/SoundSettings.ui \
    ../../qt-shared/VideoSettings.ui

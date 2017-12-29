#-------------------------------------------------
#
# Project created by QtCreator 2017-12-29T11:36:59
#
#-------------------------------------------------

QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Gearboy
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += "$$_PRO_FILE_PWD_/glew/include"
INCLUDEPATH += "$$_PRO_FILE_PWD_/sdl/include"

LIBS += -L$$_PRO_FILE_PWD_/sdl/lib -L$$_PRO_FILE_PWD_/glew/lib -lSDL2 -lopengl32 -lglew32

SOURCES += \
    ../../qt-shared/About.cpp \
    ../../qt-shared/Emulator.cpp \
    ../../qt-shared/GLFrame.cpp \
    ../../qt-shared/InputSettings.cpp \
    ../../qt-shared/main.cpp \
    ../../qt-shared/MainWindow.cpp \
    ../../qt-shared/RenderThread.cpp \
    ../../qt-shared/SoundSettings.cpp \
    ../../qt-shared/VideoSettings.cpp \
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
    ../../../src/opcodes.cpp \
    ../../../src/opcodes_cb.cpp \
    ../../../src/Processor.cpp \
    ../../../src/RomOnlyMemoryRule.cpp \
    ../../../src/Video.cpp \
    ../../../src/audio/Blip_Buffer.cpp \
    ../../../src/audio/Effects_Buffer.cpp \
    ../../../src/audio/Gb_Apu.cpp \
    ../../../src/audio/Gb_Apu_State.cpp \
    ../../../src/audio/Gb_Oscs.cpp \
    ../../../src/audio/Multi_Buffer.cpp

HEADERS += \
    ../../qt-shared/About.h \
    ../../qt-shared/Emulator.h \
    ../../qt-shared/GLFrame.h \
    ../../qt-shared/InputSettings.h \
    ../../qt-shared/MainWindow.h \
    ../../qt-shared/RenderThread.h \
    ../../qt-shared/SoundSettings.h \
    ../../qt-shared/VideoSettings.h \
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
    ../../../src/Memory.h \
    ../../../src/Memory_inline.h \
    ../../../src/MemoryRule.h \
    ../../../src/MultiMBC1MemoryRule.h \
    ../../../src/opcode_names.h \
    ../../../src/opcode_timing.h \
    ../../../src/Processor.h \
    ../../../src/Processor_inline.h \
    ../../../src/RomOnlyMemoryRule.h \
    ../../../src/SixteenBitRegister.h \
    ../../../src/Video.h \
    ../../../src/audio/blargg_common.h \
    ../../../src/audio/blargg_config.h \
    ../../../src/audio/blargg_source.h \
    ../../../src/audio/Blip_Buffer.h \
    ../../../src/audio/Blip_Synth.h \
    ../../../src/audio/Effects_Buffer.h \
    ../../../src/audio/Gb_Apu.h \
    ../../../src/audio/Gb_Oscs.h \
    ../../../src/audio/Multi_Buffer.h

FORMS += \
    ../../qt-shared/About.ui \
    ../../qt-shared/InputSettings.ui \
    ../../qt-shared/MainWindow.ui \
    ../../qt-shared/SoundSettings.ui \
    ../../qt-shared/VideoSettings.ui

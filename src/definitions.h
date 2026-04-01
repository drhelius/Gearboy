/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef DEBUG
#define DEBUG_GEARBOY 1
#endif

#if defined(PS2) || defined(PSP)
#define PERFORMANCE
#endif

#if !defined(EMULATOR_BUILD)
    #define EMULATOR_BUILD "undefined"
#endif

#define GEARBOY_TITLE "Gearboy"
#define GEARBOY_VERSION EMULATOR_BUILD
#define GEARBOY_TITLE_ASCII "" \
"   ____                 _                  \n" \
"  / ___| ___  __ _ _ __| |__   ___  _   _  \n" \
" | |  _ / _ \\/ _` | '__| '_ \\ / _ \\| | | | \n" \
" | |_| |  __/ (_| | |  | |_) | (_) | |_| | \n" \
"  \\____|\\___|\\__,_|_|  |_.__/ \\___/ \\__, | \n" \
"                                    |___/  \n"

#ifndef NULL
#define NULL 0
#endif

#ifdef _WIN32
#define BLARGG_USE_NAMESPACE 1
#endif

//#define GEARBOY_DISABLE_DISASSEMBLER

#define MAX_ROM_SIZE 0x800000

#define SafeDelete(pointer) if(pointer != NULL) {delete pointer; pointer = NULL;}
#define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}

#define InitPointer(pointer) ((pointer) = NULL)
#define IsValidPointer(pointer) ((pointer) != NULL)

#define UNUSED(expr) (void)(expr)

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(value, min, max) MIN(MAX(value, min), max)

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

typedef void (*RamChangedCallback) (void);

#define FLAG_ZERO 0x80
#define FLAG_SUB 0x40
#define FLAG_HALF 0x20
#define FLAG_CARRY 0x10
#define FLAG_NONE 0

#define GEARBOY_MASTER_CLOCK_RATE 4194304

#define GEARBOY_MAX_GAMEPADS 1

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

#define SGB_SCREEN_WIDTH 256
#define SGB_SCREEN_HEIGHT 224

#define GAMEBOY_CLOCKS_PER_FRAME 70224
#define GAMEBOY_CLOCKS_SAFE_LIMIT ((GAMEBOY_CLOCKS_PER_FRAME * 5) / 4)

#define AUDIO_BUFFER_SIZE 4096
#define GB_AUDIO_SAMPLE_RATE 44100
#define GB_AUDIO_QUEUE_SIZE 1850

#define SAVESTATE_MAGIC 0x28011983

#define GB_SAVESTATE_MAGIC 0x28011983
#define GB_SAVESTATE_VERSION 102
#define GB_SAVESTATE_MIN_VERSION 100
#define GB_SAVESTATE_LEGACY_VERSION 0

static const u16 kTACTriggerBits[] = {512, 8, 32, 128};

struct GB_SaveState_Header
{
    u32 magic;
    u32 version;
    u32 size;
    s64 timestamp;
    char rom_name[128];
    u32 rom_crc;
    u32 screenshot_size;
    u16 screenshot_width;
    u16 screenshot_height;
    char emu_build[32];
};

struct GB_SaveState_Header_Libretro
{
    u32 magic;
    u32 version;
};

struct GB_SaveState_Screenshot
{
    u32 width;
    u32 height;
    u32 size;
    u8* data;
};

struct GB_Color
{
    u8 red;
    u8 green;
    u8 blue;
};

enum GB_Color_Format
{
    GB_PIXEL_RGB565,
    GB_PIXEL_RGB555,
    GB_PIXEL_BGR565,
    GB_PIXEL_BGR555
};

enum Gameboy_Keys
{
    A_Key = 0x10,
    B_Key = 0x20,
    Start_Key = 0x80,
    Select_Key = 0x40,
    Right_Key = 0x01,
    Left_Key = 0x02,
    Up_Key = 0x04,
    Down_Key = 0x08
};

struct GB_RuntimeInfo
{
    int screen_width;
    int screen_height;
};

struct GB_Disassembler_Record
{
    u32 address;
    u8 bank;
    char name[64];
    char bytes[25];
    char segment[8];
    u8 opcodes[4];
    int size;
    bool jump;
    u16 jump_address;
    u8 jump_bank;
    bool subroutine;
    int irq;
    bool has_operand_address;
    u16 operand_address;
    char auto_symbol[64];
};

typedef GB_Disassembler_Record GS_Disassembler_Record;

inline u8 SetBit(const u8 value, const u8 bit)
{
    return value | static_cast<u8>(0x01 << bit);
}

inline u8 UnsetBit(const u8 value, const u8 bit)
{
    return value & (~(0x01 << bit));
}

inline bool IsSetBit(const u8 value, const u8 bit)
{
    return (value & (0x01 << bit)) != 0;
}

inline int AsHex(const char c)
{
  return c >= 'A' ? c - 'A' + 0xA : c - '0';
}

#if !defined(DEBUG_GEARBOY)
    #if defined(__GNUC__) || defined(__clang__)
        #if !defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
            #warning "Compiling without optimizations."
            #define GEARBOY_NO_OPTIMIZATIONS
        #endif
    #elif defined(_MSC_VER)
        #if !defined(NDEBUG)
            #pragma message("Compiling without optimizations.")
            #define GEARBOY_NO_OPTIMIZATIONS
        #endif
    #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define INLINE inline __attribute__((always_inline))
    #define NO_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define INLINE __forceinline
    #define NO_INLINE __declspec(noinline)
#else
    #define INLINE inline
    #define NO_INLINE
#endif

#if !defined(DEBUG_GEARBOY)
    #if defined(__GNUC__) || defined(__clang__)
        #if !defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
            #warning "Compiling without optimizations."
            #define GEARBOY_NO_OPTIMIZATIONS
        #endif
    #elif defined(_MSC_VER)
        #if !defined(NDEBUG)
            #pragma message("Compiling without optimizations.")
            #define GEARBOY_NO_OPTIMIZATIONS
        #endif
    #endif
#endif

#endif	/* DEFINITIONS_H */

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

#define DEBUG_GEARBOY 1
#define GEARBOY_VERSION "3.0.0"

#ifndef NULL
#define NULL 0
#endif

#ifdef _WIN32
#define BLARGG_USE_NAMESPACE 1
#endif

#define SafeDelete(pointer) if(pointer != NULL) {delete pointer; pointer = NULL;}
#define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}

#define InitPointer(pointer) ((pointer) = NULL)
#define IsValidPointer(pointer) ((pointer) != NULL)

#if defined(MSB_FIRST) || defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define IS_BIG_ENDIAN
#else
#define IS_LITTLE_ENDIAN
#endif

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

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

#define AUDIO_BUFFER_SIZE 4096

#define SAVESTATE_MAGIC 0x28011983

struct GB_Color
{
#if defined(__LIBRETRO__)
    #if defined(IS_LITTLE_ENDIAN)
    u8 blue;
    u8 green;
    u8 red;
    u8 alpha;
    #elif defined(IS_BIG_ENDIAN)
    u8 alpha;
    u8 red;
    u8 green;
    u8 blue;
    #endif
#else
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
#endif
};

enum Gameboy_Keys
{
    A_Key = 4,
    B_Key = 5,
    Start_Key = 7,
    Select_Key = 6,
    Right_Key = 0,
    Left_Key = 1,
    Up_Key = 2,
    Down_Key = 3
};

#ifdef DEBUG_GEARBOY
    #ifdef __ANDROID__
        #include <android/log.h>
        #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "GEARBOY", __VA_ARGS__);
    #endif
#define Log(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#else
#define Log(msg, ...)
#endif

inline void Log_func(const char* const msg, ...)
{
    static int count = 1;
    char szBuf[512];

    va_list args;
    va_start(args, msg);
    vsprintf(szBuf, msg, args);
    va_end(args);

    printf("%d: %s\n", count, szBuf);

    count++;
}

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

#endif	/* DEFINITIONS_H */

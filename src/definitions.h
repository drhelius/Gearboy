/* 
 * Gearboy Gameboy Emulator
 * Copyright (C) 2012 Ignacio Sanchez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * The full license is available at http://www.gnu.org/licenses/gpl.html
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <fstream>

//#define DEBUG_GEARBOY 1

#ifndef NULL
#define NULL 0
#endif

#define SafeDelete(pointer) if(pointer != NULL) {delete pointer; pointer = NULL;}
#define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}

#define InitPointer(pointer) ((pointer) = NULL)
#define IsValidPointer(pointer) ((pointer) != NULL)

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;

#define BIT_MASK_4 0x0F
#define BIT_MASK_8 0xFF
#define BIT_MASK_16 0xFFFF

#define FLAG_ZERO 0x80
#define FLAG_SUB 0x40
#define FLAG_HALF 0x20
#define FLAG_CARRY 0x10
#define FLAG_NONE 0

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

#define MAX_STRING_SIZE 256

struct GB_Color
{
    u8 red;
    u8 green;
    u8 blue;
	u8 alpha;
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

enum Gameboy_MemoryBankControllers
{
    MBC_NONE,
    MBC1,
    MBC2,
    MBC3,
    MBC5
};

inline void Log(const char* const msg, ...)
{
#ifdef DEBUG_GEARBOY
    static int count = 1;
    char szBuf[MAX_STRING_SIZE];

    va_list args;
    va_start(args, msg);
    vsprintf(szBuf, msg, args);
    va_end(args);

    printf("%d: %s\n", count, szBuf);

    count++;
#endif
}

inline u8 SetBit(const u8 value, const u8 bit)
{
    return value | (0x01 << bit);
}

inline u8 UnsetBit(const u8 value, const u8 bit)
{
    return value & (~(0x01 << bit));
}

inline bool IsSetBit(const u8 value, const u8 bit)
{
    return (value & (0x01 << bit)) != 0;
}

#endif	/* DEFINITIONS_H */


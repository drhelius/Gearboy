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

#ifndef GUI_DEBUG_CONSTANTS_H
#define GUI_DEBUG_CONSTANTS_H

#include "imgui.h"
#include "gearboy.h"
#include "gui_colors.h"

struct stDebugLabel
{
    u16 address;
    const char* label;
};

enum eDebugIODirection
{
    IO_IN   = 1,
    IO_OUT  = 2,
    IO_BOTH = 3,
};

struct stDebugIOLabel
{
    u16 address;
    const char* label;
    int direction;
};

static const int k_debug_io_label_count = 65;
static const stDebugIOLabel k_debug_io_labels[k_debug_io_label_count] = 
{
    // Joypad
    { 0x00, "P1_JOYPAD", IO_BOTH },
    // Serial
    { 0x01, "SB_SERIAL", IO_BOTH },
    { 0x02, "SC_SERIAL", IO_BOTH },
    // Timer
    { 0x04, "DIV", IO_BOTH },
    { 0x05, "TIMA", IO_BOTH },
    { 0x06, "TMA", IO_BOTH },
    { 0x07, "TAC", IO_BOTH },
    // Interrupt Flag
    { 0x0F, "IF", IO_BOTH },
    // Sound Channel 1 - Pulse with sweep
    { 0x10, "NR10", IO_BOTH },
    { 0x11, "NR11", IO_BOTH },
    { 0x12, "NR12", IO_BOTH },
    { 0x13, "NR13", IO_OUT },
    { 0x14, "NR14", IO_BOTH },
    // Sound Channel 2 - Pulse
    { 0x16, "NR21", IO_BOTH },
    { 0x17, "NR22", IO_BOTH },
    { 0x18, "NR23", IO_OUT },
    { 0x19, "NR24", IO_BOTH },
    // Sound Channel 3 - Wave
    { 0x1A, "NR30", IO_BOTH },
    { 0x1B, "NR31", IO_OUT },
    { 0x1C, "NR32", IO_BOTH },
    { 0x1D, "NR33", IO_OUT },
    { 0x1E, "NR34", IO_BOTH },
    // Sound Channel 4 - Noise
    { 0x20, "NR41", IO_OUT },
    { 0x21, "NR42", IO_BOTH },
    { 0x22, "NR43", IO_BOTH },
    { 0x23, "NR44", IO_BOTH },
    // Sound Control
    { 0x24, "NR50", IO_BOTH },
    { 0x25, "NR51", IO_BOTH },
    { 0x26, "NR52", IO_BOTH },
    // Wave RAM
    { 0x30, "WAVE_0", IO_BOTH },
    { 0x31, "WAVE_1", IO_BOTH },
    { 0x32, "WAVE_2", IO_BOTH },
    { 0x33, "WAVE_3", IO_BOTH },
    { 0x34, "WAVE_4", IO_BOTH },
    { 0x35, "WAVE_5", IO_BOTH },
    { 0x36, "WAVE_6", IO_BOTH },
    { 0x37, "WAVE_7", IO_BOTH },
    { 0x38, "WAVE_8", IO_BOTH },
    { 0x39, "WAVE_9", IO_BOTH },
    { 0x3A, "WAVE_A", IO_BOTH },
    { 0x3B, "WAVE_B", IO_BOTH },
    { 0x3C, "WAVE_C", IO_BOTH },
    { 0x3D, "WAVE_D", IO_BOTH },
    { 0x3E, "WAVE_E", IO_BOTH },
    { 0x3F, "WAVE_F", IO_BOTH },
    // LCD
    { 0x40, "LCDC", IO_BOTH },
    { 0x41, "STAT", IO_BOTH },
    { 0x42, "SCY", IO_BOTH },
    { 0x43, "SCX", IO_BOTH },
    { 0x44, "LY", IO_IN },
    { 0x45, "LYC", IO_BOTH },
    { 0x46, "DMA", IO_OUT },
    { 0x47, "BGP", IO_BOTH },
    { 0x48, "OBP0", IO_BOTH },
    { 0x49, "OBP1", IO_BOTH },
    { 0x4A, "WY", IO_BOTH },
    { 0x4B, "WX", IO_BOTH },
    // CGB registers
    { 0x4D, "KEY1", IO_BOTH },
    { 0x4F, "VBK", IO_BOTH },
    { 0x55, "HDMA5", IO_BOTH },
    { 0x68, "BCPS", IO_BOTH },
    { 0x69, "BCPD", IO_BOTH },
    { 0x6A, "OCPS", IO_BOTH },
    { 0x6B, "OCPD", IO_BOTH },
    { 0x70, "SVBK", IO_BOTH },
};

static const int k_debug_symbol_count = 14;

static const stDebugLabel k_debug_symbols[k_debug_symbol_count] = 
{
    { 0x0000, "RST_00" },
    { 0x0008, "RST_08" },
    { 0x0010, "RST_10" },
    { 0x0018, "RST_18" },
    { 0x0020, "RST_20" },
    { 0x0028, "RST_28" },
    { 0x0030, "RST_30" },
    { 0x0038, "RST_38" },
    { 0x0040, "VBLANK_HANDLER" },
    { 0x0048, "STAT_HANDLER" },
    { 0x0050, "TIMER_HANDLER" },
    { 0x0058, "SERIAL_HANDLER" },
    { 0x0060, "JOYPAD_HANDLER" },
    { 0x0100, "ENTRY_POINT" },
};

#endif /* GUI_DEBUG_CONSTANTS_H */

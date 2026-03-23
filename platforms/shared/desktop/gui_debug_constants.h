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

static const ImVec4 cyan =          ImVec4(0.10f, 0.90f, 0.90f, 1.0f);
static const ImVec4 dark_cyan =     ImVec4(0.00f, 0.30f, 0.30f, 1.0f);
static const ImVec4 magenta =       ImVec4(1.00f, 0.50f, 0.96f, 1.0f);
static const ImVec4 dark_magenta =  ImVec4(0.30f, 0.18f, 0.27f, 1.0f);
static const ImVec4 yellow =        ImVec4(1.00f, 0.90f, 0.05f, 1.0f);
static const ImVec4 dark_yellow =   ImVec4(0.30f, 0.25f, 0.00f, 1.0f);
static const ImVec4 orange =        ImVec4(1.00f, 0.50f, 0.00f, 1.0f);
static const ImVec4 dark_orange =   ImVec4(0.60f, 0.20f, 0.00f, 1.0f);
static const ImVec4 red =           ImVec4(0.98f, 0.15f, 0.45f, 1.0f);
static const ImVec4 dark_red =      ImVec4(0.30f, 0.04f, 0.16f, 1.0f);
static const ImVec4 green =         ImVec4(0.10f, 0.90f, 0.10f, 1.0f);
static const ImVec4 dim_green =     ImVec4(0.05f, 0.40f, 0.05f, 1.0f);
static const ImVec4 dark_green =    ImVec4(0.03f, 0.20f, 0.02f, 1.0f);
static const ImVec4 violet =        ImVec4(0.68f, 0.51f, 1.00f, 1.0f);
static const ImVec4 dark_violet =   ImVec4(0.24f, 0.15f, 0.30f, 1.0f);
static const ImVec4 blue =          ImVec4(0.20f, 0.40f, 1.00f, 1.0f);
static const ImVec4 dark_blue =     ImVec4(0.07f, 0.10f, 0.30f, 1.0f);
static const ImVec4 white =         ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
static const ImVec4 gray =          ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
static const ImVec4 mid_gray =      ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
static const ImVec4 dark_gray =     ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
static const ImVec4 black =         ImVec4(0.00f, 0.00f, 0.00f, 1.0f);
static const ImVec4 brown =         ImVec4(0.68f, 0.50f, 0.36f, 1.0f);
static const ImVec4 dark_brown =    ImVec4(0.38f, 0.20f, 0.06f, 1.0f);

static const char* const c_cyan = "{19E6E6}";
static const char* const c_dark_cyan = "{004C4C}";
static const char* const c_magenta = "{FF80F5}";
static const char* const c_dark_magenta = "{4C2E45}";
static const char* const c_yellow = "{FFE60D}";
static const char* const c_dark_yellow = "{4C4000}";
static const char* const c_orange = "{FF8000}";
static const char* const c_dark_orange = "{993300}";
static const char* const c_red = "{FA2673}";
static const char* const c_dark_red = "{4C0A29}";
static const char* const c_green = "{19E619}";
static const char* const c_dim_green = "{0D660D}";
static const char* const c_dark_green = "{083305}";
static const char* const c_violet = "{AD82FF}";
static const char* const c_dark_violet = "{3D274D}";
static const char* const c_blue = "{3366FF}";
static const char* const c_dark_blue = "{12194D}";
static const char* const c_white = "{FFFFFF}";
static const char* const c_gray = "{808080}";
static const char* const c_mid_gray = "{666666}";
static const char* const c_dark_gray = "{1A1A1A}";
static const char* const c_black = "{000000}";
static const char* const c_brown = "{AD805C}";
static const char* const c_dark_brown = "{61330F}";

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
    { 0x0040, "VBlank_Handler" },
    { 0x0048, "LCD_STAT_Handler" },
    { 0x0050, "Timer_Handler" },
    { 0x0058, "Serial_Handler" },
    { 0x0060, "Joypad_Handler" },
    { 0x0100, "Entry_Point" },
};

#endif /* GUI_DEBUG_CONSTANTS_H */
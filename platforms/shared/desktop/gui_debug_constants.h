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
#include "config.h"

struct GuiDebugColor
{
    ImVec4 dark;
    ImVec4 light;

    operator ImVec4() const
    {
        return (config_emulator.theme == config_Theme_Light) ? light : dark;
    }
};

struct GuiDebugTextColor
{
    const char* dark;
    const char* light;

    const char* c_str() const
    {
        return (config_emulator.theme == config_Theme_Light) ? light : dark;
    }

    operator const char*() const
    {
        return c_str();
    }
};

static const GuiDebugColor cyan =          { ImVec4(0.10f, 0.90f, 0.90f, 1.0f), ImVec4(0.00f, 0.49f, 0.60f, 1.0f) };
static const GuiDebugColor dark_cyan =     { ImVec4(0.00f, 0.30f, 0.30f, 1.0f), ImVec4(0.64f, 0.91f, 0.95f, 1.0f) };
static const GuiDebugColor magenta =       { ImVec4(1.00f, 0.50f, 0.96f, 1.0f), ImVec4(0.82f, 0.08f, 0.76f, 1.0f) };
static const GuiDebugColor dark_magenta =  { ImVec4(0.30f, 0.18f, 0.27f, 1.0f), ImVec4(0.93f, 0.74f, 0.92f, 1.0f) };
static const GuiDebugColor yellow =        { ImVec4(1.00f, 0.90f, 0.05f, 1.0f), ImVec4(0.64f, 0.48f, 0.00f, 1.0f) };
static const GuiDebugColor dark_yellow =   { ImVec4(0.30f, 0.25f, 0.00f, 1.0f), ImVec4(0.96f, 0.88f, 0.50f, 1.0f) };
static const GuiDebugColor orange =        { ImVec4(1.00f, 0.50f, 0.00f, 1.0f), ImVec4(0.84f, 0.28f, 0.00f, 1.0f) };
static const GuiDebugColor dark_orange =   { ImVec4(0.60f, 0.20f, 0.00f, 1.0f), ImVec4(0.98f, 0.76f, 0.58f, 1.0f) };
static const GuiDebugColor red =           { ImVec4(0.98f, 0.15f, 0.45f, 1.0f), ImVec4(0.86f, 0.00f, 0.26f, 1.0f) };
static const GuiDebugColor dark_red =      { ImVec4(0.30f, 0.04f, 0.16f, 1.0f), ImVec4(0.97f, 0.68f, 0.78f, 1.0f) };
static const GuiDebugColor green =         { ImVec4(0.10f, 0.90f, 0.10f, 1.0f), ImVec4(0.00f, 0.55f, 0.10f, 1.0f) };
static const GuiDebugColor dim_green =     { ImVec4(0.05f, 0.40f, 0.05f, 1.0f), ImVec4(0.32f, 0.60f, 0.28f, 1.0f) };
static const GuiDebugColor dark_green =    { ImVec4(0.03f, 0.20f, 0.02f, 1.0f), ImVec4(0.68f, 0.91f, 0.64f, 1.0f) };
static const GuiDebugColor violet =        { ImVec4(0.68f, 0.51f, 1.00f, 1.0f), ImVec4(0.46f, 0.24f, 0.82f, 1.0f) };
static const GuiDebugColor dark_violet =   { ImVec4(0.24f, 0.15f, 0.30f, 1.0f), ImVec4(0.80f, 0.70f, 0.94f, 1.0f) };
static const GuiDebugColor blue =          { ImVec4(0.20f, 0.40f, 1.00f, 1.0f), ImVec4(0.05f, 0.24f, 0.88f, 1.0f) };
static const GuiDebugColor dark_blue =     { ImVec4(0.07f, 0.10f, 0.30f, 1.0f), ImVec4(0.68f, 0.76f, 0.96f, 1.0f) };
static const GuiDebugColor white =         { ImVec4(1.00f, 1.00f, 1.00f, 1.0f), ImVec4(0.12f, 0.11f, 0.15f, 1.0f) };
static const GuiDebugColor gray =          { ImVec4(0.50f, 0.50f, 0.50f, 1.0f), ImVec4(0.45f, 0.43f, 0.50f, 1.0f) };
static const GuiDebugColor mid_gray =      { ImVec4(0.40f, 0.40f, 0.40f, 1.0f), ImVec4(0.62f, 0.59f, 0.67f, 1.0f) };
static const GuiDebugColor dark_gray =     { ImVec4(0.10f, 0.10f, 0.10f, 1.0f), ImVec4(0.64f, 0.61f, 0.69f, 1.0f) };
static const GuiDebugColor black =         { ImVec4(0.00f, 0.00f, 0.00f, 1.0f), ImVec4(1.00f, 1.00f, 1.00f, 1.0f) };
static const GuiDebugColor brown =         { ImVec4(0.68f, 0.50f, 0.36f, 1.0f), ImVec4(0.56f, 0.30f, 0.10f, 1.0f) };
static const GuiDebugColor dark_brown =    { ImVec4(0.38f, 0.20f, 0.06f, 1.0f), ImVec4(0.90f, 0.72f, 0.55f, 1.0f) };

static const GuiDebugTextColor c_cyan = { "{19E6E6}", "{007D99}" };
static const GuiDebugTextColor c_dark_cyan = { "{004C4C}", "{A3E8F2}" };
static const GuiDebugTextColor c_magenta = { "{FF80F5}", "{D114C2}" };
static const GuiDebugTextColor c_dark_magenta = { "{4C2E45}", "{EDBDEB}" };
static const GuiDebugTextColor c_yellow = { "{FFE60D}", "{A37A00}" };
static const GuiDebugTextColor c_dark_yellow = { "{4C4000}", "{F5E080}" };
static const GuiDebugTextColor c_orange = { "{FF8000}", "{D64700}" };
static const GuiDebugTextColor c_dark_orange = { "{993300}", "{FAC294}" };
static const GuiDebugTextColor c_red = { "{FA2673}", "{DB0042}" };
static const GuiDebugTextColor c_dark_red = { "{4C0A29}", "{F7ADC7}" };
static const GuiDebugTextColor c_green = { "{19E619}", "{008C1A}" };
static const GuiDebugTextColor c_dim_green = { "{0D660D}", "{529947}" };
static const GuiDebugTextColor c_dark_green = { "{083305}", "{ADE8A3}" };
static const GuiDebugTextColor c_violet = { "{AD82FF}", "{753DD1}" };
static const GuiDebugTextColor c_dark_violet = { "{3D274D}", "{CCB3F0}" };
static const GuiDebugTextColor c_blue = { "{3366FF}", "{0D3DE0}" };
static const GuiDebugTextColor c_dark_blue = { "{12194D}", "{ADC2F5}" };
static const GuiDebugTextColor c_white = { "{FFFFFF}", "{1F1D26}" };
static const GuiDebugTextColor c_gray = { "{808080}", "{736E80}" };
static const GuiDebugTextColor c_mid_gray = { "{666666}", "{9E96AB}" };
static const GuiDebugTextColor c_dark_gray = { "{1A1A1A}", "{A39CB0}" };
static const GuiDebugTextColor c_black = { "{000000}", "{000000}" };
static const GuiDebugTextColor c_brown = { "{AD805C}", "{8F4D1A}" };
static const GuiDebugTextColor c_dark_brown = { "{61330F}", "{E6B88C}" };

static inline ImVec4 gui_debug_lerp_color(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

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
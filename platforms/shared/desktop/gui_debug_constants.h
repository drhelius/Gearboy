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

static inline ImVec4 gui_debug_color(unsigned int rgb)
{
    return ImVec4(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, 1.0f);
}

static const GuiDebugColor cyan = { gui_debug_color(0x1AE6E6), gui_debug_color(0x007C91) };
static const GuiDebugColor dark_cyan = { gui_debug_color(0x004D4D), gui_debug_color(0xCBEFF3) };
static const GuiDebugColor magenta = { gui_debug_color(0xFF80F5), gui_debug_color(0xB42375) };
static const GuiDebugColor dark_magenta = { gui_debug_color(0x4D2E45), gui_debug_color(0xF3D6E8) };
static const GuiDebugColor yellow = { gui_debug_color(0xFFE60D), gui_debug_color(0x8A6000) };
static const GuiDebugColor dark_yellow = { gui_debug_color(0x4D4000), gui_debug_color(0xF7E7B2) };
static const GuiDebugColor orange = { gui_debug_color(0xFF8000), gui_debug_color(0xC44D00) };
static const GuiDebugColor dark_orange = { gui_debug_color(0x993300), gui_debug_color(0xF8D4B6) };
static const GuiDebugColor red = { gui_debug_color(0xFA2673), gui_debug_color(0xC7254E) };
static const GuiDebugColor dark_red = { gui_debug_color(0x4D0A29), gui_debug_color(0xF6CDD8) };
static const GuiDebugColor green = { gui_debug_color(0x1AE61A), gui_debug_color(0x17823B) };
static const GuiDebugColor dim_green = { gui_debug_color(0x0D660D), gui_debug_color(0x4D7438) };
static const GuiDebugColor dark_green = { gui_debug_color(0x083305), gui_debug_color(0xD5E8D6) };
static const GuiDebugColor violet = { gui_debug_color(0xAD82FF), gui_debug_color(0x7047C2) };
static const GuiDebugColor dark_violet = { gui_debug_color(0x3D264D), gui_debug_color(0xE4D9F7) };
static const GuiDebugColor blue = { gui_debug_color(0x3366FF), gui_debug_color(0x0969DA) };
static const GuiDebugColor dark_blue = { gui_debug_color(0x121A4D), gui_debug_color(0xD7E5FA) };
static const GuiDebugColor white = { gui_debug_color(0xFFFFFF), gui_debug_color(0x21201C) };
static const GuiDebugColor gray = { gui_debug_color(0x808080), gui_debug_color(0x69645D) };
static const GuiDebugColor mid_gray = { gui_debug_color(0x666666), gui_debug_color(0x756F67) };
static const GuiDebugColor dark_gray = { gui_debug_color(0x1A1A1A), gui_debug_color(0x4B4842) };
static const GuiDebugColor black = { gui_debug_color(0x000000), gui_debug_color(0x21201C) };
static const GuiDebugColor brown = { gui_debug_color(0xAD805C), gui_debug_color(0x87502C) };
static const GuiDebugColor dark_brown = { gui_debug_color(0x61330F), gui_debug_color(0xE8D6C8) };

static const GuiDebugTextColor c_cyan = { "{1AE6E6}", "{007C91}" };
static const GuiDebugTextColor c_dark_cyan = { "{004D4D}", "{CBEFF3}" };
static const GuiDebugTextColor c_magenta = { "{FF80F5}", "{B42375}" };
static const GuiDebugTextColor c_dark_magenta = { "{4D2E45}", "{F3D6E8}" };
static const GuiDebugTextColor c_yellow = { "{FFE60D}", "{8A6000}" };
static const GuiDebugTextColor c_dark_yellow = { "{4D4000}", "{F7E7B2}" };
static const GuiDebugTextColor c_orange = { "{FF8000}", "{C44D00}" };
static const GuiDebugTextColor c_dark_orange = { "{993300}", "{F8D4B6}" };
static const GuiDebugTextColor c_red = { "{FA2673}", "{C7254E}" };
static const GuiDebugTextColor c_dark_red = { "{4D0A29}", "{F6CDD8}" };
static const GuiDebugTextColor c_green = { "{1AE61A}", "{17823B}" };
static const GuiDebugTextColor c_dim_green = { "{0D660D}", "{4D7438}" };
static const GuiDebugTextColor c_dark_green = { "{083305}", "{D5E8D6}" };
static const GuiDebugTextColor c_violet = { "{AD82FF}", "{7047C2}" };
static const GuiDebugTextColor c_dark_violet = { "{3D264D}", "{E4D9F7}" };
static const GuiDebugTextColor c_blue = { "{3366FF}", "{0969DA}" };
static const GuiDebugTextColor c_dark_blue = { "{121A4D}", "{D7E5FA}" };
static const GuiDebugTextColor c_white = { "{FFFFFF}", "{21201C}" };
static const GuiDebugTextColor c_gray = { "{808080}", "{69645D}" };
static const GuiDebugTextColor c_mid_gray = { "{666666}", "{756F67}" };
static const GuiDebugTextColor c_dark_gray = { "{1A1A1A}", "{4B4842}" };
static const GuiDebugTextColor c_black = { "{000000}", "{21201C}" };
static const GuiDebugTextColor c_brown = { "{AD805C}", "{87502C}" };
static const GuiDebugTextColor c_dark_brown = { "{61330F}", "{E8D6C8}" };

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

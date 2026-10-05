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

#ifndef GUI_COLORS_H
#define GUI_COLORS_H

#include "imgui.h"
#include "config.h"

struct GuiColor
{
    ImVec4 dark;
    ImVec4 light;

    operator ImVec4() const
    {
        return (config_emulator.theme == config_Theme_Light) ? light : dark;
    }
};

struct GuiTextColor
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

static inline ImVec4 gui_color(unsigned int rgb)
{
    return ImVec4(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, 1.0f);
}

static const GuiColor cyan = { gui_color(0x1AE6E6), gui_color(0x007C91) };
static const GuiColor dark_cyan = { gui_color(0x004D4D), gui_color(0xCBEFF3) };
static const GuiColor magenta = { gui_color(0xFF80F5), gui_color(0xB42375) };
static const GuiColor dark_magenta = { gui_color(0x4D2E45), gui_color(0xF3D6E8) };
static const GuiColor yellow = { gui_color(0xFFE60D), gui_color(0x8A6000) };
static const GuiColor dark_yellow = { gui_color(0x4D4000), gui_color(0xF7E7B2) };
static const GuiColor amber = { gui_color(0xE6B31A), gui_color(0x9C5D00) };
static const GuiColor orange = { gui_color(0xFF8000), gui_color(0xC44D00) };
static const GuiColor dark_orange = { gui_color(0x993300), gui_color(0xF8D4B6) };
static const GuiColor red = { gui_color(0xFA2673), gui_color(0xC7254E) };
static const GuiColor dark_red = { gui_color(0x4D0A29), gui_color(0xF6CDD8) };
static const GuiColor green = { gui_color(0x1AE61A), gui_color(0x17823B) };
static const GuiColor dim_green = { gui_color(0x0D660D), gui_color(0x4D7438) };
static const GuiColor dark_green = { gui_color(0x083305), gui_color(0xD5E8D6) };
static const GuiColor violet = { gui_color(0xAD82FF), gui_color(0x7047C2) };
static const GuiColor dark_violet = { gui_color(0x3D264D), gui_color(0xE4D9F7) };
static const GuiColor blue = { gui_color(0x3366FF), gui_color(0x0969DA) };
static const GuiColor dark_blue = { gui_color(0x121A4D), gui_color(0xD7E5FA) };
static const GuiColor cornflower = { gui_color(0x6394ED), gui_color(0x3D67B6) };
static const GuiColor white = { gui_color(0xFFFFFF), gui_color(0x21201C) };
static const GuiColor gray = { gui_color(0x808080), gui_color(0x69645D) };
static const GuiColor mid_gray = { gui_color(0x666666), gui_color(0x756F67) };
static const GuiColor dark_gray = { gui_color(0x1A1A1A), gui_color(0x4B4842) };
static const GuiColor black = { gui_color(0x000000), gui_color(0x21201C) };
static const GuiColor brown = { gui_color(0xAD805C), gui_color(0x87502C) };
static const GuiColor dark_brown = { gui_color(0x61330F), gui_color(0xE8D6C8) };
static const GuiColor accent = { gui_color(0x856ED0), gui_color(0x6550B9) };

static const GuiTextColor c_cyan = { "{1AE6E6}", "{007C91}" };
static const GuiTextColor c_dark_cyan = { "{004D4D}", "{CBEFF3}" };
static const GuiTextColor c_magenta = { "{FF80F5}", "{B42375}" };
static const GuiTextColor c_dark_magenta = { "{4D2E45}", "{F3D6E8}" };
static const GuiTextColor c_yellow = { "{FFE60D}", "{8A6000}" };
static const GuiTextColor c_dark_yellow = { "{4D4000}", "{F7E7B2}" };
static const GuiTextColor c_amber = { "{E6B31A}", "{9C5D00}" };
static const GuiTextColor c_orange = { "{FF8000}", "{C44D00}" };
static const GuiTextColor c_dark_orange = { "{993300}", "{F8D4B6}" };
static const GuiTextColor c_red = { "{FA2673}", "{C7254E}" };
static const GuiTextColor c_dark_red = { "{4D0A29}", "{F6CDD8}" };
static const GuiTextColor c_green = { "{1AE61A}", "{17823B}" };
static const GuiTextColor c_dim_green = { "{0D660D}", "{4D7438}" };
static const GuiTextColor c_dark_green = { "{083305}", "{D5E8D6}" };
static const GuiTextColor c_violet = { "{AD82FF}", "{7047C2}" };
static const GuiTextColor c_dark_violet = { "{3D264D}", "{E4D9F7}" };
static const GuiTextColor c_blue = { "{3366FF}", "{0969DA}" };
static const GuiTextColor c_dark_blue = { "{121A4D}", "{D7E5FA}" };
static const GuiTextColor c_cornflower = { "{6394ED}", "{3D67B6}" };
static const GuiTextColor c_white = { "{FFFFFF}", "{21201C}" };
static const GuiTextColor c_gray = { "{808080}", "{69645D}" };
static const GuiTextColor c_mid_gray = { "{666666}", "{756F67}" };
static const GuiTextColor c_dark_gray = { "{1A1A1A}", "{4B4842}" };
static const GuiTextColor c_black = { "{000000}", "{21201C}" };
static const GuiTextColor c_brown = { "{AD805C}", "{87502C}" };
static const GuiTextColor c_dark_brown = { "{61330F}", "{E8D6C8}" };
static const GuiTextColor c_accent = { "{856ED0}", "{6550B9}" };

static inline ImVec4 gui_lerp_color(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

#endif /* GUI_COLORS_H */

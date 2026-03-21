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

#define GUI_DEBUG_TMS9918_IMPORT
#include "gui_debug_tms9918.h"

#include <math.h>
#include <cmath>
#include "imgui.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "ogl_renderer.h"
#include "utils.h"
#include "gui_filedialogs.h"

static ImVec4 color_444_to_float(u16 color);
static ImVec4 color_222_to_float(u8 color);
static void draw_context_menu_sprites(int index);
static void draw_context_menu_background(void);
static void draw_context_menu_tiles(void);

void gui_debug_window_vram_nametable(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(648, 254), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(546, 412), ImGuiCond_FirstUseEver);

    ImGui::Begin("Name Table", &config_debug.show_video_nametable);

    Video* video = emu_get_core()->GetVideo();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
    bool isMode224 = video->IsExtendedMode224();
    bool isGG = emu_get_core()->GetCartridge()->IsGameGear() && !emu_get_core()->GetCartridge()->IsGameGearInSMSMode();
    bool isSG1000 = video->IsSG1000Mode();
    int TMS9918mode = video->GetTMS9918Mode();

    bool window_hovered = ImGui::IsWindowHovered();
    static bool show_grid = true;
    static bool show_screen = true;
    int lines = 28;
    if (isMode224)
        lines = 32;
    else if (isSG1000)
        lines = 24;
    float scale = 1.5f;
    float size_h = 256.0f * scale;
    float size_v = 8.0f * lines * scale;
    float spacing = 8.0f * scale;

    ImGui::Checkbox("Show Grid##grid_bg", &show_grid); ImGui::SameLine();
    ImGui::Checkbox("Show Screen Rect", &show_screen);

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "bg", false);
    ImGui::SetColumnOffset(1, size_h + 10.0f);

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_background, ImVec2(size_h, size_v), ImVec2(0.0f, 0.0f), ImVec2(1.0f, (1.0f / 32.0f) * lines));

    draw_context_menu_background();

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + size_v), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;
        for (int n = 0; n <= lines; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + size_h, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    if (show_screen)
    {
        int scroll_x = 256 - regs[8];
        int scroll_y = regs[9];

        if (isGG)
        {
            scroll_x += GS_RESOLUTION_GG_X_OFFSET;
            scroll_y += GS_RESOLUTION_GG_Y_OFFSET;
        }

        scroll_x &= 0xFF;
        scroll_y &= 0xFF;

        float grid_x_max = p.x + size_h;
        float grid_y_max = p.y + size_v;

        float rect_x_min = p.x + (scroll_x * scale);
        float rect_y_min = p.y + (scroll_y * scale);
        float rect_x_max = p.x + ((scroll_x + runtime.screen_width) * scale);
        float rect_y_max = p.y + ((scroll_y + runtime.screen_height) * scale);

        float x_overflow = 0.0f;
        float y_overflow = 0.0f;

        if (rect_x_max > grid_x_max)
            x_overflow = rect_x_max - grid_x_max;
        if (rect_y_max > grid_y_max)
            y_overflow = rect_y_max - grid_y_max;

        draw_list->AddLine(ImVec2(rect_x_min, rect_y_min), ImVec2(fminf(rect_x_max, grid_x_max), rect_y_min), ImColor(green), 2.0f);
        if (x_overflow > 0.0f)
            draw_list->AddLine(ImVec2(p.x, rect_y_min), ImVec2(p.x + x_overflow, rect_y_min), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2(rect_x_min, rect_y_min), ImVec2(rect_x_min, fminf(rect_y_max, grid_y_max)), ImColor(green), 2.0f);
        if (y_overflow > 0.0f)
            draw_list->AddLine(ImVec2(rect_x_min, p.y), ImVec2(rect_x_min, p.y + y_overflow), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2(rect_x_min, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImVec2(fminf(rect_x_max, grid_x_max), (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImColor(green), 2.0f);
        if (x_overflow > 0.0f)
            draw_list->AddLine(ImVec2(p.x, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImVec2(p.x + x_overflow, (y_overflow > 0.0f) ? p.y + y_overflow : rect_y_max), ImColor(green), 2.0f);

        draw_list->AddLine(ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, rect_y_min), ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, fminf(rect_y_max, grid_y_max)), ImColor(green), 2.0f);
        if (y_overflow > 0.0f)
            draw_list->AddLine(ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, p.y), ImVec2((x_overflow > 0.0f) ? p.x + x_overflow : rect_x_max, p.y + y_overflow), ImColor(green), 2.0f);
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int tile_x = -1;
    int tile_y = -1;
    if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < size_h) && (mouse_y >= 0.0f) && (mouse_y < size_v))
    {
        tile_x = (int)(mouse_x / spacing);
        tile_y = (int)(mouse_y / spacing);

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_background, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::TextColored(green, "INFO:");

        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_x); ImGui::SameLine();
        ImGui::TextColored(cyan, "   Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_y);

        if (isSG1000)
        {
            int cols = (TMS9918mode == 1) ? 40 : 32;
            int name_table_addr = regs[2] << 10;
            int color_table_addr = regs[3] << 6;
            if (TMS9918mode == 2)
                color_table_addr &= 0x2000;
            int pattern_table_addr = regs[4] << 11;
            int region = (tile_y & 0x18) << 5;

            int tile_number = (tile_y * cols) + tile_x;
            int name_tile_addr = (name_table_addr + tile_number) & 0x3FFF;
            int name_tile = vram[name_tile_addr];

            if (TMS9918mode == 2)
            {
                pattern_table_addr &= 0x2000;
                name_tile += region;
            }
            else if (TMS9918mode == 4)
            {
                pattern_table_addr &= 0x2000;
            }

            int tile_addr = (pattern_table_addr + (name_tile << 3)) & 0x3FFF;

            int color_mask = ((regs[3] & 0x7F) << 3) | 0x07;

            int color_tile_addr = 0;

            if (TMS9918mode == 2)
                color_tile_addr = color_table_addr + ((name_tile & color_mask) << 3);
            else if (TMS9918mode == 0)
                color_tile_addr = color_table_addr + (name_tile >> 3);

            ImGui::TextColored(cyan, " Name Addr:"); ImGui::SameLine();
            ImGui::Text(" $%04X", name_tile_addr);
            ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
            ImGui::Text("$%03X", name_tile);
            ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
            ImGui::Text(" $%04X", tile_addr);
            ImGui::TextColored(cyan, " Color Addr:"); ImGui::SameLine();
            ImGui::Text("$%04X", color_tile_addr);
        }
        else
        {
            int name_table_addr = (regs[2] & (isMode224 ? 0x0C : 0x0E)) << 10;
            if (isMode224)
                name_table_addr |= 0x700;
            u16 map_addr = name_table_addr + (64 * tile_y) + (tile_x * 2);

            ImGui::TextColored(cyan, " Map Addr: "); ImGui::SameLine();
            ImGui::Text(" $%04X", map_addr);

            u16 tile_info_lo = vram[map_addr];
            u16 tile_info_hi = vram[map_addr + 1];

            int tile_number = ((tile_info_hi & 1) << 8) | tile_info_lo;
            bool tile_hflip = IsSetBit((u8)tile_info_hi, 1);
            bool tile_vflip = IsSetBit((u8)tile_info_hi, 2);
            int tile_palette = IsSetBit((u8)tile_info_hi, 3) ? 16 : 0;
            bool tile_priority = IsSetBit((u8)tile_info_hi, 4);

            ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
            ImGui::Text(" $%04X", tile_number << 5);
            ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
            ImGui::Text("$%03X", tile_number);
            ImGui::TextColored(cyan, " Value:"); ImGui::SameLine();
            ImGui::Text("$%04X", tile_info_hi << 8 | tile_info_lo);
            ImGui::TextColored(cyan, " Palette:"); ImGui::SameLine();
            ImGui::Text("%d", tile_palette);

            ImGui::TextColored(cyan, " H-Flip:"); ImGui::SameLine();
            tile_hflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

            ImGui::TextColored(cyan, " V-Flip:"); ImGui::SameLine();
            tile_vflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

            ImGui::TextColored(cyan, " Priority:"); ImGui::SameLine();
            tile_priority ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");
        }
    }

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_vram_tiles(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(648, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(540, 260), ImGuiCond_FirstUseEver);

    ImGui::Begin("Pattern Table", &config_debug.show_video_tiles);

    Video* video = emu_get_core()->GetVideo();
    u8* regs = video->GetRegisters();
    int TMS9918mode = video->GetTMS9918Mode();
    bool isSG1000 = video->IsSG1000Mode();

    bool window_hovered = ImGui::IsWindowHovered();
    static bool show_grid = true;
    int lines = isSG1000 ? 32 : 16;
    float scale = 1.5f;
    float width = 8.0f * 32.0f * scale;
    float height = 8.0f * lines * scale;
    float spacing = 8.0f * scale;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p;

    ImGui::Checkbox("Show Grid##grid_tiles", &show_grid);

    if (!isSG1000)
    {
        ImGui::SameLine(140.0f);
        ImGui::PushItemWidth(200.0f);
        ImGui::Combo("Palette##tile_palette", &emu_debug_tile_palette, "Palette 0 (BG)\0Palette 1 (BG & Sprites)\0\0");
        ImGui::PopItemWidth();
    }

    ImGui::Columns(2, "tiles", false);
    ImGui::SetColumnOffset(1, width + 10.0f);

    p = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_tiles, ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, (1.0f / 32.0f) * lines));

    draw_context_menu_tiles();

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + height), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;
        for (int n = 0; n <= lines; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + width, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int tile_x = -1;
    int tile_y = -1;

    if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
    {
        tile_x = (int)(mouse_x / spacing);
        tile_y = (int)(mouse_y / spacing);

        draw_list->AddRect(ImVec2(p.x + (tile_x * spacing), p.y + (tile_y * spacing)), ImVec2(p.x + ((tile_x + 1) * spacing), p.y + ((tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_tiles, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::PushFont(gui_default_font);

        ImGui::TextColored(green, "DETAILS:");

        int tile = (tile_y << 5) + tile_x;

        int tile_addr = 0;

        if (isSG1000)
        {
            int pattern_table_addr = (regs[4] & (TMS9918mode == 2 ? 0x04 : 0x07)) << 11;
            tile_addr = pattern_table_addr + (tile << 3);
        }
        else
        {
            tile_addr = tile << 5;
        }

        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%03X", tile);
        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", tile_addr);

        ImGui::PopFont();
    }

    ImGui::Columns(1);

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_vram_sprites(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(100, 254), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(480, 336), ImGuiCond_FirstUseEver);

    ImGui::Begin("Sprites", &config_debug.show_video_sprites);

    float scale = 4.0f;
    float size_8 = 8.0f * scale;
    float size_16 = 16.0f * scale;

    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* regs = video->GetRegisters();
    u8* vram = video->GetVRAM();
    GS_RuntimeInfo runtime;
    emu_get_runtime(runtime);
    bool isGG = core->GetCartridge()->IsGameGear() && !core->GetCartridge()->IsGameGearInSMSMode();
    bool isSG1000 = video->IsSG1000Mode();
    bool sprites_16 = IsSetBit(regs[1], 1);

    float width = 0.0f;
    float height = 0.0f;

    if (isSG1000)
    {
        width = sprites_16 ? size_16 : size_8;
        height = sprites_16 ? size_16 : size_8;
    }
    else
    {
        width = size_8;
        height = sprites_16 ? size_16 : size_8;
    }

    ImVec2 spr_pos[64];

    ImGuiIO& io = ImGui::GetIO();

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "spr", false);
    ImGui::SetColumnOffset(1, (sprites_16 && isSG1000) ? 330.0f : 200.0f);

    ImGui::BeginChild("sprites", ImVec2(0, 0.0f), true);

    bool child_hovered = ImGui::IsWindowHovered();

    static int selected_sprite = -1;
    int hovered_sprite = -1;

    for (int s = 0; s < 64; s++)
    {
        spr_pos[s] = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_sprites[s], ImVec2(width, height), ImVec2(0.0f, 0.0f), ImVec2((1.0f / 16.0f) * (width / scale), (1.0f / 16.0f) * (height / scale)));

        draw_context_menu_sprites(s);

        float mx = io.MousePos.x - spr_pos[s].x;
        float my = io.MousePos.y - spr_pos[s].y;

        bool is_hovered = child_hovered && (mx >= 0.0f) && (mx < width) && (my >= 0.0f) && (my < height);

        if (is_hovered)
        {
            hovered_sprite = s;
            if (ImGui::IsMouseClicked(0))
                selected_sprite = (selected_sprite == s) ? -1 : s;
        }

        if (is_hovered || selected_sprite == s)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRect(ImVec2(spr_pos[s].x, spr_pos[s].y), ImVec2(spr_pos[s].x + width, spr_pos[s].y + height), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
        }

        if (s % 4 < 3)
            ImGui::SameLine();
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    ImVec2 p_screen = ImGui::GetCursorScreenPos();

    float screen_scale = 1.0f;
    float tex_h = (float)runtime.screen_width / (float)(SYSTEM_TEXTURE_WIDTH);
    float tex_v = (float)runtime.screen_height / (float)(SYSTEM_TEXTURE_HEIGHT);

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_texture, ImVec2(runtime.screen_width * screen_scale, runtime.screen_height * screen_scale), ImVec2(0, 0), ImVec2(tex_h, tex_v));

    int display_sprite = (hovered_sprite >= 0) ? hovered_sprite : selected_sprite;

    if (display_sprite >= 0)
    {
        int s = display_sprite;
        int x = 0;
        int y = 0;
        int tile = 0;
        int sprite_tile_addr = 0;
        int sprite_shift = 0;
        float real_x = 0.0f;
        float real_y = 0.0f;

        if (isSG1000)
        {
            u16 sprite_attribute_addr = (regs[5] & 0x7F) << 7;
            u16 sprite_pattern_addr = (regs[6] & 0x07) << 11;
            int sprite_attribute_offset = sprite_attribute_addr + (s << 2);
            tile = vram[sprite_attribute_offset + 2];
            sprite_tile_addr = sprite_pattern_addr + (tile << 3);
            sprite_shift = (vram[sprite_attribute_offset + 3] & 0x80) ? 32 : 0;
            x = vram[sprite_attribute_offset + 1];
            y = vram[sprite_attribute_offset];

            int final_y = (y + 1) & 0xFF;

            if (final_y >= 0xE0)
                final_y = -(0x100 - final_y);

            real_x = (float)(x - sprite_shift);
            real_y = (float)final_y;
        }
        else
        {
            sprite_shift = IsSetBit(regs[0], 3) ? 8 : 0;
            u16 sprite_table_address = (regs[5] << 7) & 0x3F00;
            u16 sprite_table_address_2 = sprite_table_address + 0x80;
            u16 sprite_info_address = sprite_table_address_2 + (s << 1);
            u16 sprite_tiles_address = (regs[6] << 11) & 0x2000;
            y = vram[sprite_table_address + s];
            x = vram[sprite_info_address];
            tile = vram[sprite_info_address + 1];
            tile &= sprites_16 ? 0xFE : 0xFF;
            sprite_tile_addr = sprite_tiles_address + (tile << 5);

            real_x = (float)(x - sprite_shift - (isGG ? GS_RESOLUTION_GG_X_OFFSET : 0));
            real_y = (float)(y + 1.0f - (isGG ? GS_RESOLUTION_GG_Y_OFFSET : 0));
        }

        float max_width = 8.0f;
        float max_height = sprites_16 ? 16.0f : 8.0f;

        if (isSG1000)
        {
            if (sprites_16)
                max_width = 16.0f;

            if (IsSetBit(regs[1], 0))
            {
                max_width *= 2.0f;
                max_height *= 2.0f;
            }
        }

        float rectx_min = p_screen.x + (real_x * screen_scale);
        float rectx_max = p_screen.x + ((real_x + max_width) * screen_scale);
        float recty_min = p_screen.y + (real_y * screen_scale);
        float recty_max = p_screen.y + ((real_y + max_height) * screen_scale);

        rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
        rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (runtime.screen_width * screen_scale));
        recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));
        recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (runtime.screen_height * screen_scale));

        float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
        ImVec4 pulse_color = ImVec4(
            red.x + (white.x - red.x) * t,
            red.y + (white.y - red.y) * t,
            red.z + (white.z - red.z) * t,
            1.0f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::TextColored(green, "DETAILS:");
        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", x); ImGui::SameLine();
        ImGui::TextColored(cyan, "  Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", y); ImGui::SameLine();

        ImGui::TextColored(cyan, "  Tile:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile);

        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", sprite_tile_addr);

        ImGui::TextColored(cyan, " Horizontal Sprite Shift:"); ImGui::SameLine();
        sprite_shift > 0 ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF");
    }

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_vram_palettes(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(648, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(502, 174), ImGuiCond_FirstUseEver);

    ImGui::Begin("Palettes", &config_debug.show_video_palettes);

    GearsystemCore* core = emu_get_core();
    Video* video = core->GetVideo();
    u8* palettes = video->GetCRAM();
    bool isGG = core->GetCartridge()->IsGameGear();

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(green, "PALETTE 0 (BG):");

    for (int i = 0; i < 2; i++)
    {
        ImGui::Text(" "); ImGui::SameLine(31.0f);
        for (int c = 0; c < 16; c++)
        {
            ImVec4 float_color;
            if (isGG)
            {
                u8 color_lo = palettes[(i << 5) + (c << 1)];
                u8 color_hi = palettes[(i << 5) + (c << 1) + 1];
                u16 color = color_hi << 8 | color_lo;
                float_color = color_444_to_float(color);
            }
            else
            {
                u8 color = palettes[(i << 4) + c];
                float_color = color_222_to_float(color);
            }

            char id[16];
            snprintf(id, sizeof(id), "##pal_%d_%d", i, c);
            ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 15)
            {
                ImGui::SameLine(31.0f + (29.0f * (c + 1)));
            }
        }

        ImGui::Text("  "); ImGui::SameLine();

        for (int c = 0; c < 16; c++)
        {
            if (isGG)
            {
                u8 color_lo = palettes[(i << 5) + (c << 1)];
                u8 color_hi = palettes[(i << 5) + (c << 1) + 1];
                u16 color = color_hi << 8 | color_lo;
                ImGui::Text("%03X", color);
            }
            else
            {
                u8 color = palettes[(i << 4) + c];
                ImGui::Text("$%02X", color);
            }

            if (c < 15)
                ImGui::SameLine();
        }

        if (i == 0)
        {
            ImGui::TextColored(green, " ");
            ImGui::TextColored(green, "PALETTE 1 (BG & SPRITES):");
        }
    }

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_window_vram_regs(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(648, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(306, 240), ImGuiCond_FirstUseEver);

    ImGui::Begin("VDP Registers", &config_debug.show_video_regs);

    ImGui::PushFont(gui_default_font);

    Video* video = emu_get_core()->GetVideo();
    u8* regs = video->GetRegisters();

    const char* reg_desc[] = {"CONTROL 1     ", "CONTROL 2     ", "NAME TABLE    ", "COLOR TABLE   ", "PATTERN TABLE ", "SPRITE ATTR   ", "SPRITE PATTERN", "BACKDROP COLOR", "H SCROLL      ", "V SCROLL      ", "V INTERRUPT   "};

    ImGui::TextColored(green, "VDP REGISTERS:");

    for (int i = 0; i < 11; i++)
    {
        ImGui::TextColored(cyan, " REG $%01X ", i); ImGui::SameLine();
        ImGui::TextColored(violet, "%s ", reg_desc[i]); ImGui::SameLine();
        ImGui::Text("$%02X  ", regs[i]); ImGui::SameLine(0, 0);
        ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(regs[i]));
    }

    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleVar();
}

static ImVec4 color_444_to_float(u16 color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 15.0f) * (color & 0xF);
    ret.y = (1.0f / 15.0f) * ((color >> 4) & 0xF);
    ret.z = (1.0f / 15.0f) * ((color >> 8) & 0xF);
    return ret;
}

static ImVec4 color_222_to_float(u8 color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 3.0f) * (color & 0x3);
    ret.y = (1.0f / 3.0f) * ((color >> 2) & 0x3);
    ret.z = (1.0f / 3.0f) * ((color >> 4) & 0x3);
    return ret;
}

static void draw_context_menu_sprites(int index)
{
    ImGui::PopFont();

    char ctx_id[16];
    snprintf(ctx_id, sizeof(ctx_id), "##spr_ctx_%02d", index);

    if (ImGui::BeginPopupContextItem(ctx_id))
    {
        if (ImGui::Selectable("Save Sprite As..."))
            gui_file_dialog_save_sprite(index);
        if (ImGui::Selectable("Save All Sprites To Folder..."))
            gui_file_dialog_save_all_sprites();

        ImGui::EndPopup();
    }

    ImGui::PushFont(gui_default_font);
}

static void draw_context_menu_background(void)
{
    ImGui::PopFont();

    if (ImGui::BeginPopupContextItem("##bg_ctx"))
    {
        if (ImGui::Selectable("Save Background As..."))
            gui_file_dialog_save_background();

        ImGui::EndPopup();
    }

    ImGui::PushFont(gui_default_font);
}

static void draw_context_menu_tiles(void)
{
    if (ImGui::BeginPopupContextItem("##tiles_ctx"))
    {
        if (ImGui::Selectable("Save Pattern Table As..."))
            gui_file_dialog_save_tiles();

        ImGui::EndPopup();
    }
}

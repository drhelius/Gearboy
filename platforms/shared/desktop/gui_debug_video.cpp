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

#define GUI_DEBUG_VIDEO_IMPORT
#include "gui_debug_video.h"

#include "imgui.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "ogl_renderer.h"
#include "gui_filedialogs.h"
#include "gui_debug_memory.h"
#include "utils.h"

static ImVec4 color_565_to_float(u16 color)
{
    ImVec4 ret;
    ret.w = 0;
    ret.x = (1.0f / 31.0f) * ((color >> 11) & 0x1F);
    ret.y = (1.0f / 63.0f) * ((color >> 5) & 0x3F);
    ret.z = (1.0f / 31.0f) * (color & 0x1F);
    return ret;
}

void gui_debug_window_vram_nametable(void)
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(542, 538), ImGuiCond_FirstUseEver);

    ImGui::Begin("Name Table", &config_debug.show_video_nametable);

    static int selected_bg_tile_x = -1;
    static int selected_bg_tile_y = -1;

    if (ImGui::IsWindowAppearing())
    {
        selected_bg_tile_x = -1;
        selected_bg_tile_y = -1;
    }

    Memory* memory = emu_get_core()->GetMemory();

    bool window_hovered = ImGui::IsWindowHovered();
    static bool show_grid = true;
    static bool show_screen = true;
    static int tile_address_radio = 0;
    static int map_address_radio = 0;
    float scale = 1.5f;
    float size = 256.0f * scale;
    float spacing = 8.0f * scale;

    static int layer_radio = 0;
    ImGui::RadioButton("Background", &layer_radio, 0); ImGui::SameLine();
    ImGui::RadioButton("Window", &layer_radio, 1);
    emu_debug_background_is_window = (layer_radio == 1);

    ImGui::Checkbox("Show Grid##grid_bg", &show_grid); ImGui::SameLine();
    ImGui::Checkbox("Show Screen Rect", &show_screen);

    ImGui::PushFont(gui_default_font);

    ImGui::Columns(2, "bg", false);
    ImGui::SetColumnOffset(1, size + 10.0f);

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_background, ImVec2(size, size));

    ImGui::PopFont();

    if (ImGui::BeginPopupContextItem("##bg_ctx"))
    {
        if (ImGui::Selectable("Save Background As..."))
            gui_file_dialog_save_background();

        ImGui::EndPopup();
    }

    ImGui::PushFont(gui_default_font);

    if (show_grid)
    {
        float x = p.x;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(x, p.y), ImVec2(x, p.y + size), ImColor(dark_gray), 1.0f);
            x += spacing;
        }

        float y = p.y;
        for (int n = 0; n <= 32; n++)
        {
            draw_list->AddLine(ImVec2(p.x, y), ImVec2(p.x + size, y), ImColor(dark_gray), 1.0f);
            y += spacing;
        }
    }

    if (show_screen)
    {
        if (emu_debug_background_is_window)
        {
            u8 wx = memory->Retrieve(0xFF4B);
            u8 wy = memory->Retrieve(0xFF4A);
            int visible_w = GAMEBOY_WIDTH - (wx - 7);
            int visible_h = GAMEBOY_HEIGHT - wy;
            if (visible_w > 0 && visible_h > 0)
            {
                if (visible_w > 256) visible_w = 256;
                if (visible_h > 256) visible_h = 256;
                float rect_x_min = p.x;
                float rect_y_min = p.y;
                float rect_x_max = p.x + (visible_w * scale);
                float rect_y_max = p.y + (visible_h * scale);
                draw_list->AddRect(ImVec2(rect_x_min, rect_y_min), ImVec2(rect_x_max, rect_y_max), ImColor(green), 0.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            }
        }
        else
        {
            u8 scroll_x = memory->Retrieve(0xFF43);
            u8 scroll_y = memory->Retrieve(0xFF42);

            float grid_x_max = p.x + size;
            float grid_y_max = p.y + size;

            float rect_x_min = p.x + (scroll_x * scale);
            float rect_y_min = p.y + (scroll_y * scale);
            float rect_x_max = p.x + ((scroll_x + GAMEBOY_WIDTH) * scale);
            float rect_y_max = p.y + ((scroll_y + GAMEBOY_HEIGHT) * scale);

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
    }

    float mouse_x = io.MousePos.x - p.x;
    float mouse_y = io.MousePos.y - p.y;

    int hovered_tile_x = -1;
    int hovered_tile_y = -1;

    if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < size) && (mouse_y >= 0.0f) && (mouse_y < size))
    {
        hovered_tile_x = (int)(mouse_x / spacing);
        hovered_tile_y = (int)(mouse_y / spacing);

        if (ImGui::IsMouseClicked(0))
        {
            if (selected_bg_tile_x == hovered_tile_x && selected_bg_tile_y == hovered_tile_y)
            {
                selected_bg_tile_x = -1;
                selected_bg_tile_y = -1;
            }
            else
            {
                selected_bg_tile_x = hovered_tile_x;
                selected_bg_tile_y = hovered_tile_y;
            }

            u8 lcdc = memory->Retrieve(0xFF40);
            int ts_addr = emu_debug_background_tile_address >= 0 ? emu_debug_background_tile_address : IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
            int ms_addr = emu_debug_background_map_address >= 0 ? emu_debug_background_map_address : (emu_debug_background_is_window ? (IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800) : (IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800));
            u16 m_addr = ms_addr + (32 * hovered_tile_y) + hovered_tile_x;
            int m_tile = (ts_addr == 0x8800) ? (static_cast<s8>(memory->Retrieve(m_addr)) + 128) : memory->Retrieve(m_addr);
            gui_debug_memory_goto(MEMORY_EDITOR_VRAM, ts_addr + (m_tile << 4));
        }

        if (!(hovered_tile_x == selected_bg_tile_x && hovered_tile_y == selected_bg_tile_y))
            draw_list->AddRect(ImVec2(p.x + (hovered_tile_x * spacing), p.y + (hovered_tile_y * spacing)), ImVec2(p.x + ((hovered_tile_x + 1) * spacing), p.y + ((hovered_tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
    }

    if (selected_bg_tile_x >= 0 && selected_bg_tile_y >= 0)
    {
        float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
        ImVec4 pulse_color = ImVec4(red.x + (white.x - red.x) * t, red.y + (white.y - red.y) * t, red.z + (white.z - red.z) * t, 1.0f);
        draw_list->AddRect(ImVec2(p.x + (selected_bg_tile_x * spacing), p.y + (selected_bg_tile_y * spacing)), ImVec2(p.x + ((selected_bg_tile_x + 1) * spacing), p.y + ((selected_bg_tile_y + 1) * spacing)), ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
    }

    int tile_x = (hovered_tile_x >= 0) ? hovered_tile_x : selected_bg_tile_x;
    int tile_y = (hovered_tile_y >= 0) ? hovered_tile_y : selected_bg_tile_y;

    if (tile_x >= 0 && tile_y >= 0)
    {

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_background, ImVec2(128.0f, 128.0f), ImVec2((1.0f / 32.0f) * tile_x, (1.0f / 32.0f) * tile_y), ImVec2((1.0f / 32.0f) * (tile_x + 1), (1.0f / 32.0f) * (tile_y + 1)));

        ImGui::TextColored(magenta, "DMG:");

        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_x); ImGui::SameLine();
        ImGui::TextColored(cyan, "   Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile_y);

        u8 lcdc = memory->Retrieve(0xFF40);

        int tile_start_addr = emu_debug_background_tile_address >= 0 ? emu_debug_background_tile_address : IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
        int map_start_addr = emu_debug_background_map_address >= 0 ? emu_debug_background_map_address : (emu_debug_background_is_window ? (IsSetBit(lcdc, 6) ? 0x9C00 : 0x9800) : (IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800));

        u16 map_addr = map_start_addr + (32 * tile_y) + tile_x;

        ImGui::TextColored(cyan, " Map Addr: "); ImGui::SameLine();
        ImGui::Text("$%04X", map_addr);

        int map_tile = 0;

        if (tile_start_addr == 0x8800)
        {
            map_tile = static_cast<s8>(memory->Retrieve(map_addr));
            map_tile += 128;
        }
        else
        {
            map_tile = memory->Retrieve(map_addr);
        }

        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", tile_start_addr + (map_tile << 4));
        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%02X", memory->Retrieve(map_addr));

        if (emu_is_cgb())
        {
            ImGui::TextColored(magenta, "GBC:");

            u8 cgb_tile_attr = memory->ReadCGBLCDRAM(map_addr, true);
            int cgb_tile_pal = cgb_tile_attr & 0x07;
            int cgb_tile_bank = IsSetBit(cgb_tile_attr, 3) ? 1 : 0;
            bool cgb_tile_xflip = IsSetBit(cgb_tile_attr, 5);
            bool cgb_tile_yflip = IsSetBit(cgb_tile_attr, 6);

            ImGui::TextColored(cyan, " Attributes:"); ImGui::SameLine();
            ImGui::Text("$%02X", cgb_tile_attr);
            ImGui::TextColored(cyan, " Palette:"); ImGui::SameLine();
            ImGui::Text("%d", cgb_tile_pal);
            ImGui::TextColored(cyan, " Bank:"); ImGui::SameLine();
            ImGui::Text("%d", cgb_tile_bank);

            ImGui::TextColored(cyan, " X-Flip:"); ImGui::SameLine();
            cgb_tile_xflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

            ImGui::TextColored(cyan, " Y-Flip:"); ImGui::SameLine();
            cgb_tile_yflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

            bool cgb_tile_priority = IsSetBit(cgb_tile_attr, 7);
            ImGui::TextColored(cyan, " BG Priority:"); ImGui::SameLine();
            cgb_tile_priority ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");
        }
    }

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::Text("Tile address:"); ImGui::SameLine();
    ImGui::RadioButton("Auto##tile", &tile_address_radio, 0); ImGui::SameLine();
    ImGui::RadioButton("0x8000", &tile_address_radio, 1); ImGui::SameLine();
    ImGui::RadioButton("0x8800", &tile_address_radio, 2);

    switch (tile_address_radio)
    {
    case 0: emu_debug_background_tile_address = -1; break;
    case 1: emu_debug_background_tile_address = 0x8000; break;
    case 2: emu_debug_background_tile_address = 0x8800; break;
    default: emu_debug_background_tile_address = -1; break;
    }

    ImGui::Text("Map address:"); ImGui::SameLine();
    ImGui::RadioButton("Auto##map", &map_address_radio, 0); ImGui::SameLine();
    ImGui::RadioButton("0x9C00", &map_address_radio, 1); ImGui::SameLine();
    ImGui::RadioButton("0x9800", &map_address_radio, 2);

    switch (map_address_radio)
    {
    case 0: emu_debug_background_map_address = -1; break;
    case 1: emu_debug_background_map_address = 0x9C00; break;
    case 2: emu_debug_background_map_address = 0x9800; break;
    default: emu_debug_background_map_address = -1; break;
    }

    ImGui::End();
}

void gui_debug_window_vram_tiles(void)
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(546, 360), ImGuiCond_FirstUseEver);

    ImGui::Begin("Pattern Table", &config_debug.show_video_tiles);

    static int selected_tile_x = -1;
    static int selected_tile_y = -1;
    static int selected_tile_bank = -1;

    if (ImGui::IsWindowAppearing())
    {
        selected_tile_x = -1;
        selected_tile_y = -1;
        selected_tile_bank = -1;
    }

    static bool show_grid = true;
    bool window_hovered = ImGui::IsWindowHovered();
    float scale = 1.5f;
    float width = 8.0f * 16.0f * scale;
    float height = 8.0f * 24.0f * scale;
    float spacing = 8.0f * scale;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p[2];

    ImGui::Checkbox("Show Grid##grid_tiles", &show_grid);
    ImGui::SameLine(150.0f);

    ImGui::PushItemWidth(80.0f);

    if (!emu_is_cgb())
    {
        ImGui::Combo("Palette##dmg_tile_palette", &emu_debug_tile_dmg_palette, "BGP\0OBP0\0OBP1\0\0");
    }
    else
    {
        ImGui::Combo("Palette##cgb_tile_palette", &emu_debug_tile_color_palette, "BCP0\0BCP1\0BCP2\0BCP3\0BCP4\0BCP5\0BCP6\0BCP7\0OCP0\0OCP1\0OCP2\0OCP3\0OCP4\0OCP5\0OCP6\0OCP7\0\0");
    }

    ImGui::PopItemWidth();

    ImGui::Columns(2, "bg", false);
    ImGui::SetColumnOffset(1, (width * 2.0f) + 16.0f);

    p[0] = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_tiles[0], ImVec2(width, height), ImVec2(0, 0), ImVec2(1.0f, 0.75f));

    ImGui::SameLine();

    p[1] = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_tiles[1], ImVec2(width, height), ImVec2(0, 0), ImVec2(1.0f, 0.75f));

    for (int i = 0; i < 2; i++)
    {
        if (show_grid)
        {
            float x = p[i].x;
            for (int n = 0; n <= 16; n++)
            {
                draw_list->AddLine(ImVec2(x, p[i].y), ImVec2(x, p[i].y + height), ImColor(dark_gray), 1.0f);
                x += spacing;
            }

            float y = p[i].y;
            for (int n = 0; n <= 24; n++)
            {
                draw_list->AddLine(ImVec2(p[i].x, y), ImVec2(p[i].x + width, y), ImColor(dark_gray), ((n == 8) || (n == 16)) ? 3.0f : 1.0f);
                y += spacing;
            }
        }
    }

    int hovered_tile_x = -1;
    int hovered_tile_y = -1;
    int hovered_tile_bank = -1;

    for (int i = 0; i < 2; i++)
    {
        float mouse_x = io.MousePos.x - p[i].x;
        float mouse_y = io.MousePos.y - p[i].y;

        if (window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < height))
        {
            hovered_tile_x = (int)(mouse_x / spacing);
            hovered_tile_y = (int)(mouse_y / spacing);
            hovered_tile_bank = i;

            if (ImGui::IsMouseClicked(0))
            {
                if (selected_tile_x == hovered_tile_x && selected_tile_y == hovered_tile_y && selected_tile_bank == i)
                {
                    selected_tile_x = -1;
                    selected_tile_y = -1;
                    selected_tile_bank = -1;
                }
                else
                {
                    selected_tile_x = hovered_tile_x;
                    selected_tile_y = hovered_tile_y;
                    selected_tile_bank = i;
                }

                int tile_full = (hovered_tile_y << 4) + hovered_tile_x;
                gui_debug_memory_goto(MEMORY_EDITOR_VRAM, 0x8000 + (tile_full << 4));
            }

            if (!(hovered_tile_x == selected_tile_x && hovered_tile_y == selected_tile_y && i == selected_tile_bank))
                draw_list->AddRect(ImVec2(p[i].x + (hovered_tile_x * spacing), p[i].y + (hovered_tile_y * spacing)), ImVec2(p[i].x + ((hovered_tile_x + 1) * spacing), p[i].y + ((hovered_tile_y + 1) * spacing)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
        }
    }

    if (selected_tile_x >= 0 && selected_tile_bank >= 0)
    {
        float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
        ImVec4 pulse_color = ImVec4(red.x + (white.x - red.x) * t, red.y + (white.y - red.y) * t, red.z + (white.z - red.z) * t, 1.0f);
        draw_list->AddRect(ImVec2(p[selected_tile_bank].x + (selected_tile_x * spacing), p[selected_tile_bank].y + (selected_tile_y * spacing)), ImVec2(p[selected_tile_bank].x + ((selected_tile_x + 1) * spacing), p[selected_tile_bank].y + ((selected_tile_y + 1) * spacing)), ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
    }

    int display_tile_x = (hovered_tile_x >= 0) ? hovered_tile_x : selected_tile_x;
    int display_tile_y = (hovered_tile_y >= 0) ? hovered_tile_y : selected_tile_y;
    int display_tile_bank = (hovered_tile_bank >= 0) ? hovered_tile_bank : selected_tile_bank;

    if (display_tile_x >= 0 && display_tile_y >= 0 && display_tile_bank >= 0)
    {

        ImGui::NextColumn();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_tiles[display_tile_bank], ImVec2(128.0f, 128.0f), ImVec2((1.0f / 16.0f) * display_tile_x, (1.0f / 32.0f) * display_tile_y), ImVec2((1.0f / 16.0f) * (display_tile_x + 1), (1.0f / 32.0f) * (display_tile_y + 1)));

        ImGui::PushFont(gui_default_font);

        ImGui::TextColored(magenta, "DETAILS:");

        int tile_full = (display_tile_y << 4) + display_tile_x;
        int tile = tile_full & 0xFF;

        ImGui::TextColored(cyan, " Tile Number:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile);
        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", 0x8000 + (tile_full << 4));
        ImGui::TextColored(cyan, " Bank:"); ImGui::SameLine();
        ImGui::Text("%d", display_tile_bank);

        ImGui::PopFont();
    }

    ImGui::Columns(1);

    if (ImGui::BeginPopupContextItem("##tiles_ctx"))
    {
        if (ImGui::Selectable("Save Tiles As..."))
            gui_file_dialog_save_tiles();

        ImGui::EndPopup();
    }

    ImGui::End();
}

void gui_debug_window_vram_sprites(void)
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(540, 406), ImGuiCond_FirstUseEver);

    ImGui::Begin("Sprites (OAM)", &config_debug.show_video_sprites);

    static int selected_sprite = -1;

    if (ImGui::IsWindowAppearing())
        selected_sprite = -1;

    float scale = 5.0f;
    float width = 8.0f * scale;
    float height_8 = 8.0f * scale;
    float height_16 = 16.0f * scale;

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();

    ImVec2 p[40];

    ImGuiIO& io = ImGui::GetIO();

    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_16 = IsSetBit(lcdc, 2);

    ImGui::Columns(2, "oam", false);
    ImGui::SetColumnOffset(1, 280.0f);

    ImGui::BeginChild("sprites", ImVec2(0, 0), true);

    bool window_hovered = ImGui::IsWindowHovered();

    int hovered_sprite = -1;

    ImGui::PushFont(gui_default_font);

    for (int s = 0; s < 40; s++)
    {
        p[s] = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_debug_vram_sprites[s], ImVec2(width, sprites_16 ? height_16 : height_8), ImVec2(0.0f, 0.0f), ImVec2(1.0f, sprites_16 ? 1.0f : 0.5f));

        ImGui::PopFont();

        char ctx_id[16];
        snprintf(ctx_id, sizeof(ctx_id), "##spr_ctx_%02d", s);

        if (ImGui::BeginPopupContextItem(ctx_id))
        {
            if (ImGui::Selectable("Save Sprite As..."))
                gui_file_dialog_save_sprite(s);
            if (ImGui::Selectable("Save All Sprites To Folder..."))
                gui_file_dialog_save_all_sprites();

            ImGui::EndPopup();
        }

        ImGui::PushFont(gui_default_font);

        float mouse_x = io.MousePos.x - p[s].x;
        float mouse_y = io.MousePos.y - p[s].y;

        bool is_hovered = window_hovered && (mouse_x >= 0.0f) && (mouse_x < width) && (mouse_y >= 0.0f) && (mouse_y < (sprites_16 ? height_16 : height_8));

        if (is_hovered)
        {
            hovered_sprite = s;
            if (ImGui::IsMouseClicked(0))
                selected_sprite = (selected_sprite == s) ? -1 : s;
        }

        if (is_hovered && selected_sprite != s)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + width, p[s].y + (sprites_16 ? height_16 : height_8)), ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
        }

        if (selected_sprite == s)
        {
            float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
            ImVec4 pulse_color = ImVec4(red.x + (white.x - red.x) * t, red.y + (white.y - red.y) * t, red.z + (white.z - red.z) * t, 1.0f);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(ImVec2(p[s].x, p[s].y), ImVec2(p[s].x + width, p[s].y + (sprites_16 ? height_16 : height_8)), ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 3.0f);
        }

        if (s % 5 < 4)
            ImGui::SameLine();
    }

    ImGui::PopFont();

    ImGui::EndChild();

    ImGui::NextColumn();

    ImVec2 p_screen = ImGui::GetCursorScreenPos();

    float screen_scale = 1.5f;

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_texture, ImVec2(GAMEBOY_WIDTH * screen_scale, GAMEBOY_HEIGHT * screen_scale),
        ImVec2(0, 0), ImVec2((float)GAMEBOY_WIDTH / (float)SYSTEM_TEXTURE_WIDTH, (float)GAMEBOY_HEIGHT / (float)SYSTEM_TEXTURE_HEIGHT));

    int display_sprite = (hovered_sprite >= 0) ? hovered_sprite : selected_sprite;

    ImGui::PushFont(gui_default_font);

    if (display_sprite >= 0)
    {
        int s = display_sprite;
        u16 address = 0xFE00 + (4 * s);

        u8 y = memory->Retrieve(address);
        u8 x = memory->Retrieve(address + 1);
        u8 tile = memory->Retrieve(address + 2);
        u8 flags = memory->Retrieve(address + 3);
        int palette = IsSetBit(flags, 4) ? 1 : 0;
        bool xflip = IsSetBit(flags, 5);
        bool yflip = IsSetBit(flags, 6);
        bool priority = !IsSetBit(flags, 7);
        bool cgb_bank = IsSetBit(flags, 3);
        int cgb_pal = flags & 0x07;

        float real_x = x - 8.0f;
        float real_y = y - 16.0f;
        float rectx_min = p_screen.x + (real_x * screen_scale);
        float rectx_max = p_screen.x + ((real_x + 8.0f) * screen_scale);
        float recty_min = p_screen.y + (real_y * screen_scale);
        float recty_max = p_screen.y + ((real_y + (sprites_16 ? 16.0f : 8.0f)) * screen_scale);

        rectx_min = fminf(fmaxf(rectx_min, p_screen.x), p_screen.x + (GAMEBOY_WIDTH * screen_scale));
        rectx_max = fminf(fmaxf(rectx_max, p_screen.x), p_screen.x + (GAMEBOY_WIDTH * screen_scale));
        recty_min = fminf(fmaxf(recty_min, p_screen.y), p_screen.y + (GAMEBOY_HEIGHT * screen_scale));
        recty_max = fminf(fmaxf(recty_max, p_screen.y), p_screen.y + (GAMEBOY_HEIGHT * screen_scale));

        float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
        ImVec4 pulse_color = ImVec4(
            red.x + (white.x - red.x) * t,
            red.y + (white.y - red.y) * t,
            red.z + (white.z - red.z) * t,
            1.0f);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRect(ImVec2(rectx_min, recty_min), ImVec2(rectx_max, recty_max), ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);

        ImGui::NewLine();

        ImGui::TextColored(magenta, "DETAILS:");
        ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
        ImGui::Text("$%02X", x); ImGui::SameLine();
        ImGui::TextColored(cyan, "  Y:"); ImGui::SameLine();
        ImGui::Text("$%02X", y); ImGui::SameLine();

        ImGui::TextColored(cyan, "   Tile:"); ImGui::SameLine();
        ImGui::Text("$%02X", tile);

        ImGui::TextColored(cyan, " Tile Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", 0x8000 + (tile * 16)); ImGui::SameLine();

        ImGui::TextColored(cyan, "  Bank:"); ImGui::SameLine();
        ImGui::Text("%d", cgb_bank);

        ImGui::TextColored(cyan, " OAM Addr:"); ImGui::SameLine();
        ImGui::Text("$%04X", address); ImGui::SameLine();

        ImGui::TextColored(cyan, "  Flags:"); ImGui::SameLine();
        ImGui::Text("$%02X", flags);

        ImGui::TextColored(cyan, " Priority:"); ImGui::SameLine();
        priority ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF"); ImGui::SameLine();

        ImGui::TextColored(cyan, "  Palette:"); ImGui::SameLine();
        ImGui::Text("%d", emu_is_cgb() ? cgb_pal : palette);

        ImGui::TextColored(cyan, " X-Flip:"); ImGui::SameLine();
        xflip ? ImGui::TextColored(green, "ON ") : ImGui::TextColored(gray, "OFF"); ImGui::SameLine();

        ImGui::TextColored(cyan, "  Y-Flip:"); ImGui::SameLine();
        yflip ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");
    }

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::End();
}

void gui_debug_window_vram_dmg_palettes(void)
{
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(438, 124), ImGuiCond_FirstUseEver);

    ImGui::Begin("DMG Palettes", &config_debug.show_video_palettes);

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();
    u16* palette = core->GetDMGInternalPalette();

    ImGui::PushFont(gui_default_font);

    ImGui::TextColored(cyan, "                               0       1       2       3");

    u8 bgp = memory->Retrieve(0xFF47);
    u8 obp0 = memory->Retrieve(0xFF48);
    u8 obp1 = memory->Retrieve(0xFF49);

    ImGui::TextColored(cyan, " $FF47"); ImGui::SameLine();
    ImGui::TextColored(violet, "BGP "); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")  ", bgp, BYTE_TO_BINARY(bgp)); ImGui::SameLine();

    for (int i = 0; i < 4; i++)
    {
        int index = (bgp >> (i * 2)) & 0x03;
        int color = palette[index];
        ImVec4 float_color = color_565_to_float(color);
        char id[16];
        snprintf(id, sizeof(id), "##dmg_bgp_%d", i);
        ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker); ImGui::SameLine();
        ImGui::Text("%d  ", index);
        if (i < 3)
            ImGui::SameLine();
    }

    ImGui::TextColored(cyan, " $FF48"); ImGui::SameLine();
    ImGui::TextColored(violet, "OBP0"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")  ", obp0, BYTE_TO_BINARY(obp0)); ImGui::SameLine();

    for (int i = 0; i < 4; i++)
    {
        int index = (obp0 >> (i * 2)) & 0x03;
        int color = palette[index];
        ImVec4 float_color = color_565_to_float(color);
        char id[16];
        snprintf(id, sizeof(id), "##dmg_obp0_%d", i);
        ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker); ImGui::SameLine();
        ImGui::Text("%d  ", index);
        if (i < 3)
            ImGui::SameLine();
    }

    ImGui::TextColored(cyan, " $FF49"); ImGui::SameLine();
    ImGui::TextColored(violet, "OBP1"); ImGui::SameLine();
    ImGui::Text("$%02X (" BYTE_TO_BINARY_PATTERN_SPACED ")  ", obp1, BYTE_TO_BINARY(obp1)); ImGui::SameLine();

    for (int i = 0; i < 4; i++)
    {
        int index = (obp1 >> (i * 2)) & 0x03;
        int color = palette[index];
        ImVec4 float_color = color_565_to_float(color);
        char id[16];
        snprintf(id, sizeof(id), "##dmg_obp1_%d", i);
        ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker); ImGui::SameLine();
        ImGui::Text("%d  ", index);
        if (i < 3)
            ImGui::SameLine();
    }

    ImGui::PopFont();

    ImGui::End();
}

void gui_debug_window_vram_gbc_palettes(void)
{
    ImGui::SetNextWindowPos(ImVec2(80, 80), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 380), ImGuiCond_FirstUseEver);

    ImGui::Begin("GBC Palettes", &config_debug.show_video_gbc_palettes);

    Video* video = emu_get_core()->GetVideo();

    ImGui::PushFont(gui_default_font);

    PaletteMatrix bg_palettes = video->GetCGBBackgroundPalettes();
    PaletteMatrix sprite_palettes = video->GetCGBSpritePalettes();

    ImGui::Columns(2, "gbc_palettes");

    ImGui::TextColored(magenta, "BACKGROUND:");

    ImGui::NextColumn();

    ImGui::TextColored(magenta, "SPRITES:");

    ImGui::NextColumn();

    ImGui::Separator();

    for (int p = 0; p < 8; p++)
    {
        ImGui::TextColored(cyan, " %d ", p); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            u16 color = (*bg_palettes)[p][c][1];
            ImVec4 float_color = color_565_to_float(color);
            char id[16];
            snprintf(id, sizeof(id), "##cgb_bg_%d_%d", p, c);
            ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 3)
            {
                ImGui::SameLine(); ImGui::Dummy(ImVec2(8.0f, 0.0f));
                ImGui::SameLine();
            }
        }

        ImGui::Text("  "); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            u16 color = (*bg_palettes)[p][c][1];
            ImGui::Text("%04X ", color);
            if (c < 3)
                ImGui::SameLine();
        }
    }

    ImGui::NextColumn();

    for (int p = 0; p < 8; p++)
    {
        ImGui::TextColored(cyan, " %d ", p); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            u16 color = (*sprite_palettes)[p][c][1];
            ImVec4 float_color = color_565_to_float(color);
            char id[16];
            snprintf(id, sizeof(id), "##cgb_spr_%d_%d", p, c);
            ImGui::ColorEdit3(id, (float*)&float_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 3)
            {
                ImGui::SameLine(); ImGui::Dummy(ImVec2(8.0f, 0.0f));
                ImGui::SameLine();
            }
        }

        ImGui::Text("  "); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            u16 color = (*sprite_palettes)[p][c][1];
            ImGui::Text("%04X ", color);
            if (c < 3)
                ImGui::SameLine();
        }
    }

    ImGui::Columns(1);

    ImGui::PopFont();

    ImGui::End();
}

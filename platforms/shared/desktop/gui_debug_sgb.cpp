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

#define GUI_DEBUG_SGB_IMPORT
#include "gui_debug_sgb.h"

#include "imgui.h"
#include "gearboy.h"
#include "gui_debug_constants.h"
#include "gui.h"
#include "config.h"
#include "emu.h"
#include "gui_debug_memory.h"
#include "gui_filedialogs.h"

static ImVec4 sgb_color_to_float(u16 rgb15)
{
    u8 r = (rgb15) & 0x1F;
    u8 g = (rgb15 >> 5) & 0x1F;
    u8 b = (rgb15 >> 10) & 0x1F;
    return ImVec4(r / 31.0f, g / 31.0f, b / 31.0f, 1.0f);
}

static const char* sgb_mask_mode_name(int mode)
{
    switch (mode)
    {
        case 0: return "DISABLED";
        case 1: return "FREEZE";
        case 2: return "BLACK";
        case 3: return "COLOR 0";
        default: return "UNKNOWN";
    }
}

static const char* sgb_transfer_dest_name(int dest)
{
    switch (dest)
    {
        case 0: return "LOW TILES";
        case 1: return "HIGH TILES";
        case 2: return "BORDER DATA";
        case 3: return "PALETTES";
        case 4: return "ATTRIBUTES";
        default: return "UNKNOWN";
    }
}

static const char* sgb_command_name(u8 code)
{
    switch (code)
    {
        case 0x00: return "PAL01";
        case 0x01: return "PAL23";
        case 0x02: return "PAL03";
        case 0x03: return "PAL12";
        case 0x04: return "ATTR_BLK";
        case 0x05: return "ATTR_LIN";
        case 0x06: return "ATTR_DIV";
        case 0x07: return "ATTR_CHR";
        case 0x08: return "SOUND";
        case 0x09: return "SOU_TRN";
        case 0x0A: return "PAL_SET";
        case 0x0B: return "PAL_TRN";
        case 0x0F: return "DATA_SND";
        case 0x11: return "MLT_REQ";
        case 0x13: return "CHR_TRN";
        case 0x14: return "PCT_TRN";
        case 0x15: return "ATTR_TRN";
        case 0x16: return "ATTR_SET";
        case 0x17: return "MASK_EN";
        default: return "???";
    }
}

void gui_debug_window_sgb_state(void)
{
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(210, 376), ImGuiCond_FirstUseEver);

    ImGui::Begin("SGB State", &config_debug.show_sgb_state);

    ImGui::PushFont(gui_default_font);

    GearboyCore* core = emu_get_core();
    SGB* sgb = core->GetSGB();

    ImGui::TextColored(magenta, "MODE:");

    ImGui::TextColored(violet, " SGB ACTIVE "); ImGui::SameLine();
    core->IsSGB() ? ImGui::TextColored(green, "YES") : ImGui::TextColored(gray, "NO");

    ImGui::TextColored(violet, " MASK MODE  "); ImGui::SameLine();
    int mask = sgb->GetMaskMode();
    ImGui::Text("%d (%s)", mask, sgb_mask_mode_name(mask));

    ImGui::TextColored(violet, " COMMANDS   "); ImGui::SameLine();
    sgb->AreCommandsDisabled() ? ImGui::TextColored(red, "DISABLED") : ImGui::TextColored(green, "ENABLED");

    ImGui::Separator();

    ImGui::TextColored(magenta, "MULTIPLAYER:");

    ImGui::TextColored(violet, " PLAYERS    "); ImGui::SameLine();
    ImGui::Text("%d", sgb->GetPlayerCount());

    ImGui::TextColored(violet, " CURRENT    "); ImGui::SameLine();
    ImGui::Text("%d", sgb->GetCurrentPlayer());

    ImGui::Separator();

    ImGui::TextColored(magenta, "COMMAND:");

    const u8* cmd = sgb->GetCommand();
    u8 cmdCode = cmd[0] >> 3;
    u8 cmdLen = cmd[0] & 7;

    ImGui::NewLine();

    for (int i = 0; i < 8; i++)
    {
        ImGui::SameLine();
        ImGui::TextColored(yellow, "%02X", cmd[i]);
    }

    ImGui::TextColored(violet, " LAST CODE  "); ImGui::SameLine();
    ImGui::Text("$%02X (%s)", cmdCode, sgb_command_name(cmdCode));

    ImGui::TextColored(violet, " LENGTH     "); ImGui::SameLine();
    ImGui::Text("%d packet(s)", cmdLen);

    ImGui::TextColored(violet, " WRITE IDX  "); ImGui::SameLine();
    ImGui::Text("%d / %d bits", sgb->GetCommandWriteIndex(), cmdLen * SGB_PACKET_SIZE * 8);

    ImGui::TextColored(violet, " FLAGS      "); ImGui::SameLine();
    ImGui::Text("P:%d W:%d S:%d",
        sgb->IsReadyForPulse() ? 1 : 0,
        sgb->IsReadyForWrite() ? 1 : 0,
        sgb->IsReadyForStop() ? 1 : 0);

    ImGui::Separator();

    ImGui::TextColored(magenta, "TRANSFER:");

    ImGui::TextColored(violet, " COUNTDOWN  "); ImGui::SameLine();

    u8 countdown = sgb->GetVRAMTransferCountdown();

    if (countdown > 0)
        ImGui::TextColored(yellow, "%d", countdown);
    else
        ImGui::TextColored(gray, "IDLE");

    ImGui::TextColored(violet, " DEST       "); ImGui::SameLine();
    ImGui::Text("%s", sgb_transfer_dest_name(sgb->GetTransferDest()));

    ImGui::Separator();

    ImGui::TextColored(magenta, "BORDER:");

    ImGui::TextColored(violet, " ANIMATION  "); ImGui::SameLine();

    u8 anim = sgb->GetBorderAnimation();

    if (anim > 0)
        ImGui::TextColored(yellow, "%d", anim);
    else
        ImGui::TextColored(gray, "NONE");

    ImGui::TextColored(violet, " EMPTY      "); ImGui::SameLine();
    sgb->IsBorderEmpty() ? ImGui::TextColored(gray, "YES") : ImGui::TextColored(green, "NO");

    ImGui::PopFont();

    ImGui::End();
}

void gui_debug_window_sgb_video(void)
{
    ImGui::SetNextWindowPos(ImVec2(120, 120), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(668, 540), ImGuiCond_FirstUseEver);

    ImGui::Begin("SGB Video", &config_debug.show_sgb_video);

    GearboyCore* core = emu_get_core();
    SGB* sgb = core->GetSGB();

    if (ImGui::BeginTabBar("SGBVideoTabs"))
    {
        if (ImGui::BeginTabItem("Border Tilemap"))
        {
            const SGB::Border* border = sgb->GetBorder();

            static bool show_grid = true;
            static int selected_tile_x = -1;
            static int selected_tile_y = -1;

            if (ImGui::IsWindowAppearing())
            {
                selected_tile_x = -1;
                selected_tile_y = -1;
            }

            ImGui::Checkbox("Show Grid", &show_grid);

            ImGui::PushFont(gui_default_font);

            bool window_hovered = ImGui::IsWindowHovered();
            float scale = 2.0f;
            float tile_size = 8.0f * scale;
            float width = 32.0f * tile_size;
            float height = 28.0f * tile_size;

            ImGui::Columns(2, "sgb_border", false);
            ImGui::SetColumnOffset(1, width + 10.0f);

            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImGuiIO& io = ImGui::GetIO();

            ImVec4 borderColors[16 * 4];
            for (int i = 0; i < 16 * 4; i++)
            {
                u16 raw = border->palette[i];
                borderColors[i] = sgb_color_to_float(raw);
            }

            ImVec4 gameColor0 = sgb_color_to_float(sgb->GetEffectivePalettes()[0]);

            ImGui::InvisibleButton("##border_canvas", ImVec2(width, height));

            ImGui::PopFont();

            if (ImGui::BeginPopupContextItem("##sgb_border_ctx"))
            {
                if (ImGui::Selectable("Save Border As..."))
                    gui_file_dialog_save_sgb_border();
                ImGui::EndPopup();
            }

            ImGui::PushFont(gui_default_font);

            for (int ty = 0; ty < 28; ty++)
            {
                for (int tx = 0; tx < 32; tx++)
                {
                    u16 tileEntry = border->map[tx + ty * 32];

                    if (tileEntry & 0x300)
                    {
                        draw_list->AddRectFilled(
                            ImVec2(p.x + tx * tile_size, p.y + ty * tile_size),
                            ImVec2(p.x + (tx + 1) * tile_size, p.y + (ty + 1) * tile_size),
                            ImColor(0.05f, 0.05f, 0.05f, 1.0f));
                        continue;
                    }

                    u8 flipX = (tileEntry & 0x4000) ? 0 : 7;
                    u8 flipY = (tileEntry & 0x8000) ? 7 : 0;
                    u8 pal = (tileEntry >> 10) & 3;
                    int tileIdx = tileEntry & 0xFF;

                    for (int y = 0; y < 8; y++)
                    {
                        int base = tileIdx * 32 + (y ^ flipY) * 2;

                        for (int x = 0; x < 8; x++)
                        {
                            u8 bit = 1 << (x ^ flipX);

                            u8 color = ((border->tiles[base + 0 ] & bit) ? 1 : 0) |
                                       ((border->tiles[base + 1 ] & bit) ? 2 : 0) |
                                       ((border->tiles[base + 16] & bit) ? 4 : 0) |
                                       ((border->tiles[base + 17] & bit) ? 8 : 0);

                            ImVec4 c;
                            if (color == 0)
                                c = gameColor0;
                            else
                                c = borderColors[color + pal * 16];

                            float px = p.x + tx * tile_size + x * scale;
                            float py = p.y + ty * tile_size + y * scale;
                            draw_list->AddRectFilled(
                                ImVec2(px, py),
                                ImVec2(px + scale, py + scale),
                                ImColor(c));
                        }
                    }
                }
            }

            if (show_grid)
            {
                for (int n = 0; n <= 32; n++)
                    draw_list->AddLine(ImVec2(p.x + n * tile_size, p.y), ImVec2(p.x + n * tile_size, p.y + height), ImColor(dark_gray), 1.0f);
                for (int n = 0; n <= 28; n++)
                    draw_list->AddLine(ImVec2(p.x, p.y + n * tile_size), ImVec2(p.x + width, p.y + n * tile_size), ImColor(dark_gray), 1.0f);

                draw_list->AddRect(
                    ImVec2(p.x + 6 * tile_size, p.y + 5 * tile_size),
                    ImVec2(p.x + 26 * tile_size, p.y + 23 * tile_size),
                    ImColor(green), 0.0f, 0, 2.0f);
            }

            float mouse_x = io.MousePos.x - p.x;
            float mouse_y = io.MousePos.y - p.y;
            int hovered_tile_x = -1;
            int hovered_tile_y = -1;

            if (window_hovered && mouse_x >= 0.0f && mouse_x < width && mouse_y >= 0.0f && mouse_y < height)
            {
                hovered_tile_x = (int)(mouse_x / tile_size);
                hovered_tile_y = (int)(mouse_y / tile_size);
                if (hovered_tile_x > 31) hovered_tile_x = 31;
                if (hovered_tile_y > 27) hovered_tile_y = 27;

                if (ImGui::IsMouseClicked(0))
                {
                    if (selected_tile_x == hovered_tile_x && selected_tile_y == hovered_tile_y)
                    {
                        selected_tile_x = -1;
                        selected_tile_y = -1;
                    }
                    else
                    {
                        selected_tile_x = hovered_tile_x;
                        selected_tile_y = hovered_tile_y;
                    }

                    u16 clicked_entry = border->map[hovered_tile_x + hovered_tile_y * 32];
                    int clicked_tile_idx = clicked_entry & 0xFF;
                    gui_debug_memory_goto(MEMORY_EDITOR_SGB_BORDER_TILES, clicked_tile_idx * 32);
                }

                if (!(hovered_tile_x == selected_tile_x && hovered_tile_y == selected_tile_y))
                    draw_list->AddRect(
                        ImVec2(p.x + hovered_tile_x * tile_size, p.y + hovered_tile_y * tile_size),
                        ImVec2(p.x + (hovered_tile_x + 1) * tile_size, p.y + (hovered_tile_y + 1) * tile_size),
                        ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            }

            if (selected_tile_x >= 0 && selected_tile_y >= 0)
            {
                float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
                ImVec4 pulse_color = ImVec4(red.x + (white.x - red.x) * t, red.y + (white.y - red.y) * t, red.z + (white.z - red.z) * t, 1.0f);
                draw_list->AddRect(
                    ImVec2(p.x + selected_tile_x * tile_size, p.y + selected_tile_y * tile_size),
                    ImVec2(p.x + (selected_tile_x + 1) * tile_size, p.y + (selected_tile_y + 1) * tile_size),
                    ImColor(pulse_color), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            }

            int detail_x = (hovered_tile_x >= 0) ? hovered_tile_x : selected_tile_x;
            int detail_y = (hovered_tile_y >= 0) ? hovered_tile_y : selected_tile_y;

            if (detail_x >= 0 && detail_y >= 0)
            {
                ImGui::NextColumn();

                u16 entry = border->map[detail_x + detail_y * 32];
                int tileIdx = entry & 0xFF;
                u8 pal = (entry >> 10) & 3;
                u8 flipX = (entry & 0x4000) ? 0 : 7;
                u8 flipY = (entry & 0x8000) ? 7 : 0;

                float zoom = 16.0f;
                ImVec2 tp = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("##tile_preview", ImVec2(8 * zoom, 8 * zoom));

                if (!(entry & 0x300))
                {
                    for (int y = 0; y < 8; y++)
                    {
                        int base = tileIdx * 32 + (y ^ flipY) * 2;
                        for (int x = 0; x < 8; x++)
                        {
                            u8 bit = 1 << (x ^ flipX);
                            u8 color = ((border->tiles[base + 0 ] & bit) ? 1 : 0) |
                                       ((border->tiles[base + 1 ] & bit) ? 2 : 0) |
                                       ((border->tiles[base + 16] & bit) ? 4 : 0) |
                                       ((border->tiles[base + 17] & bit) ? 8 : 0);
                            ImVec4 c = (color == 0) ? gameColor0 : borderColors[color + pal * 16];
                            draw_list->AddRectFilled(
                                ImVec2(tp.x + x * zoom, tp.y + y * zoom),
                                ImVec2(tp.x + (x + 1) * zoom, tp.y + (y + 1) * zoom),
                                ImColor(c));
                        }
                    }
                }

                ImGui::TextColored(cyan, " X:"); ImGui::SameLine();
                ImGui::Text("%d", detail_x); ImGui::SameLine();
                ImGui::TextColored(cyan, "   Y:"); ImGui::SameLine();
                ImGui::Text("%d", detail_y);

                ImGui::TextColored(cyan, " Map Entry:"); ImGui::SameLine();
                ImGui::Text("$%04X", entry);

                ImGui::TextColored(cyan, " Index:"); ImGui::SameLine();
                ImGui::Text("%d ($%02X)", tileIdx, tileIdx);

                ImGui::TextColored(cyan, " Palette:"); ImGui::SameLine();
                ImGui::Text("%d", pal);

                ImGui::TextColored(cyan, " X-Flip:"); ImGui::SameLine();
                (entry & 0x4000) ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

                ImGui::TextColored(cyan, " Y-Flip:"); ImGui::SameLine();
                (entry & 0x8000) ? ImGui::TextColored(green, "ON") : ImGui::TextColored(gray, "OFF");

                bool unused = (entry & 0x300) != 0;
                ImGui::TextColored(cyan, " Unused:"); ImGui::SameLine();
                unused ? ImGui::TextColored(yellow, "YES") : ImGui::TextColored(gray, "NO");
            }

            ImGui::Columns(1);

            ImGui::PopFont();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Border Tiles"))
        {
            const SGB::Border* border = sgb->GetBorder();

            static bool show_tile_grid = true;
            static int selected_bt_x = -1;
            static int selected_bt_y = -1;

            if (ImGui::IsWindowAppearing())
            {
                selected_bt_x = -1;
                selected_bt_y = -1;
            }

            static int tile_pal = 0;
            ImGui::PushItemWidth(60.0f);
            ImGui::Combo("Palette##bt_pal", &tile_pal, "0\0001\0002\0003\0");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Checkbox("Show Grid##grid_bt", &show_tile_grid);

            ImGui::PushFont(gui_default_font);

            bool bt_hovered = ImGui::IsWindowHovered();
            float bt_scale = 2.0f;
            float bt_spacing = 8.0f * bt_scale;
            float bt_width = 16.0f * bt_spacing;
            float bt_height = 16.0f * bt_spacing;

            ImVec4 btColors[16 * 4];
            for (int i = 0; i < 16 * 4; i++)
                btColors[i] = sgb_color_to_float(border->palette[i]);
            ImVec4 btGameColor0 = sgb_color_to_float(sgb->GetEffectivePalettes()[0]);

            ImGui::Columns(2, "sgb_bt", false);
            ImGui::SetColumnOffset(1, bt_width + 10.0f);

            ImVec2 bt_p = ImGui::GetCursorScreenPos();
            ImDrawList* bt_dl = ImGui::GetWindowDrawList();
            ImGuiIO& bt_io = ImGui::GetIO();

            ImGui::InvisibleButton("##bt_canvas", ImVec2(bt_width, bt_height));

            ImGui::PopFont();

            if (ImGui::BeginPopupContextItem("##sgb_tiles_ctx"))
            {
                if (ImGui::Selectable("Save Tiles As..."))
                    gui_file_dialog_save_sgb_tiles(tile_pal);
                ImGui::EndPopup();
            }

            ImGui::PushFont(gui_default_font);

            for (int ty = 0; ty < 16; ty++)
            {
                for (int tx = 0; tx < 16; tx++)
                {
                    int tileIdx = ty * 16 + tx;
                    for (int y = 0; y < 8; y++)
                    {
                        int base = tileIdx * 32 + y * 2;
                        for (int x = 0; x < 8; x++)
                        {
                            u8 bit = 1 << (7 - x);
                            u8 color = ((border->tiles[base + 0 ] & bit) ? 1 : 0) |
                                       ((border->tiles[base + 1 ] & bit) ? 2 : 0) |
                                       ((border->tiles[base + 16] & bit) ? 4 : 0) |
                                       ((border->tiles[base + 17] & bit) ? 8 : 0);
                            ImVec4 c = (color == 0) ? btGameColor0 : btColors[color + tile_pal * 16];
                            float px = bt_p.x + tx * bt_spacing + x * bt_scale;
                            float py = bt_p.y + ty * bt_spacing + y * bt_scale;
                            bt_dl->AddRectFilled(ImVec2(px, py), ImVec2(px + bt_scale, py + bt_scale), ImColor(c));
                        }
                    }
                }
            }

            if (show_tile_grid)
            {
                for (int n = 0; n <= 16; n++)
                {
                    bt_dl->AddLine(ImVec2(bt_p.x + n * bt_spacing, bt_p.y), ImVec2(bt_p.x + n * bt_spacing, bt_p.y + bt_height), ImColor(dark_gray), 1.0f);
                    bt_dl->AddLine(ImVec2(bt_p.x, bt_p.y + n * bt_spacing), ImVec2(bt_p.x + bt_width, bt_p.y + n * bt_spacing), ImColor(dark_gray), 1.0f);
                }
            }

            float bt_mx = bt_io.MousePos.x - bt_p.x;
            float bt_my = bt_io.MousePos.y - bt_p.y;
            int hov_bt_x = -1, hov_bt_y = -1;

            if (bt_hovered && bt_mx >= 0 && bt_mx < bt_width && bt_my >= 0 && bt_my < bt_height)
            {
                hov_bt_x = (int)(bt_mx / bt_spacing);
                hov_bt_y = (int)(bt_my / bt_spacing);
                if (hov_bt_x > 15) hov_bt_x = 15;
                if (hov_bt_y > 15) hov_bt_y = 15;

                if (ImGui::IsMouseClicked(0))
                {
                    if (selected_bt_x == hov_bt_x && selected_bt_y == hov_bt_y)
                    {
                        selected_bt_x = -1;
                        selected_bt_y = -1;
                    }
                    else
                    {
                        selected_bt_x = hov_bt_x;
                        selected_bt_y = hov_bt_y;
                    }

                    int clicked_idx = hov_bt_y * 16 + hov_bt_x;
                    gui_debug_memory_goto(MEMORY_EDITOR_SGB_BORDER_TILES, clicked_idx * 32);
                }

                if (!(hov_bt_x == selected_bt_x && hov_bt_y == selected_bt_y))
                    bt_dl->AddRect(ImVec2(bt_p.x + hov_bt_x * bt_spacing, bt_p.y + hov_bt_y * bt_spacing),
                        ImVec2(bt_p.x + (hov_bt_x + 1) * bt_spacing, bt_p.y + (hov_bt_y + 1) * bt_spacing),
                        ImColor(cyan), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            }

            if (selected_bt_x >= 0 && selected_bt_y >= 0)
            {
                float t = (float)(0.5 + 0.5 * sin(ImGui::GetTime() * 4.0));
                ImVec4 pc = ImVec4(red.x + (white.x - red.x) * t, red.y + (white.y - red.y) * t, red.z + (white.z - red.z) * t, 1.0f);
                bt_dl->AddRect(ImVec2(bt_p.x + selected_bt_x * bt_spacing, bt_p.y + selected_bt_y * bt_spacing),
                    ImVec2(bt_p.x + (selected_bt_x + 1) * bt_spacing, bt_p.y + (selected_bt_y + 1) * bt_spacing),
                    ImColor(pc), 2.0f, ImDrawFlags_RoundCornersAll, 2.0f);
            }

            int dt_x = (hov_bt_x >= 0) ? hov_bt_x : selected_bt_x;
            int dt_y = (hov_bt_y >= 0) ? hov_bt_y : selected_bt_y;

            if (dt_x >= 0 && dt_y >= 0)
            {
                ImGui::NextColumn();

                int tileIdx = dt_y * 16 + dt_x;

                float zoom = 16.0f;
                ImVec2 tp = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton("##bt_detail", ImVec2(8 * zoom, 8 * zoom));

                for (int y = 0; y < 8; y++)
                {
                    int base = tileIdx * 32 + y * 2;
                    for (int x = 0; x < 8; x++)
                    {
                        u8 bit = 1 << (7 - x);
                        u8 color = ((border->tiles[base + 0 ] & bit) ? 1 : 0) |
                                   ((border->tiles[base + 1 ] & bit) ? 2 : 0) |
                                   ((border->tiles[base + 16] & bit) ? 4 : 0) |
                                   ((border->tiles[base + 17] & bit) ? 8 : 0);
                        ImVec4 c = (color == 0) ? btGameColor0 : btColors[color + tile_pal * 16];
                        bt_dl->AddRectFilled(ImVec2(tp.x + x * zoom, tp.y + y * zoom),
                            ImVec2(tp.x + (x + 1) * zoom, tp.y + (y + 1) * zoom), ImColor(c));
                    }
                }

                ImGui::TextColored(cyan, " Index:"); ImGui::SameLine();
                ImGui::Text("%d ($%02X)", tileIdx, tileIdx);

                ImGui::TextColored(cyan, " Data Offset:"); ImGui::SameLine();
                ImGui::Text("$%04X", tileIdx * 32);

                ImGui::TextColored(cyan, " Palette:"); ImGui::SameLine();
                ImGui::Text("%d", tile_pal);
            }

            ImGui::Columns(1);

            ImGui::PopFont();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Attribute Map"))
        {
            ImGui::PushFont(gui_default_font);

            const u8* attrMap = sgb->GetAttributeMap();

            ImGui::NewLine();

            float cell = 20.0f;
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            static const ImU32 pal_colors[4] = {
                IM_COL32(60, 60, 180, 255),
                IM_COL32(60, 180, 60, 255),
                IM_COL32(180, 60, 60, 255),
                IM_COL32(180, 180, 60, 255),
            };

            ImGui::InvisibleButton("##attr_canvas", ImVec2(20 * cell, 18 * cell));

            for (int y = 0; y < 18; y++)
            {
                for (int x = 0; x < 20; x++)
                {
                    u8 pal = attrMap[x + y * 20] & 3;
                    ImVec2 tl(p.x + x * cell, p.y + y * cell);
                    ImVec2 br(tl.x + cell, tl.y + cell);
                    draw_list->AddRectFilled(tl, br, pal_colors[pal]);
                    draw_list->AddRect(tl, br, ImColor(dark_gray));

                    char label[4];
                    snprintf(label, sizeof(label), "%d", pal);
                    draw_list->AddText(ImVec2(tl.x + 6, tl.y + 3), ImColor(white), label);
                }
            }

            ImGui::PopFont();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Pending Border"))
        {
            ImGui::PushFont(gui_default_font);

            const SGB::Border* pending = sgb->GetPendingBorder();

            float scale = 2.0f;
            float tile_size = 8.0f * scale;
            float width = 32.0f * tile_size;
            float height = 28.0f * tile_size;

            ImVec2 p = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImGui::InvisibleButton("##pending_canvas", ImVec2(width, height));

            ImVec4 pendingColors[16 * 4];
            for (int i = 0; i < 16 * 4; i++)
            {
                u16 raw = pending->palette[i];
                pendingColors[i] = sgb_color_to_float(raw);
            }

            ImVec4 pendingGameColor0 = sgb_color_to_float(sgb->GetEffectivePalettes()[0]);

            for (int ty = 0; ty < 28; ty++)
            {
                for (int tx = 0; tx < 32; tx++)
                {
                    u16 tileEntry = pending->map[tx + ty * 32];

                    if (tileEntry & 0x300)
                    {
                        draw_list->AddRectFilled(
                            ImVec2(p.x + tx * tile_size, p.y + ty * tile_size),
                            ImVec2(p.x + (tx + 1) * tile_size, p.y + (ty + 1) * tile_size),
                            ImColor(0.05f, 0.05f, 0.05f, 1.0f));
                        continue;
                    }

                    u8 flipX = (tileEntry & 0x4000) ? 0 : 7;
                    u8 flipY = (tileEntry & 0x8000) ? 7 : 0;
                    u8 pal = (tileEntry >> 10) & 3;
                    int tileIdx = tileEntry & 0xFF;

                    for (int y = 0; y < 8; y++)
                    {
                        int base = tileIdx * 32 + (y ^ flipY) * 2;

                        for (int x = 0; x < 8; x++)
                        {
                            u8 bit = 1 << (x ^ flipX);

                            u8 color = ((pending->tiles[base + 0 ] & bit) ? 1 : 0) |
                                       ((pending->tiles[base + 1 ] & bit) ? 2 : 0) |
                                       ((pending->tiles[base + 16] & bit) ? 4 : 0) |
                                       ((pending->tiles[base + 17] & bit) ? 8 : 0);

                            ImVec4 c;
                            if (color == 0)
                                c = pendingGameColor0;
                            else
                                c = pendingColors[color + pal * 16];

                            float px = p.x + tx * tile_size + x * scale;
                            float py = p.y + ty * tile_size + y * scale;
                            draw_list->AddRectFilled(
                                ImVec2(px, py),
                                ImVec2(px + scale, py + scale),
                                ImColor(c));
                        }
                    }
                }
            }

            ImGui::PopFont();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void gui_debug_window_sgb_palettes(void)
{
    ImGui::SetNextWindowPos(ImVec2(140, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(210, 200), ImGuiCond_FirstUseEver);

    ImGui::Begin("SGB Effective Palettes", &config_debug.show_sgb_palettes);

    ImGui::PushFont(gui_default_font);

    SGB* sgb = emu_get_core()->GetSGB();
    const u16* effective = sgb->GetEffectivePalettes();

    for (int p = 0; p < 4; p++)
    {
        ImGui::TextColored(cyan, "%3d ", p); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            u16 raw = effective[p * 4 + c];
            ImVec4 fc = sgb_color_to_float(raw);
            char id[16];
            snprintf(id, sizeof(id), "##eff_%d_%d", p, c);
            ImGui::ColorEdit3(id, (float*)&fc, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 3)
            {
                ImGui::SameLine(); ImGui::Dummy(ImVec2(8.0f, 0.0f));
                ImGui::SameLine();
            }
        }

        ImGui::Text("   "); ImGui::SameLine();

        for (int c = 0; c < 4; c++)
        {
            ImGui::Text("%04X ", effective[p * 4 + c]);
            if (c < 3)
                ImGui::SameLine();
        }
    }

    ImGui::PopFont();

    ImGui::End();
}

void gui_debug_window_sgb_system_palettes(void)
{
    ImGui::SetNextWindowPos(ImVec2(160, 160), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(218, 286), ImGuiCond_FirstUseEver);

    ImGui::Begin("SGB System Palettes", &config_debug.show_sgb_system_palettes);

    ImGui::PushFont(gui_default_font);

    SGB* sgb = emu_get_core()->GetSGB();
    const u16* system = sgb->GetSystemPalettes();

    ImGuiListClipper clipper;
    clipper.Begin(SGB_SYSTEM_PALETTE_COUNT);

    while (clipper.Step())
    {
        for (int p = clipper.DisplayStart; p < clipper.DisplayEnd; p++)
        {
            ImGui::TextColored(cyan, "%3d ", p); ImGui::SameLine();

            for (int c = 0; c < 4; c++)
            {
                u16 raw = system[p * 4 + c];
                ImVec4 fc = sgb_color_to_float(raw);
                char id[20];
                snprintf(id, sizeof(id), "##sys_%d_%d", p, c);
                ImGui::ColorEdit3(id, (float*)&fc, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
                if (c < 3)
                {
                    ImGui::SameLine(); ImGui::Dummy(ImVec2(8.0f, 0.0f));
                    ImGui::SameLine();
                }
            }

            ImGui::Text("   "); ImGui::SameLine();

            for (int c = 0; c < 4; c++)
            {
                ImGui::Text("%04X ", system[p * 4 + c]);
                if (c < 3)
                    ImGui::SameLine();
            }
        }
    }

    ImGui::PopFont();

    ImGui::End();
}

void gui_debug_window_sgb_border_palettes(void)
{
    ImGui::SetNextWindowPos(ImVec2(180, 180), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(484, 130), ImGuiCond_FirstUseEver);

    ImGui::Begin("SGB Border Palettes", &config_debug.show_sgb_border_palettes);

    ImGui::PushFont(gui_default_font);

    SGB* sgb = emu_get_core()->GetSGB();
    const SGB::Border* border = sgb->GetBorder();

    for (int pal = 0; pal < 4; pal++)
    {
        ImGui::TextColored(cyan, "%3d ", pal); ImGui::SameLine();

        for (int c = 0; c < 16; c++)
        {
            u16 raw = border->palette[pal * 16 + c];
            ImVec4 fc = sgb_color_to_float(raw);
            char id[32];
            snprintf(id, sizeof(id), "##bpal_%d_%d", pal, c);
            ImGui::ColorEdit3(id, (float*)&fc, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
            if (c < 15)
                ImGui::SameLine();
        }
    }

    ImGui::PopFont();

    ImGui::End();
}

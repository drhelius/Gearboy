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

#include <string>
#include <stdexcept>
#include <algorithm>
#include <SDL.h>
#include "memory_editor.h"
#include "colors.h"

MemEditor::MemEditor()
{
    m_separator_column_width = 8.0f;
    m_selection_start = 0;
    m_selection_end = 0;
    m_bytes_per_row = 16;
    m_row_scroll_top = 0;
    m_row_scroll_bottom = 0;
    m_editing_address = -1;
    m_set_keyboard_here = false;
    m_uppercase_hex = true;
    m_gray_out_zeros = true;
    m_preview_data_type = 0;
    m_preview_endianess = 0;
    m_jump_to_address = -1;
    m_mem_data = NULL;
    m_mem_size = 0;
    m_mem_base_addr = 0;
    m_mem_word = 1;
    m_goto_address[0] = 0;
    m_add_bookmark = false;
    m_draw_list = 0;
}

MemEditor::~MemEditor()
{

}

void MemEditor::Draw(uint8_t* mem_data, int mem_size, int base_display_addr, int word, bool ascii, bool preview, bool options, bool cursors)
{
    m_mem_data = mem_data;
    m_mem_size = mem_size;
    m_mem_base_addr = base_display_addr;
    m_mem_word = word;
    if (m_mem_word > 2)
        m_mem_word = 2;

    if ((m_mem_word > 1) && ((m_preview_data_type < 2) || (m_preview_data_type > 3)))
        m_preview_data_type = 2;

    uint8_t hex_digits = 1;
    int size = m_mem_size - 1;

    while (size >>= 4)
    {
        hex_digits++;
    }

    snprintf(m_hex_mem_format, 8, "%%0%dX", hex_digits);

    ImVec4 addr_color = cyan;
    ImVec4 ascii_color = magenta;
    ImVec4 column_color = yellow;
    ImVec4 normal_color = white;
    ImVec4 highlight_color = orange;
    ImVec4 gray_color = mid_gray;

    int total_rows = (m_mem_size + (m_bytes_per_row - 1)) / m_bytes_per_row;
    int separator_count = (m_bytes_per_row - 1) / 4;
    int byte_column_count = 2 + m_bytes_per_row + separator_count + 2;
    int byte_cell_padding = 0;
    int ascii_padding = 4;
    int character_cell_padding = 0;
    int max_chars_per_cell = 2 * m_mem_word;
    ImVec2 character_size = ImGui::CalcTextSize("0");
    float footer_height = 0;

    if (options)
        footer_height += ImGui::GetFrameHeightWithSpacing();
    if (preview)
        footer_height += ((character_size.y + 4) * 3) + 4;
    if (cursors)
        footer_height += ImGui::GetFrameHeightWithSpacing();

    char buf[32];

    if (ImGui::BeginChild("##mem", ImVec2(ImGui::GetContentRegionAvail().x, -footer_height), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
    {
        m_draw_list = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.5, 0));

        if (ImGui::BeginTable("##header", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible))
        {
            char addr_spaces[32];
            int addr_padding = hex_digits - 2;
            snprintf(addr_spaces, 32, "ADDR %*s", addr_padding, "");
            ImGui::TableSetupColumn(addr_spaces);
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                snprintf(buf, 32, "%02X", i);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            if ((m_mem_word == 1) && ascii)
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x * ascii_padding);
                ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, (character_size.x + character_cell_padding * 1) * m_bytes_per_row);
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(addr_color, "%s", ImGui::TableGetColumnName(0));

            for (int i = 1; i < (ImGui::TableGetColumnCount() - 1); i++) {
                ImGui::TableNextColumn();
                ImGui::TextColored(column_color, "%s", ImGui::TableGetColumnName(i));
            }

            if ((m_mem_word == 1) && ascii)
            {
                ImGui::TableNextColumn();
                ImGui::TextColored(ascii_color, "%s", ImGui::TableGetColumnName(ImGui::TableGetColumnCount() - 1));
            }

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("##hex", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            m_row_scroll_top = (int)(ImGui::GetScrollY() / character_size.y);
            m_row_scroll_bottom = m_row_scroll_top + (int)(ImGui::GetWindowHeight() / character_size.y);

            ImGui::TableSetupColumn("ADDR");
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, character_size.x * ascii_padding);
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, (character_size.x + character_cell_padding * 1) * m_bytes_per_row);

            ImGuiListClipper clipper;
            clipper.Begin(total_rows);

            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    ImGui::TableNextRow();
                    int address = (row * m_bytes_per_row);

                    ImGui::TableNextColumn();
                    char single_addr[32];
                    snprintf(single_addr, 32, "%s:  ", m_hex_mem_format);                    
                    ImGui::Text(single_addr, address + m_mem_base_addr);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.75f, 0.0f));
                    for (int x = 0; x < m_bytes_per_row; x++)
                    {
                        int byte_address = address + x;

                        ImGui::TableNextColumn();
                        if (IsColumnSeparator(x, m_bytes_per_row))
                            ImGui::TableNextColumn();

                        ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                        ImVec2 cell_size = (character_size * ImVec2((float)max_chars_per_cell, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2((float)(1 + byte_cell_padding), 0);

                        ImVec2 hover_cell_size = cell_size;

                        if (IsColumnSeparator(x + 1, m_bytes_per_row))
                        {
                            hover_cell_size.x += m_separator_column_width + 1;
                        }

                        bool cell_hovered = ImGui::IsMouseHoveringRect(cell_start_pos, cell_start_pos + hover_cell_size, false) && ImGui::IsWindowHovered();

                        DrawSelectionBackground(x, byte_address, cell_start_pos, cell_size);

                        if (cell_hovered)
                        {
                            HandleSelection(byte_address, row);
                        }

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                        if (m_editing_address == byte_address)
                        {
                            ImGui::PushItemWidth((character_size).x * (2 * m_mem_word));

                            if (m_mem_word == 1)
                                snprintf(buf, 32, "%02X", m_mem_data[byte_address]);
                            else if (m_mem_word == 2)
                            {
                                uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                snprintf(buf, 32, "%04X", mem_data_16[byte_address]);
                            }

                            if (m_set_keyboard_here)
                            {
                                ImGui::SetKeyboardFocusHere();
                                m_set_keyboard_here = false;
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, yellow);
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, dark_cyan);

                            if (ImGui::InputText("##editing_input", buf, (m_mem_word == 1) ? 3 : 5, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AlwaysOverwrite))
                            {
                                try
                                {
                                    if (m_mem_word == 1)
                                        m_mem_data[byte_address] = (uint8_t)std::stoul(buf, 0, 16);
                                    else if (m_mem_word == 2)
                                    {
                                        uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                        mem_data_16[byte_address] = (uint16_t)std::stoul(buf, 0, 16);
                                    }

                                    if (byte_address < (m_mem_size - 1))
                                    {
                                        m_editing_address = byte_address + 1;
                                        m_selection_end = m_selection_start = m_editing_address;
                                        m_set_keyboard_here = true;
                                    }
                                    else
                                        m_editing_address = -1;
                                }
                                catch (const std::invalid_argument&)
                                {
                                    m_editing_address = -1;
                                }
                            }

                            ImGui::PopStyleColor();
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::PushItemWidth((character_size).x);

                            uint16_t data = 0;

                            if (m_mem_word == 1)
                                data = m_mem_data[byte_address];
                            else if (m_mem_word == 2)
                            {
                                uint16_t* mem_data_16 = (uint16_t*)m_mem_data;
                                data = mem_data_16[byte_address];
                            }

                            bool gray_out = m_gray_out_zeros && (data== 0);
                            bool highlight = (byte_address >= m_selection_start && byte_address < (m_selection_start + (DataPreviewSize() / m_mem_word)));

                            ImVec4 color = highlight ? highlight_color : (gray_out ? gray_color : normal_color);
                            if (m_mem_word == 1)
                                ImGui::TextColored(color, m_uppercase_hex ? "%02X" : "%02x", data);
                            else if (m_mem_word == 2)
                                ImGui::TextColored(color, m_uppercase_hex ? "%04X" : "%04x", data);

                            if (cell_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            {
                                m_editing_address = byte_address;
                                m_set_keyboard_here = true;
                            }


                            DrawContexMenu(byte_address, cell_hovered);
                        }

                        ImGui::PopItemWidth();
                        ImGui::PopStyleVar();

                        DrawSelectionFrame(x, row, byte_address, cell_start_pos, cell_size);
                    }

                    ImGui::PopStyleVar();

                    if ((m_mem_word == 1) && ascii)
                    {
                        ImGui::TableNextColumn();
                        float column_x = ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2.0f);
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        ImVec2 window_pos = ImGui::GetWindowPos();
                        draw_list->AddLine(ImVec2(window_pos.x + column_x, window_pos.y), ImVec2(window_pos.x + column_x, window_pos.y + 9999), ImGui::GetColorU32(dark_magenta));

                        ImGui::TableNextColumn();

                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                        if (ImGui::BeginTable("##ascii_column", m_bytes_per_row))
                        {
                            for (int x = 0; x < m_bytes_per_row; x++)
                            {
                                snprintf(buf, 32, "##ascii_cell%d", x);
                                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x + character_cell_padding * 1);
                            }

                            ImGui::TableNextRow();

                            for (int x = 0; x < m_bytes_per_row; x++)
                            {
                                ImGui::TableNextColumn();

                                int byte_address = address + x;
                                ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                                ImVec2 cell_size = (character_size * ImVec2(1, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2((float)(1 + byte_cell_padding), 0);

                                DrawSelectionAsciiBackground(byte_address, cell_start_pos, cell_size);

                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (character_cell_padding * 1) / 2);
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                                ImGui::PushItemWidth(character_size.x);

                                unsigned char c = m_mem_data[byte_address];

                                bool gray_out = m_gray_out_zeros && (c < 32 || c >= 128);
                                ImGui::TextColored(gray_out ? gray_color : normal_color, "%c", (c >= 32 && c < 128) ? c : '.');

                                ImGui::PopItemWidth();
                                ImGui::PopStyleVar();
                            }

                            ImGui::EndTable();
                        }
                        ImGui::PopStyleVar();
                    }
                }
            }

            if (m_jump_to_address >= 0 && m_jump_to_address < m_mem_size)
            {
                ImGui::SetScrollY((m_jump_to_address / m_bytes_per_row) * character_size.y);
                m_selection_start = m_selection_end = m_jump_to_address;
                m_jump_to_address = -1;
            }

            ImGui::EndTable();

        }

        ImGui::PopStyleVar();

    }
    ImGui::EndChild();

    if (cursors)
        DrawCursors();
    if (preview)
        DrawDataPreview(m_selection_start);
    if (options)
        DrawOptions();

    BookMarkPopup();
}

bool MemEditor::IsColumnSeparator(int current_column, int column_count)
{
    return (current_column > 0) && (current_column < column_count) && ((current_column % 4) == 0);
}

void MemEditor::DrawSelectionBackground(int x, int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    ImVec4 background_color = dark_cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    m_draw_list->AddRectFilled(cell_pos, cell_pos + cell_size + ImVec2(1, 0), ImColor(background_color));
}

void MemEditor::DrawSelectionAsciiBackground(int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec4 background_color = dark_cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;
    drawList->AddRectFilled(cell_pos, cell_pos + cell_size, ImColor(background_color));
}

void MemEditor::DrawSelectionFrame(int x, int y, int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    m_draw_list->Flags = ImDrawListFlags_None;
    ImVec4 frame_color = cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;
    bool multiline = (start / m_bytes_per_row) != (end / m_bytes_per_row);

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    if ((x == 0) || (address == start))
        m_draw_list->AddLine(cell_pos + ImVec2(-1, -1), cell_pos + ImVec2(-1, cell_size.y), ImColor(frame_color), 1);

    if ((x == (m_bytes_per_row - 1)) || (address == end))
        m_draw_list->AddLine(cell_pos + ImVec2(cell_size.x, multiline && (address == end) && (x != (m_bytes_per_row - 1)) ? 0 : -1), cell_pos + ImVec2(cell_size.x, cell_size.y), ImColor(frame_color), 1);

    if ((y == 0) || ((address - m_bytes_per_row) < start))
        m_draw_list->AddLine(cell_pos + ImVec2(-1, -1), cell_pos + ImVec2(cell_size.x, -1), ImColor(frame_color), 1);

    if ((address + m_bytes_per_row) > end)
        m_draw_list->AddLine(cell_pos + ImVec2(-1, cell_size.y), cell_pos + ImVec2(cell_size.x, cell_size.y), ImColor(frame_color), 1);

    if (multiline && (address == end) && (x != (m_bytes_per_row - 1)))
        m_draw_list->AddLine(cell_pos + ImVec2(cell_size.x, 0), cell_pos + ImVec2(cell_size.x + cell_size.x, 0), ImColor(frame_color), 1);
}

void MemEditor::HandleSelection(int address, int row)
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        m_selection_end = address;

        if (m_selection_start != m_selection_end)
        {
            if (row > (m_row_scroll_bottom - 3))
            {
                ImGui::SetScrollY(ImGui::GetScrollY() + 5);
            }
            else if (row < (m_row_scroll_top + 4))
            {
                ImGui::SetScrollY(ImGui::GetScrollY() - 5);
            }
        }
    }
    else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))// || ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        m_selection_start = address;
        m_selection_end = address;
    }
    else if (m_selection_start > m_selection_end)
    {
        int tmp = m_selection_start;
        m_selection_start = m_selection_end;
        m_selection_end = tmp;
    }

    if (m_editing_address != m_selection_start)
    {
        m_editing_address = -1;
    }
}

void MemEditor::DrawCursors()
{
    ImGui::PushItemWidth(55);
    if (ImGui::InputTextWithHint("##gotoaddr", "XXXXXX", m_goto_address, IM_ARRAYSIZE(m_goto_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
    {
        try
        {
            JumpToAddress((int)std::stoul(m_goto_address, 0, 16));
            m_goto_address[0] = 0;
        }
        catch(const std::invalid_argument&)
        {
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Go!", ImVec2(30, 0)))
    {
        try
        {
            JumpToAddress((int)std::stoul(m_goto_address, 0, 16));
            m_goto_address[0] = 0;
        }
        catch(const std::invalid_argument&)
        {
        }
    }

    ImGui::SameLine();

    char range_addr[32];
    char region_text[32];
    char single_addr[32];
    char selection_text[32];
    char all_text[128];
    snprintf(range_addr, 32, "%s-%s", m_hex_mem_format, m_hex_mem_format);
    snprintf(region_text, 32, range_addr, m_mem_base_addr, m_mem_base_addr + m_mem_size - 1);
    snprintf(single_addr, 32, "%s", m_hex_mem_format);
    if (m_selection_start == m_selection_end)
        snprintf(selection_text, 32, single_addr, m_mem_base_addr + m_selection_start);
    else
        snprintf(selection_text, 32, range_addr, m_mem_base_addr + m_selection_start, m_mem_base_addr + m_selection_end);
    snprintf(all_text, 128, "REGION: %s SELECTION: %s", region_text, selection_text);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(all_text).x 
    - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);

    ImVec4 color = ImVec4(0.1f,0.9f,0.9f,1.0f);

    ImGui::TextColored(color, "REGION:");
    ImGui::SameLine();
    ImGui::Text("%s", region_text);
    ImGui::SameLine();
    ImGui::TextColored(color, " SELECTION:");
    ImGui::SameLine();
    ImGui::Text("%s", selection_text);
}

void MemEditor::DrawOptions()
{
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("context");

    if (ImGui::BeginPopup("context"))
    {
        ImGui::Text("Columns:   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::SliderInt("##columns", &m_bytes_per_row, 4, 32);
        ImGui::Text("Preview as:");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::Combo("##preview_type", &m_preview_data_type, "Uint8\0Int8\0Uint16\0Int16\0UInt32\0Int32\0\0");
        ImGui::Text("Preview as:");
        ImGui::SameLine();
        ImGui::PushItemWidth(120.0f);
        ImGui::Combo("##preview_endianess", &m_preview_endianess, "Little Endian\0Big Endian\0\0");
        ImGui::Checkbox("Uppercase hex", &m_uppercase_hex);
        ImGui::Checkbox("Gray out zeros", &m_gray_out_zeros);

        ImGui::EndPopup();
    }
}

void MemEditor::DrawDataPreview(int address)
{
    ImGui::Separator();

    if (address < 0 || address >= m_mem_size)
        return;

    int data = 0;
    int data_size = DataPreviewSize();
    int final_address = address * m_mem_word;

    for (int i = 0; i < data_size; i++)
    {
        if (m_preview_endianess == 0)
            data |= m_mem_data[final_address + i] << (i * 8);
        else
            data |= m_mem_data[final_address + data_size - i - 1] << (i * 8);
    }

    ImVec4 color = orange;

    ImGui::TextColored(color, "Dec:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsDec(data);
    else
        ImGui::Text(" ");

    ImGui::TextColored(color, "Hex:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsHex(data);
    else
        ImGui::Text(" ");

    ImGui::TextColored(color, "Bin:");
    ImGui::SameLine();
    if (final_address + data_size <= (m_mem_size * m_mem_word))
        DrawDataPreviewAsBin(data);
    else
        ImGui::Text(" ");
}

void MemEditor::DrawDataPreviewAsHex(int data)
{
    int data_size = DataPreviewSize();
    const char* format = ((data_size == 1) ? "%02X" : (data_size == 2 ? "%04X" : "%08X"));

    ImGui::Text(format, data);
}

void MemEditor::DrawDataPreviewAsDec(int data)
{
    switch (m_preview_data_type)
    {
        case 0:
        {
            ImGui::Text("%u (Uint8)", (uint8_t)data);
            break;
        }
        case 1:
        {
            ImGui::Text("%d (Int8)", (int8_t)data);
            break;
        }
        case 2:
        {
            ImGui::Text("%u (Uint16)", (uint16_t)data);
            break;
        }
        case 3:
        {
            ImGui::Text("%d (Int16)", (int16_t)data);
            break;
        }
        case 4:
        {
            ImGui::Text("%u (Uint32)", (uint32_t)data);
            break;
        }
        case 5:
        {
            ImGui::Text("%d (Int32)", (int32_t)data);
            break;
        }
    }
}
void MemEditor::DrawDataPreviewAsBin(int data)
{
    int data_size = DataPreviewSize();

    std::string bin = "";
    for (int i = 0; i < data_size * 8; i++)
    {
        if ((i % 4) == 0 && i > 0)
            bin = " " + bin;
        bin = ((data >> i) & 1 ? "1" : "0") + bin;
    }

    ImGui::Text("%s", bin.c_str());
}

int MemEditor::DataPreviewSize()
{
    switch (m_preview_data_type)
    {
        case 0:
        case 1:
            return 1;
        case 2:
        case 3:
            return 2;
        case 4:
        case 5:
            return 4;
        default:
            return 1;
    }
}

void MemEditor::DrawContexMenu(int address, bool cell_hovered)
{
    char id[16];
    snprintf(id, 16, "##context_%d", address);

    if (cell_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup(id);

    if (ImGui::BeginPopup(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
    {
        if (m_gui_font != NULL)
            ImGui::PushFont(m_gui_font);

        if ((address < m_selection_start) || (address > m_selection_end))
            m_selection_start = m_selection_end = address;

        if (ImGui::Selectable("Copy"))
        {
            Copy();
        }

        if (ImGui::Selectable("Paste"))
        {
            Paste();
        }

        if (ImGui::Selectable("Select All"))
        {
            SelectAll();
        }

        if (ImGui::Selectable("Add Bookmark..."))
        {
            m_add_bookmark = true;
        }

        if (m_gui_font != NULL)
            ImGui::PopFont();

        ImGui::EndPopup();
    }
}

void MemEditor::BookMarkPopup()
{
    if (m_add_bookmark)
    {
        ImGui::OpenPopup("Add Bookmark");
        m_add_bookmark = false;
    }

    if (m_gui_font != NULL)
        ImGui::PushFont(m_gui_font);

    if (ImGui::BeginPopupModal("Add Bookmark", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char address[9] = "";
        static char name[32] = "";
        int bookmark_address = m_selection_start;

        if (bookmark_address > 0)
            snprintf(address, 9, "%06X", bookmark_address);

        ImGui::Text("Name:");
        ImGui::PushItemWidth(200);ImGui::SetItemDefaultFocus();
        ImGui::InputText("##name", name, IM_ARRAYSIZE(name));

        ImGui::Text("Address:");
        ImGui::PushItemWidth(70);
        ImGui::InputTextWithHint("##bookaddr", "XXXXXX", address, 7, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(90, 0)))
        {
            try
            {
                bookmark_address = (int)std::stoul(address, 0, 16);

                if (strlen(name) == 0)
                {
                    snprintf(name, 32, "Bookmark_%06X", bookmark_address);
                }

                Bookmark bookmark;
                bookmark.address = bookmark_address;
                snprintf(bookmark.name, 32, "%s", name);
                m_bookmarks.push_back(bookmark);
                ImGui::CloseCurrentPopup();

                address[0] = 0;
                name[0] = 0;
            }
            catch(const std::invalid_argument&)
            {
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(90, 0))) { ImGui::CloseCurrentPopup(); }

        ImGui::EndPopup();
    }

    if (m_gui_font != NULL)
            ImGui::PopFont();
}

void MemEditor::Copy()
{
    int size = (m_selection_end - m_selection_start + 1) * m_mem_word;
    uint8_t* data = m_mem_data + (m_selection_start * m_mem_word);

    std::string text;

    for (int i = 0; i < size; i++)
    {
        char byte[4];
        snprintf(byte, 4, "%02X", data[i]);
        if (i > 0)
            text += " ";
        text += byte;
    }

    SDL_SetClipboardText(text.c_str());
}

void MemEditor::Paste()
{
    char* clipboard = SDL_GetClipboardText();

    if (clipboard != NULL)
    {
        std::string text(clipboard);

        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
        text.erase(std::remove(text.begin(), text.end(), ' '), text.end());

        int buffer_size = (int)text.size() / 2;

        uint8_t* data = new uint8_t[buffer_size];

        for (int i = 0; i < buffer_size; i ++)
        {
            std::string byte = text.substr(i * 2, 2);

            try
            {
                data[i] = (uint8_t)std::stoul(byte, 0, 16);
            }
            catch(const std::invalid_argument&)
            {
                delete[] data;
                SDL_free(clipboard);
                return;
            }
        }

        int selection_size = (m_selection_end - m_selection_start + 1) * m_mem_word;
        int start = m_selection_start * m_mem_word;
        int end = start + std::min(buffer_size, selection_size);

        for (int i = start; i < end; i++)
        {
            m_mem_data[i] = data[i - start];
        }

        delete[] data;
    }

    SDL_free(clipboard);
}

void MemEditor::JumpToAddress(int address)
{
    if (address >= m_mem_base_addr && address < (m_mem_base_addr + m_mem_size))
        m_jump_to_address = address - m_mem_base_addr;
}

void MemEditor::SelectAll()
{
    m_selection_start = 0;
    m_selection_end = m_mem_size - 1;
}

void MemEditor::ClearSelection()
{
    m_selection_start = m_selection_end = 0;
}

void MemEditor::SetValueToSelection(int value)
{
    int selection_size = (m_selection_end - m_selection_start + 1) * m_mem_word;
    int start = m_selection_start * m_mem_word;
    int end = start + selection_size;
    int mask = m_mem_word == 1 ? 0xFF : 0xFFFF;

    for (int i = start; i < end; i++)
    {
        m_mem_data[i] = value & mask;
    }
}

void MemEditor::SaveToFile(const char* file_path)
{
    int size = m_mem_size * m_mem_word;
    int row = m_bytes_per_row * m_mem_word;

    FILE* file = fopen(file_path, "w");

    if (file)
    {
        for (int i = 0; i < (size - 1); i++)
        {
            fprintf(file, "%02X ", m_mem_data[i]);

            if ((i % row) == (row - 1))
                fprintf(file, "\n");
        }

        fprintf(file, "%02X", m_mem_data[(size - 1)]);

        fclose(file);
    }
}

void MemEditor::AddBookmark()
{
    m_add_bookmark = true;
}

void MemEditor::RemoveBookmarks()
{
    m_bookmarks.clear();
}

std::vector<MemEditor::Bookmark>* MemEditor::GetBookmarks()
{
    return &m_bookmarks;
}

void MemEditor::SetGuiFont(ImFont* gui_font)
{
    m_gui_font = gui_font;
}
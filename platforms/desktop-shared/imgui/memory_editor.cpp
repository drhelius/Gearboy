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

#include "memory_editor.h"
#include <string>
#include <stdexcept>
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
}

MemEditor::~MemEditor()
{

}

void MemEditor::Draw(uint8_t* mem_data, int mem_size, int base_display_addr)
{
    m_mem_data = mem_data;
    m_mem_size = mem_size;
    m_mem_base_addr = base_display_addr;

    ImVec4 addr_color = cyan;
    ImVec4 ascii_color = magenta;
    ImVec4 column_color = yellow;
    ImVec4 normal_color = white;
    ImVec4 highlight_color = orange;
    ImVec4 gray_color = mid_gray;

    int total_rows = (mem_size + (m_bytes_per_row - 1)) / m_bytes_per_row;
    int separator_count = (m_bytes_per_row - 1) / 4;
    int byte_column_count = 2 + m_bytes_per_row + separator_count + 2;
    int byte_cell_padding = 0;
    int character_cell_padding = 0;
    int max_chars_per_cell = 2;
    ImVec2 character_size = ImGui::CalcTextSize("0");
    float footer_height = (ImGui::GetFrameHeightWithSpacing() * 4) + 4;
    char buf[32];

    if (ImGui::BeginChild("##mem", ImVec2(ImGui::GetContentRegionAvail().x, -footer_height), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.5, 0));

        if (ImGui::BeginTable("##header", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible))
        {
            ImGui::TableSetupColumn("ADDR   ");
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                snprintf(buf, sizeof(buf), "%02X", i);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            ImGui::TableSetupColumn("");
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, (character_size.x + character_cell_padding * 1) * m_bytes_per_row);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextColored(addr_color, "%s", ImGui::TableGetColumnName(0));

            for (int i = 1; i < (ImGui::TableGetColumnCount() - 1); i++) {
                ImGui::TableNextColumn();
                ImGui::TextColored(column_color, "%s", ImGui::TableGetColumnName(i));
            }

            ImGui::TableNextColumn();
            ImGui::TextColored(ascii_color, "%s", ImGui::TableGetColumnName(ImGui::TableGetColumnCount() - 1));

            ImGui::EndTable();
        }

        if (ImGui::BeginTable("##hex", byte_column_count, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoKeepColumnsVisible | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
        {
            m_row_scroll_top = ImGui::GetScrollY() / character_size.y;
            m_row_scroll_bottom = m_row_scroll_top + (ImGui::GetWindowHeight() / character_size.y);

            ImGui::TableSetupColumn("ADDR");
            ImGui::TableSetupColumn("");

            for (int i = 0; i < m_bytes_per_row; i++) {
                if (IsColumnSeparator(i, m_bytes_per_row))
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_separator_column_width);

                snprintf(buf, sizeof(buf), "%02X", i);

                ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x * max_chars_per_cell + (6 + byte_cell_padding) * 1);
            }

            ImGui::TableSetupColumn("  ");
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
                    ImGui::Text("%04X:  ", address + base_display_addr);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2.75f, 0.0f));
                    for (int x = 0; x < m_bytes_per_row; x++)
                    {
                        int byte_address = address + x;

                        ImGui::TableNextColumn();
                        if (IsColumnSeparator(x, m_bytes_per_row))
                            ImGui::TableNextColumn();

                        ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                        ImVec2 cell_size = (character_size * ImVec2(max_chars_per_cell, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2(1 + byte_cell_padding, 0);
                        bool cell_hovered = ImGui::IsMouseHoveringRect(cell_start_pos, cell_start_pos + cell_size, false) && ImGui::IsWindowHovered();

                        DrawSelectionBackground(x, byte_address, cell_start_pos, cell_size);

                        if (cell_hovered)
                        {
                            HandleSelection(byte_address, row);
                        }

                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                        if (m_editing_address == byte_address)
                        {
                            ImGui::PushItemWidth((character_size).x *2);
                            snprintf(buf, sizeof(buf), "%02X", mem_data[byte_address]);

                            if (m_set_keyboard_here)
                            {
                                ImGui::SetKeyboardFocusHere();
                                m_set_keyboard_here = false;
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, red);
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, dark_cyan);

                            if (ImGui::InputText("##editing_input", buf, 3, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AlwaysOverwrite))
                            {
                                try
                                {
                                    mem_data[byte_address] = (uint8_t)std::stoul(buf, 0, 16);

                                    if (byte_address < (mem_size - 1))
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

                            bool gray_out = m_gray_out_zeros && mem_data[byte_address] == 0;
                            bool highlight = (byte_address >= m_selection_start && byte_address < (m_selection_start + DataPreviewSize()));

                            ImVec4 color = highlight ? highlight_color : (gray_out ? gray_color : normal_color);
                            ImGui::TextColored(color, m_uppercase_hex ? "%02X" : "%02x", mem_data[byte_address]);

                            if (cell_hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            {
                                m_editing_address = byte_address;
                                m_set_keyboard_here = true;
                            }
                        }

                        ImGui::PopItemWidth();
                        ImGui::PopStyleVar();

                        DrawSelectionFrame(x, row, byte_address, cell_start_pos, cell_size);
                    }

                    ImGui::PopStyleVar();

                    ImGui::TableNextColumn();
                    ImGui::TableNextColumn();

                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                    if (ImGui::BeginTable("##ascii_column", m_bytes_per_row))
                    {
                        for (int x = 0; x < m_bytes_per_row; x++)
                        {
                            snprintf(buf, sizeof(buf), "##ascii_cell%d", x);
                            ImGui::TableSetupColumn(buf, ImGuiTableColumnFlags_WidthFixed, character_size.x + character_cell_padding * 1);
                        }

                        ImGui::TableNextRow();

                        for (int x = 0; x < m_bytes_per_row; x++)
                        {
                            ImGui::TableNextColumn();

                            int byte_address = address + x;
                            ImVec2 cell_start_pos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().CellPadding;
                            ImVec2 cell_size = (character_size * ImVec2(1, 1)) + (ImVec2(2, 2) * ImGui::GetStyle().CellPadding) + ImVec2(1 + byte_cell_padding, 0);

                            DrawSelectionAsciiBackground(byte_address, cell_start_pos, cell_size);

                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (character_cell_padding * 1) / 2);
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                            ImGui::PushItemWidth(character_size.x);

                            unsigned char c = mem_data[byte_address];

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

            if (m_jump_to_address >= 0 && m_jump_to_address < mem_size)
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

    DrawCursors();
    DrawDataPreview(m_selection_start);
    DrawOptions();
}

bool MemEditor::IsColumnSeparator(int current_column, int column_count)
{
    return (current_column > 0) && (current_column < column_count) && ((current_column % 4) == 0);
}

void MemEditor::DrawSelectionBackground(int x, int address, ImVec2 cell_pos, ImVec2 cell_size)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec4 background_color = dark_cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    drawList->AddRectFilled(cell_pos + ImVec2(x == 0 ? 1 : 0, 0), cell_pos + cell_size, ImColor(background_color));
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
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec4 frame_color = cyan;
    int start = m_selection_start <= m_selection_end ? m_selection_start : m_selection_end;
    int end = m_selection_end >= m_selection_start ? m_selection_end : m_selection_start;

    if (address < start || address > end)
        return;

    if (IsColumnSeparator(x + 1, m_bytes_per_row) && (address != end))
    {
        cell_size.x += m_separator_column_width + 1;
    }

    if (x == 0 || address == start)
        drawList->AddLine(cell_pos, cell_pos + ImVec2(0, cell_size.y), ImColor(frame_color), 1);

    if (x == (m_bytes_per_row - 1) || (address) == end)
        drawList->AddLine(cell_pos + ImVec2(cell_size.x, 0), cell_pos + cell_size - ImVec2(0, 0), ImColor(frame_color), 1);

    if (y == 0 || (address - m_bytes_per_row) < start)
        drawList->AddLine(cell_pos - ImVec2(1, 0), cell_pos + ImVec2(cell_size.x + 1, 0), ImColor(frame_color), 1);

    if ((address + m_bytes_per_row) >= end)
        drawList->AddLine(cell_pos + ImVec2(0, cell_size.y), cell_pos + cell_size + ImVec2(1, 0), ImColor(frame_color), 1);
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
    else if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right))
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
    ImVec4 color = ImVec4(0.1f,0.9f,0.9f,1.0f);

    ImGui::TextColored(color, "REGION:");
    ImGui::SameLine();
    ImGui::Text("%04X-%04X", m_mem_base_addr, m_mem_base_addr + m_mem_size);
    ImGui::SameLine();
    ImGui::TextColored(color, " SELECTION:");
    ImGui::SameLine();
    if (m_selection_start == m_selection_end)
        ImGui::Text("%04X", m_mem_base_addr + m_selection_start);
    else
        ImGui::Text("%04X-%04X", m_mem_base_addr + m_selection_start, m_mem_base_addr + m_selection_end);
    ImGui::Separator();
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

    ImGui::SameLine();
    ImGui::Text("Go to:");
    ImGui::SameLine();
    ImGui::PushItemWidth(45);
    char goto_address[5] = "";
    if (ImGui::InputTextWithHint("##gotoaddr", "XXXX", goto_address, IM_ARRAYSIZE(goto_address), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
    {
        try
        {
            JumpToAddress((int)std::stoul(goto_address, 0, 16));
        }
        catch(const std::invalid_argument&)
        {
        }
        goto_address[0] = 0;
    }
}

void MemEditor::DrawDataPreview(int address)
{
    if (address < 0 || address >= m_mem_size)
        return;

    ImVec4 color = orange;

    ImGui::TextColored(color, "Dec:");
    ImGui::SameLine();
    DrawDataPreviewAsDec(address);

    ImGui::TextColored(color, "Hex:");
    ImGui::SameLine();
    DrawDataPreviewAsHex(address);

    ImGui::TextColored(color, "Bin:");
    ImGui::SameLine();
    DrawDataPreviewAsBin(address);
}

void MemEditor::DrawDataPreviewAsHex(int address)
{
    int data_size = DataPreviewSize();
    if (address + data_size > m_mem_size)
    {
        ImGui::Text(" ");
        return;
    }

    int data = 0;

    for (int i = 0; i < data_size; i++)
    {
        if (m_preview_endianess == 0)
            data |= m_mem_data[address + i] << (i * 8);
        else
            data |= m_mem_data[address + data_size - i - 1] << (i * 8);
    }

    const char* format = ((data_size == 1) ? "%02X" : (data_size == 2 ? "%04X" : "%08X"));

    ImGui::Text(format, data);
}

void MemEditor::DrawDataPreviewAsDec(int address)
{
    int data_size = DataPreviewSize();
    if (address + data_size > m_mem_size)
    {
        ImGui::Text(" ");
        return;
    }

    int data = 0;

    for (int i = 0; i < data_size; i++)
    {
        if (m_preview_endianess == 0)
            data |= m_mem_data[address + i] << (i * 8);
        else
            data |= m_mem_data[address + data_size - i - 1] << (i * 8);
    }

    switch (m_preview_data_type)
    {
        case 0:
        {
            ImGui::Text("%u", (uint8_t)data);
            break;
        }
        case 1:
        {
            ImGui::Text("%d", (int8_t)data);
            break;
        }
        case 2:
        {
            ImGui::Text("%u", (uint16_t)data);
            break;
        }
        case 3:
        {
            ImGui::Text("%d", (int16_t)data);
            break;
        }
        case 4:
        {
            ImGui::Text("%u", (uint32_t)data);
            break;
        }
        case 5:
        {
            ImGui::Text("%d", (int32_t)data);
            break;
        }
    }
}
void MemEditor::DrawDataPreviewAsBin(int address)
{
    int data_size = DataPreviewSize();
    if (address + data_size > m_mem_size)
    {
        ImGui::Text(" ");
        return;
    };

    int data = 0;

    for (int i = 0; i < data_size; i++)
    {
        if (m_preview_endianess == 0)
            data |= m_mem_data[address + i] << (i * 8);
        else
            data |= m_mem_data[address + data_size - i - 1] << (i * 8);
    }

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

void MemEditor::Copy(uint8_t** data, int* size)
{
    int start = m_selection_start;
    int end = m_selection_end;

    *size = end - start + 1;
    *data = m_mem_data + start;
}

void MemEditor::Paste(uint8_t* data, int size)
{
    int selection = m_selection_end - m_selection_start + 1;
    int start = m_selection_start;
    int end = m_selection_start + (size < selection ? size : selection);

    for (int i = start; i < end; i++)
    {
        m_mem_data[i] = data[i - start];
    }
}

void MemEditor::JumpToAddress(int address)
{
    m_jump_to_address = address - m_mem_base_addr;
}

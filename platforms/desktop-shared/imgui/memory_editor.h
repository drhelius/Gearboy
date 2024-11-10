/*
 * Gearcoleco - ColecoVision Emulator
 * Copyright (C) 2021  Ignacio Sanchez

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

#ifndef MEM_EDITOR_H
#define	MEM_EDITOR_H

#include <stdint.h>
#include <stdio.h>
#include "imgui.h"

class MemEditor
{
public:
    MemEditor();
    ~MemEditor();

    void Draw(uint8_t* mem_data, int mem_size, int base_display_addr = 0x0000);
    void Copy(uint8_t** data, int* size);
    void Paste(uint8_t* data, int size);
    void JumpToAddress(int address);

private:
    bool IsColumnSeparator(int current_column, int column_count);
    void DrawSelectionBackground(int x, int address, ImVec2 cellPos, ImVec2 cellSize);
    void DrawSelectionAsciiBackground(int address, ImVec2 cellPos, ImVec2 cellSize);
    void DrawSelectionFrame(int x, int y, int address, ImVec2 cellPos, ImVec2 cellSize);
    void HandleSelection(int address, int row);
    void DrawCursors();
    void DrawOptions();
    void DrawDataPreview(int address);
    void DrawDataPreviewAsHex(int address);
    void DrawDataPreviewAsDec(int address);
    void DrawDataPreviewAsBin(int address);
    int DataPreviewSize();

private:
    float m_separator_column_width;
    int m_selection_start;
    int m_selection_end;
    int m_bytes_per_row;
    int m_row_scroll_top;
    int m_row_scroll_bottom;
    int m_editing_address;
    bool m_set_keyboard_here;
    bool m_uppercase_hex;
    bool m_gray_out_zeros;
    int m_preview_data_type;
    int m_preview_endianess;
    int m_jump_to_address;
    uint8_t* m_mem_data;
    int m_mem_size;
    int m_mem_base_addr;
};

#endif	/* MEM_EDITOR_H */
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

#ifndef MEM_EDITOR_H
#define	MEM_EDITOR_H

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "imgui.h"

class MemEditor
{
public:
    struct Bookmark
    {
        int address;
        char name[32];
    };

public:
    MemEditor();
    ~MemEditor();

    void Draw(uint8_t* mem_data, int mem_size, int base_display_addr = 0x0000, int word = 1, bool ascii = true, bool preview = true, bool options = true, bool cursors = true);
    void Copy();
    void Paste();
    void JumpToAddress(int address);
    void SelectAll();
    void ClearSelection();
    void SetValueToSelection(int value);
    void SaveToFile(const char* file_path);
    void AddBookmark();
    void RemoveBookmarks();
    std::vector<Bookmark>* GetBookmarks();
    void SetGuiFont(ImFont* gui_font);

private:
    bool IsColumnSeparator(int current_column, int column_count);
    void DrawSelectionBackground(int x, int address, ImVec2 cellPos, ImVec2 cellSize);
    void DrawSelectionAsciiBackground(int address, ImVec2 cellPos, ImVec2 cellSize);
    void DrawSelectionFrame(int x, int y, int address, ImVec2 cellPos, ImVec2 cellSize);
    void HandleSelection(int address, int row);
    void DrawCursors();
    void DrawOptions();
    void DrawDataPreview(int address);
    void DrawDataPreviewAsHex(int data);
    void DrawDataPreviewAsDec(int data);
    void DrawDataPreviewAsBin(int data);
    int DataPreviewSize();
    void DrawContexMenu(int address, bool cell_hovered);
    void BookMarkPopup();

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
    char m_hex_mem_format[8];
    int m_mem_word;
    char m_goto_address[7];
    bool m_add_bookmark;
    std::vector<Bookmark> m_bookmarks;
    ImFont* m_gui_font;
    ImDrawList* m_draw_list;
};

#endif	/* MEM_EDITOR_H */
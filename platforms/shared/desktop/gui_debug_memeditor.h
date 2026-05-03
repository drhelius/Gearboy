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

#ifndef GUI_DEBUG_MEMEDITOR_H
#define GUI_DEBUG_MEMEDITOR_H

#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "imgui.h"

class MemEditor
{
public:
    struct Bookmark
    {
        int address;
        char name[32];
    };

    struct Watch
    {
        int address;
        char notes[128];
        int size;
        int format;
    };

    struct Search
    {
        int address;
        int value;
        int prev_value;
    };

    struct Options
    {
        int bytes_per_row = 16;
        int preview_data_type = 0;
        int preview_endianess = 0;
        bool uppercase_hex = true;
        bool gray_out_zeros = true;
    };

public:
    MemEditor();
    ~MemEditor();

    void Reset(const char* title, uint8_t* mem_data, int mem_size, int base_display_addr = 0x0000, int word = 1);
    void Draw(bool ascii = true, bool preview = true, bool options = true, bool cursors = true);
    void DrawWatchWindow();
    void DrawSearchWindow();
    void Copy(bool as_decimal = false);
    void Paste();
    void JumpToAddress(int address);
    void FindNextValue(int value);
    void SelectAll();
    void ClearSelection();
    void SetValueToSelection(int value);
    void SaveToTextFile(const char* file_path);
    void SaveToBinaryFile(const char* file_path);
    void AddBookmark();
    void RemoveBookmarks();
    std::vector<Bookmark>* GetBookmarks();
    void OpenWatchWindow();
    void OpenSearchWindow();
    void OpenFindBytes();
    void AddWatch();
    void PrepareAddWatch(int address, const char* notes);
    bool AddWatchDirect(int address, const char* notes, int size);
    void RemoveWatches();
    std::vector<Watch>* GetWatches();
    void SetGuiFont(ImFont* gui_font);
    void BookMarkPopup();
    void WatchPopup();
    void DrawFindBytesWindow();
    void SaveSettings(std::ostream& stream);
    void LoadSettings(std::istream& stream);
    Options GetOptions() const;
    void SetOptions(const Options& options);
    void StepFrame();
    int GetWordBytes();
    char* GetTitle();
    void GetSelection(int* start, int* end);
    bool SetSelection(int start, int end);
    void ScrollToAddress(int address);
    void SearchCapture();
    int PerformSearch(int op, int compare_type, int compare_value, int data_type);
    std::vector<Search>* GetSearchResults();
    int FindBytesSequence(const char* hex_str, int* out_addresses, int max_results);

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
    void DrawContexMenu(int address, bool cell_hovered, bool options);
    void WatchWindow();
    void SearchWindow();
    void FindBytesWindow();
    void CalculateSearchResults();
    void CalculateFindBytesResults();
    void DrawSearchValue(int value, ImVec4 color);
    void FindBytesNext(int start_offset);
    bool ParseHexByteString(const char* str, uint8_t* out, int* out_len, int max_len);
    bool NormalizeSelectionAddress(int address, int* offset);
    bool CanWatchRangeFit(int address, int size);
    uint32_t ReadWatchValue(const Watch& watch);
    int WatchSizeBytes(int size);
    void DrawWatchValue(uint32_t value, int size, int format);
    void PushGuiFont();
    void PopGuiFont();

private:
    char m_title[32];
    float m_separator_column_width;
    int m_selection_start;
    int m_selection_end;
    Options m_options;
    int m_row_scroll_top;
    int m_row_scroll_bottom;
    int m_editing_address;
    bool m_set_keyboard_here;
    int m_jump_to_address;
    int m_scroll_to_address;
    uint8_t* m_mem_data;
    int m_mem_size;
    int m_mem_base_addr;
    char m_hex_addr_format[16];
    int m_hex_addr_digits;
    int m_mem_word;
    char m_goto_address[7];
    char m_find_next[5];
    bool m_add_bookmark;
    std::vector<Bookmark> m_bookmarks;
    bool m_watch_window;
    bool m_add_watch;
    int m_pending_watch_address;
    char m_pending_watch_notes[128];
    std::vector<Watch> m_watches;
    ImFont* m_gui_font;
    ImDrawList* m_draw_list;
    bool m_search_window;
    int m_search_operator;
    int m_search_compare_type;
    int m_search_data_type;
    char m_search_compare_specific_value_str[5];
    int m_search_compare_specific_value;
    char m_search_compare_specific_address_str[7];
    int m_search_compare_specific_address;
    uint8_t* m_search_data;
    std::vector<Search> m_search_results;
    bool m_search_auto;
    bool m_find_bytes_window;
    char m_find_bytes_buffer[1025];
    int m_find_bytes_last_address;
    int m_find_bytes_pattern_len;
    std::vector<int> m_find_bytes_results;
};

#endif /* GUI_DEBUG_MEMEDITOR_H */
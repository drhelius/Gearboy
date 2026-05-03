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

#ifndef GUI_DEBUG_MEMORY_H
#define GUI_DEBUG_MEMORY_H

#include <iostream>
#include "gearboy.h"

#ifdef GUI_DEBUG_MEMORY_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum Memory_Editor_Tabs
{
    MEMORY_EDITOR_ROM0 = 0,
    MEMORY_EDITOR_ROM1,
    MEMORY_EDITOR_VRAM,
    MEMORY_EDITOR_RAM,
    MEMORY_EDITOR_WRAM0,
    MEMORY_EDITOR_WRAM1,
    MEMORY_EDITOR_WRAM,
    MEMORY_EDITOR_OAM,
    MEMORY_EDITOR_IO,
    MEMORY_EDITOR_HIRAM,
    MEMORY_EDITOR_SGB_BORDER_TILES,
    MEMORY_EDITOR_SGB_BORDER_MAP,
    MEMORY_EDITOR_SGB_BORDER_PAL,
    MEMORY_EDITOR_SGB_SYS_PAL,
    MEMORY_EDITOR_SGB_ATTR_FILES,
    MEMORY_EDITOR_SGB_ATTR_MAP,
    MEMORY_EDITOR_SGB_EFF_PAL,
    MEMORY_EDITOR_MAX
};

EXTERN void gui_debug_memory_init(void);
EXTERN void gui_debug_memory_destroy(void);
EXTERN void gui_debug_memory_reset(void);
EXTERN void gui_debug_window_memory(void);
EXTERN void gui_debug_memory_search_window(void);
EXTERN void gui_debug_memory_find_bytes_window(void);
EXTERN void gui_debug_memory_watches_window(void);
EXTERN void gui_debug_memory_step_frame(void);
EXTERN void gui_debug_memory_copy(void);
EXTERN void gui_debug_memory_paste(void);
EXTERN void gui_debug_memory_select_all(void);
EXTERN void gui_debug_memory_goto(int editor, int address);
EXTERN void gui_debug_memory_save_dump(const char* file_path, bool binary);
EXTERN bool gui_debug_memory_select_range(int editor, int start_address, int end_address);
EXTERN void gui_debug_memory_set_selection_value(int editor, u8 value);
EXTERN void gui_debug_memory_add_bookmark(int editor, int address, const char* name);
EXTERN void gui_debug_memory_remove_bookmark(int editor, int address);
EXTERN bool gui_debug_memory_add_watch(int editor, int address, const char* notes, int size);
EXTERN void gui_debug_memory_open_watch_popup(int editor, int address, const char* notes);
EXTERN void gui_debug_memory_remove_watch(int editor, int address);
EXTERN int gui_debug_memory_get_bookmarks(int editor, void** bookmarks_ptr);
EXTERN int gui_debug_memory_get_watches(int editor, void** watches_ptr);
EXTERN void gui_debug_memory_get_selection(int editor, int* start, int* end);
EXTERN void gui_debug_memory_search_capture(int editor);
EXTERN int gui_debug_memory_search(int editor, int op, int compare_type, int compare_value, int data_type, void** results_ptr);
EXTERN int gui_debug_memory_find_bytes(int editor, const char* hex_str, int* out_addresses, int max_results);
EXTERN void gui_debug_memory_save_settings(std::ostream& stream);
EXTERN void gui_debug_memory_load_settings(std::istream& stream);

#undef GUI_DEBUG_MEMORY_IMPORT
#undef EXTERN
#endif /* GUI_DEBUG_MEMORY_H */
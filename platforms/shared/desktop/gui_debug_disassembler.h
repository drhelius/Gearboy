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

#ifndef GUI_DEBUG_DISASSEMBLER_H
#define GUI_DEBUG_DISASSEMBLER_H

#include "gearboy.h"
#include <string>

#ifdef GUI_DEBUG_DISASSEMBLER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

struct DebugSymbol
{
    int bank;
    u16 address;
    char text[64];
};

EXTERN void gui_debug_disassembler_init(void);
EXTERN void gui_debug_disassembler_destroy(void);
EXTERN void gui_debug_disassembler_reset(void);
EXTERN void gui_debug_reset_symbols(void);
EXTERN void gui_debug_reset_breakpoints(void);
EXTERN bool gui_debug_load_symbols_file(const char* file_path);
EXTERN void gui_debug_toggle_breakpoint(void);
EXTERN void gui_debug_add_bookmark(void);
EXTERN void gui_debug_add_symbol(void);
EXTERN void gui_debug_add_symbol(const char* symbol_str);
EXTERN void gui_debug_remove_symbol(u8 bank, u16 address);
EXTERN void gui_debug_add_disassembler_bookmark(u16 address, const char* name);
EXTERN void gui_debug_remove_disassembler_bookmark(u16 address);
EXTERN int gui_debug_get_disassembler_bookmarks(void** bookmarks_ptr);
EXTERN void gui_debug_reset_disassembler_bookmarks(void);
EXTERN int gui_debug_get_symbols(void** symbols_ptr);
EXTERN bool gui_debug_resolve_symbol(GS_Disassembler_Record* record, std::string& instr, const char* color, const char* original_color, const char** out_name = NULL, u16* out_address = NULL);
EXTERN bool gui_debug_resolve_label(GS_Disassembler_Record* record, std::string& instr, const char* color, const char* original_color, const char** out_name = NULL, u16* out_address = NULL);
EXTERN void gui_debug_runtocursor(void);
EXTERN void gui_debug_runto_address(u16 address);
EXTERN void gui_debug_go_back(void);
EXTERN void gui_debug_window_disassembler(void);
EXTERN void gui_debug_save_disassembler(const char* file_path, bool full);
EXTERN void gui_debug_window_call_stack(void);
EXTERN void gui_debug_window_breakpoints(void);
EXTERN void gui_debug_window_symbols(void);

#undef GUI_DEBUG_DISASSEMBLER_IMPORT
#undef EXTERN
#endif /* GUI_DEBUG_DISASSEMBLER_H */
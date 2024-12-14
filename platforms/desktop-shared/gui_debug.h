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

#ifndef GUI_DEBUG_H
#define	GUI_DEBUG_H

#ifdef GUI_DEBUG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY_PATTERN_SPACED "%c%c%c%c %c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

EXTERN void gui_debug_windows(void);
EXTERN void gui_debug_reset(void);
EXTERN void gui_debug_reset_symbols(void);
EXTERN void gui_debug_load_symbols_file(const char* path);
EXTERN void gui_debug_toggle_breakpoint(void);
EXTERN void gui_debug_reset_breakpoints_cpu(void);
EXTERN void gui_debug_reset_breakpoints_mem(void);
EXTERN void gui_debug_runtocursor(void);
EXTERN void gui_debug_go_back(void);
EXTERN void gui_debug_copy_memory(void);
EXTERN void gui_debug_paste_memory(void);

#undef GUI_DEBUG_IMPORT
#undef EXTERN
#endif	/* GUI_DEBUG_H */
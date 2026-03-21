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

#ifndef GUI_DEBUG_YM2413_H
#define GUI_DEBUG_YM2413_H

#ifdef GUI_DEBUG_YM2413_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void gui_debug_ym2413_init(void);
EXTERN void gui_debug_ym2413_destroy(void);
EXTERN void gui_debug_window_ym2413(void);

#undef GUI_DEBUG_YM2413_IMPORT
#undef EXTERN
#endif /* GUI_DEBUG_YM2413_H */

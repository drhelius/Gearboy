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

#ifndef GUI_MENUS_H
#define GUI_MENUS_H

#ifdef GUI_MENUS_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN char gui_savefiles_path[4096];
EXTERN char gui_savestates_path[4096];
EXTERN char gui_screenshots_path[4096];
EXTERN char gui_dmg_bootrom_path[4096];
EXTERN char gui_gbc_bootrom_path[4096];

EXTERN void gui_init_menus(void);
EXTERN void gui_main_menu(void);

#undef GUI_MENUS_IMPORT
#undef EXTERN
#endif /* GUI_MENUS_H */
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

#ifndef GUI_ACTIONS_H
#define GUI_ACTIONS_H

#ifdef GUI_ACTIONS_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void gui_action_reset(void);
EXTERN void gui_action_reload_rom(void);
EXTERN void gui_action_pause(void);
EXTERN void gui_action_ffwd(void);
EXTERN void gui_action_rewind_pressed(void);
EXTERN void gui_action_rewind_released(void);
EXTERN void gui_action_save_screenshot(const char* path);
EXTERN void gui_action_save_sprite(const char* path, int index);
EXTERN void gui_action_save_all_sprites(const char* folder_path);
EXTERN void gui_action_save_background(const char* path);
EXTERN void gui_action_save_tiles(const char* path);
EXTERN void gui_action_save_sgb_border(const char* path);
EXTERN void gui_action_save_sgb_tiles(const char* path, int palette);

#undef GUI_ACTIONS_IMPORT
#undef EXTERN
#endif /* GUI_ACTIONS_H */
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL3/SDL.h>
#include "gearboy.h"

#ifdef APPLICATION_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN SDL_Window* application_sdl_window;
EXTERN int application_sdl_version_major;
EXTERN int application_sdl_version_minor;
EXTERN int application_sdl_version_patch;
EXTERN bool application_show_menu;

EXTERN int application_init(const char* rom_file, const char* symbol_file, bool force_fullscreen, bool force_windowed, int mcp_mode, int mcp_tcp_port);
EXTERN void application_destroy(void);
EXTERN void application_mainloop(void);
EXTERN void application_trigger_quit(void);
EXTERN void application_trigger_fullscreen(bool fullscreen);
EXTERN void application_trigger_fit_to_content(int width, int height);
EXTERN void application_refocus_window(void);
EXTERN void application_update_title_with_rom(const char* rom);
EXTERN void application_input_pump(void);
EXTERN bool application_check_single_instance(const char* rom_file, const char* symbol_file);

#undef APPLICATION_IMPORT
#undef EXTERN
#endif /* APPLICATION_H */

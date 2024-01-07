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
#define	APPLICATION_H

#include <SDL.h>

#ifdef APPLICATION_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN SDL_GameController* application_gamepad;
EXTERN int application_gamepad_mappings;
EXTERN float application_display_scale;
EXTERN SDL_version application_sdl_build_version;
EXTERN SDL_version application_sdl_link_version;

EXTERN int application_init(const char* rom_file, const char* symbol_file);
EXTERN void application_destroy(void);
EXTERN void application_mainloop(void);
EXTERN void application_trigger_quit(void);
EXTERN void application_trigger_fullscreen(bool fullscreen);
EXTERN void application_trigger_fit_to_content(int width, int height);

#undef APPLICATION_IMPORT
#undef EXTERN
#endif	/* APPLICATION_H */
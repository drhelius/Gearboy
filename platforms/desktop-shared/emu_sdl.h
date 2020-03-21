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

#ifndef EMU_SDL_H
#define	EMU_SDL_H

#include <SDL.h>

#ifdef EMU_SDL_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN SDL_Window* emu_sdl_window;
EXTERN SDL_GLContext emu_sdl_gl_context;

EXTERN int emu_sdl_init(void);
EXTERN void emu_sdl_destroy(void);
EXTERN void emu_sdl_mainloop(void);

#undef EMU_SDL_IMPORT
#undef EXTERN
#endif	/* EMU_SDL_H */
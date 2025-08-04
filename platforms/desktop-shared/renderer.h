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

#ifndef RENDERER_H
#define	RENDERER_H

#ifdef RENDERER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN uint32_t renderer_emu_texture;
EXTERN uint32_t renderer_emu_debug_vram_background;
EXTERN uint32_t renderer_emu_debug_vram_tiles[2];
EXTERN uint32_t renderer_emu_debug_vram_oam[40];
EXTERN const char* renderer_glew_version;
EXTERN const char* renderer_opengl_version;

EXTERN bool renderer_init(void);
EXTERN void renderer_destroy(void);
EXTERN void renderer_begin_render(void);
EXTERN void renderer_render(void);
EXTERN void renderer_end_render(void);

#undef RENDERER_IMPORT
#undef EXTERN
#endif	/* RENDERER_H */
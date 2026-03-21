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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL3/SDL.h>

#ifdef DISPLAY_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN SDL_GLContext display_gl_context;

EXTERN void display_begin_frame(void);
EXTERN void display_render(void);
EXTERN void display_frame_throttle(void);
EXTERN bool display_should_run_emu_frame(void);
EXTERN void display_set_vsync(bool enabled);
EXTERN void display_update_frame_pacing(void);
EXTERN void display_recreate_gl_context(void);
EXTERN void display_request_gl_context_recreate(void);
EXTERN void display_check_mixed_refresh_rates(void);
EXTERN bool display_is_vsync_forced_off(void);

#undef DISPLAY_IMPORT
#undef EXTERN
#endif /* DISPLAY_H */

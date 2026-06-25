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

#ifndef RUNAHEAD_H
#define RUNAHEAD_H

#include "gearboy.h"

#ifdef RUNAHEAD_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void runahead_init(void);
EXTERN void runahead_destroy(void);
EXTERN int runahead_get_frames(void);
EXTERN void runahead_run(int frames, u16* frame_buffer, s16* sample_buffer, int* sample_count);

#undef RUNAHEAD_IMPORT
#undef EXTERN
#endif /* RUNAHEAD_H */

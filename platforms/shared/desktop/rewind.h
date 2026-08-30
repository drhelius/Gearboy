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

#ifndef REWIND_H
#define REWIND_H

#include "gearboy.h"

#ifdef REWIND_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define REWIND_MAX_STATE_SIZE       (256 * 1024)
#define REWIND_MAX_SNAPSHOTS        600

EXTERN bool rewind_init(void);
EXTERN void rewind_destroy(void);
EXTERN void rewind_reset(void);
EXTERN void rewind_push(void);
EXTERN bool rewind_pop(void);
EXTERN bool rewind_seek(int age);
EXTERN void rewind_commit_seek(void);
EXTERN void rewind_set_active(bool a);
EXTERN bool rewind_is_active(void);
EXTERN int rewind_get_snapshot_count(void);
EXTERN int rewind_get_capacity(void);
EXTERN int rewind_get_frames_per_snapshot(void);
EXTERN size_t rewind_get_memory_usage(void);

#undef REWIND_IMPORT
#undef EXTERN
#endif /* REWIND_H */
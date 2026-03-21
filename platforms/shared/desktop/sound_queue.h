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

#ifndef SOUND_QUEUE_H
#define SOUND_QUEUE_H

#include <SDL3/SDL.h>
#include <stdint.h>
#include "gearboy.h"

#ifdef SOUND_QUEUE_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN void sound_queue_init(void);
EXTERN void sound_queue_destroy(void);
EXTERN bool sound_queue_start(int sample_rate, int channel_count, int buffer_size = GB_AUDIO_QUEUE_SIZE, int buffer_count = 3);
EXTERN void sound_queue_stop(void);
EXTERN void sound_queue_write(s16* samples, int count, bool sync);
EXTERN int sound_queue_get_sample_count(void);
EXTERN s16* sound_queue_get_currently_playing(void);
EXTERN bool sound_queue_is_open(void);

#undef SOUND_QUEUE_IMPORT
#undef EXTERN

#endif /* SOUND_QUEUE_H */
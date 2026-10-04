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

#ifndef VIDEO_RECORDER_H
#define VIDEO_RECORDER_H

#include "gearboy.h"

#ifdef VIDEO_RECORDER_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

enum Video_Recorder_Quality
{
    Video_Recorder_Quality_Low = 0,
    Video_Recorder_Quality_Medium,
    Video_Recorder_Quality_High,
    Video_Recorder_Quality_Lossless
};

EXTERN bool video_recorder_start(const char* file_path, int width, int height, double fps, int sample_rate, Video_Recorder_Quality quality);
EXTERN void video_recorder_stop(void);
EXTERN bool video_recorder_is_recording(void);
EXTERN const char* video_recorder_get_file_path(void);
EXTERN u32 video_recorder_get_frame_count(void);
EXTERN void video_recorder_add_audio(const s16* samples, int count);
EXTERN void video_recorder_add_video(const u8* frame_buffer, int width, int height, int bytes_per_pixel);

#undef VIDEO_RECORDER_IMPORT
#undef EXTERN
#endif /* VIDEO_RECORDER_H */

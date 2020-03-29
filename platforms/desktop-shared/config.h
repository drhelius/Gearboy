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

#ifndef CONFIG_H
#define	CONFIG_H

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "imgui/imgui.h"

#ifdef CONFIG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

struct config_EmulatorOptions
{
    bool paused = false;
    int save_slot = 0;
    bool start_paused = false;
    bool force_dmg = false;
    bool save_in_rom_folder = false;
    bool ffwd = false;
};

struct config_VideoOptions
{
    bool fps = false;
    bool bilinear = false;
    bool mix_frames = true;
    bool matrix = true;
    ImVec4 color[4];
};

struct config_AudioOptions
{
    bool enable = true;
    bool sync = true;
};

struct config_InputOptions
{
    bool gamepad = true;
};

EXTERN mINI::INIFile* config_ini_file;
EXTERN mINI::INIStructure config_ini_data;

EXTERN char* config_root_path;
EXTERN char config_emu_file_path[260];
EXTERN char config_imgui_file_path[260];

EXTERN config_EmulatorOptions config_emulator_options;
EXTERN config_VideoOptions config_video_options;
EXTERN config_AudioOptions config_audio_options;
EXTERN config_InputOptions config_input_options;

EXTERN void config_init(void);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);

#undef CONFIG_IMPORT
#undef EXTERN
#endif	/* CONFIG_H */
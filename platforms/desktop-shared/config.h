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

#include <SDL.h>
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
    SDL_Keycode key_left = SDLK_LEFT;
    SDL_Keycode key_right = SDLK_RIGHT;
    SDL_Keycode key_up = SDLK_UP;
    SDL_Keycode key_down = SDLK_DOWN;
    SDL_Keycode key_a = SDLK_s;
    SDL_Keycode key_b = SDLK_a;
    SDL_Keycode key_start = SDLK_RETURN;
    SDL_Keycode key_select = SDLK_SPACE;

    bool gamepad = true;
    bool gamepad_invert_x_axis = false;
    bool gamepad_invert_y_axis = false;
    int gamepad_a = 1;
    int gamepad_b = 2;
    int gamepad_start = 9;
    int gamepad_select = 8;
    int gamepad_x_axis = 0;
    int gamepad_y_axis = 1;
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
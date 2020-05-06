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
#include "../../src/gearboy.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "imgui/imgui.h"

#ifdef CONFIG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

static const int config_max_recent_roms = 10;

struct config_Emulator
{
    bool paused = false;
    int save_slot = 0;
    bool start_paused = false;
    bool force_dmg = false;
    bool save_in_rom_folder = false;
    bool ffwd = false;
    int ffwd_speed = 1;
    bool show_info = false;
    int mbc = 0;
    std::string recent_roms[config_max_recent_roms];
};

struct config_Video
{
    int scale = 0;
    int ratio = 0;
    bool fps = false;
    bool bilinear = false;
    bool mix_frames = true;
    float mix_frames_intensity = 0.50f;
    bool matrix = true;
    float matrix_intensity = 0.30f;
    int palette = 0;
    GB_Color color[4] = {{0xC4, 0xF0, 0xC2}, {0x5A, 0xB9, 0xA8}, {0x1E, 0x60, 0x6E}, {0x2D, 0x1B, 0x00}};
    bool sync = true;
    bool color_correction = true;
};

struct config_Audio
{
    bool enable = true;
    bool sync = true;
};

struct config_Input
{
    SDL_Scancode key_left = SDL_SCANCODE_LEFT;
    SDL_Scancode key_right = SDL_SCANCODE_RIGHT;
    SDL_Scancode key_up = SDL_SCANCODE_UP;
    SDL_Scancode key_down = SDL_SCANCODE_DOWN;
    SDL_Scancode key_a = SDL_SCANCODE_S;
    SDL_Scancode key_b = SDL_SCANCODE_A;
    SDL_Scancode key_start = SDL_SCANCODE_RETURN;
    SDL_Scancode key_select = SDL_SCANCODE_SPACE;

    bool gamepad = true;
    int gamepad_directional = 0;
    bool gamepad_invert_x_axis = false;
    bool gamepad_invert_y_axis = false;
    int gamepad_a = SDL_CONTROLLER_BUTTON_B;
    int gamepad_b = SDL_CONTROLLER_BUTTON_A;
    int gamepad_start = SDL_CONTROLLER_BUTTON_START;
    int gamepad_select = SDL_CONTROLLER_BUTTON_BACK;
    int gamepad_x_axis = SDL_CONTROLLER_AXIS_LEFTX;
    int gamepad_y_axis = SDL_CONTROLLER_AXIS_LEFTY;
};

struct config_Debug
{
    bool debug = false;
    bool show_gameboy = true;
    bool show_disassembler = true;
    bool show_processor = true;
    bool show_memory = false;
    bool show_iomap = false;
    bool show_audio = false;
    bool show_video = false;
};

EXTERN mINI::INIFile* config_ini_file;
EXTERN mINI::INIStructure config_ini_data;
EXTERN char* config_root_path;
EXTERN char config_emu_file_path[260];
EXTERN char config_imgui_file_path[260];
EXTERN config_Emulator config_emulator;
EXTERN config_Video config_video;
EXTERN config_Audio config_audio;
EXTERN config_Input config_input;
EXTERN config_Debug config_debug;

EXTERN void config_init(void);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);

#undef CONFIG_IMPORT
#undef EXTERN
#endif	/* CONFIG_H */
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
#define CONFIG_H

#include <SDL3/SDL.h>
#include "gearboy.h"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "imgui.h"

#ifdef CONFIG_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

static const int config_version = 2;
static const int config_max_recent_roms = 10;
static const int config_max_custom_palettes = 5;

struct config_Emulator
{
    bool maximized = false;
    bool fullscreen = false;
    int fullscreen_mode = 1;
    bool always_show_menu = false;
    bool paused = false;
    int save_slot = 0;
    bool start_paused = false;
    bool pause_when_inactive = true;
    bool force_dmg = false;
    bool force_gba = false;
    bool ffwd = false;
    int ffwd_speed = 1;
    bool show_info = false;
    int mbc = 0;
    std::string recent_roms[config_max_recent_roms];
    bool dmg_bootrom = false;
    std::string dmg_bootrom_path;
    bool gbc_bootrom = false;
    std::string gbc_bootrom_path;
    int savefiles_dir_option = 0;
    std::string savefiles_path;
    int savestates_dir_option = 0;
    std::string savestates_path;
    int screenshots_dir_option = 0;
    std::string screenshots_path;
    std::string last_open_path;
    int window_width = 800;
    int window_height = 700;
    bool status_messages = false;
};

struct config_Video
{
    int scale = 0;
    int scale_manual = 1;
    int ratio = 0;
    bool fps = false;
    bool bilinear = false;
    bool mix_frames = true;
    float mix_frames_intensity = 0.80f;
    bool matrix = true;
    float matrix_intensity = 0.05f;
    int palette = 0;
    GB_Color color[config_max_custom_palettes][4] = {
        {{0xC4, 0xF0, 0xC2}, {0x5A, 0xB9, 0xA8}, {0x1E, 0x60, 0x6E}, {0x2D, 0x1B, 0x00}},
        {{0xF8, 0xE3, 0xC4}, {0xCC, 0x34, 0x95}, {0x6B, 0x1F, 0xB1}, {0x0B, 0x06, 0x30}},
        {{0xEF, 0xF9, 0xD6}, {0xBA, 0x50, 0x44}, {0x7A, 0x1C, 0x4B}, {0x1B, 0x03, 0x26}},
        {{0xFF, 0xE4, 0xC2}, {0xDC, 0xA4, 0x56}, {0xA9, 0x60, 0x4C}, {0x42, 0x29, 0x36}},
        {{0xCE, 0xCE, 0xCE}, {0x6F, 0x9E, 0xDF}, {0x42, 0x67, 0x8E}, {0x10, 0x25, 0x33}}
    };
    bool sync = true;
    bool color_correction = true;
    float background_color[3] = {0.1f, 0.1f, 0.1f};
    float background_color_debugger[3] = {0.2f, 0.2f, 0.2f};
};

struct config_Audio
{
    bool enable = true;
    bool sync = true;
    int buffer_count = 3;
};

struct config_Input
{
    SDL_Scancode key_left;
    SDL_Scancode key_right;
    SDL_Scancode key_up;
    SDL_Scancode key_down;
    SDL_Scancode key_a;
    SDL_Scancode key_b;
    SDL_Scancode key_start;
    SDL_Scancode key_select;

    bool gamepad = true;
    int gamepad_directional = 0;
    bool gamepad_invert_x_axis = false;
    bool gamepad_invert_y_axis = false;
    int gamepad_a;
    int gamepad_b;
    int gamepad_start;
    int gamepad_select;
    int gamepad_x_axis;
    int gamepad_y_axis;
};

enum config_HotkeyIndex
{
    config_HotkeyIndex_OpenROM = 0,
    config_HotkeyIndex_ReloadROM,
    config_HotkeyIndex_Quit,
    config_HotkeyIndex_Reset,
    config_HotkeyIndex_Pause,
    config_HotkeyIndex_FFWD,
    config_HotkeyIndex_SaveState,
    config_HotkeyIndex_LoadState,
    config_HotkeyIndex_Screenshot,
    config_HotkeyIndex_Fullscreen,
    config_HotkeyIndex_ShowMainMenu,
    config_HotkeyIndex_DebugStepInto,
    config_HotkeyIndex_DebugStepOver,
    config_HotkeyIndex_DebugStepOut,
    config_HotkeyIndex_DebugStepFrame,
    config_HotkeyIndex_DebugContinue,
    config_HotkeyIndex_DebugBreak,
    config_HotkeyIndex_DebugRunToCursor,
    config_HotkeyIndex_DebugBreakpoint,
    config_HotkeyIndex_DebugGoBack,
    config_HotkeyIndex_SelectSlot1,
    config_HotkeyIndex_SelectSlot2,
    config_HotkeyIndex_SelectSlot3,
    config_HotkeyIndex_SelectSlot4,
    config_HotkeyIndex_SelectSlot5,
    config_HotkeyIndex_COUNT
};

struct config_Input_Gamepad_Shortcuts
{
    int gamepad_shortcuts[config_HotkeyIndex_COUNT];
};

struct config_Hotkey
{
    SDL_Scancode key;
    SDL_Keymod mod;
    char str[64];
};

struct config_Debug
{
    bool debug = false;
    bool show_screen = true;
    bool show_disassembler = true;
    bool show_processor = true;
    bool show_call_stack = false;
    bool show_breakpoints = false;
    bool show_symbols = false;
    bool show_memory = false;
    bool show_video = false;
    bool show_video_nametable = false;
    bool show_video_tiles = false;
    bool show_video_sprites = false;
    bool show_video_palettes = false;
    bool show_video_gbc_palettes = false;
    bool show_io = false;
    bool show_psg = false;
    bool show_trace_logger = false;
    bool trace_counter = true;
    bool trace_bank = true;
    bool trace_registers = true;
    bool trace_flags = true;
    bool trace_bytes = true;
    bool dis_show_mem = true;
    bool dis_show_symbols = true;
    bool dis_show_segment = true;
    bool dis_show_bank = true;
    bool dis_show_auto_symbols = true;
    bool dis_dim_auto_symbols = false;
    bool dis_replace_symbols = true;
    bool dis_replace_labels = true;
    int dis_look_ahead_count = 20;
    int font_size = 0;
    int scale = 1;
    bool multi_viewport = false;
    bool single_instance = false;
    bool auto_debug_settings = false;
};

EXTERN mINI::INIFile* config_ini_file;
EXTERN mINI::INIStructure config_ini_data;
EXTERN const char* config_root_path;
EXTERN char config_emu_file_path[512];
EXTERN char config_imgui_file_path[512];
EXTERN config_Emulator config_emulator;
EXTERN config_Video config_video;
EXTERN config_Audio config_audio;
EXTERN config_Input config_input;
EXTERN config_Input_Gamepad_Shortcuts config_input_gamepad_shortcuts;
EXTERN config_Hotkey config_hotkeys[config_HotkeyIndex_COUNT];
EXTERN config_Debug config_debug;

EXTERN void config_init(void);
EXTERN void config_destroy(void);
EXTERN void config_read(void);
EXTERN void config_write(void);
EXTERN void config_load_defaults(void);
EXTERN void config_update_hotkey_string(config_Hotkey* hotkey);

#undef CONFIG_IMPORT
#undef EXTERN
#endif /* CONFIG_H */

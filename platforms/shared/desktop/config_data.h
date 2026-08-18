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

#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <SDL3/SDL.h>
#include <string>
#include "gearboy.h"

static const char* config_application_name = GEARBOY_TITLE;
static const int config_version = 5;
static const int config_minimum_version = 2;
static const int config_max_recent_roms = 10;
static const int config_max_custom_palettes = 5;
static const int config_memory_editor_count = 17;

enum config_ShaderMode
{
    config_ShaderMode_PixelPerfect = 0,
    config_ShaderMode_External = 1
};

enum config_Theme
{
    config_Theme_Light = 0,
    config_Theme_Dark = 1,
    config_Theme_Count = 2
};

enum config_VideoSync
{
    config_VideoSync_Disabled = 0,
    config_VideoSync_Fixed = 1,
    config_VideoSync_VRR = 2
};

struct config_Emulator
{
    bool maximized;
    bool fullscreen;
    int fullscreen_mode;
    bool always_show_menu;
    int theme;
    bool paused;
    int save_slot;
    bool start_paused;
    bool pause_when_inactive;
    bool force_dmg;
    bool force_gba;
    bool sgb;
    bool sgb_border;
    bool ffwd;
    int ffwd_speed;
    int runahead;
    bool show_info;
    int mbc;
    std::string recent_roms[config_max_recent_roms];
    bool dmg_bootrom;
    std::string dmg_bootrom_path;
    bool gbc_bootrom;
    std::string gbc_bootrom_path;
    int savefiles_dir_option;
    std::string savefiles_path;
    int savestates_dir_option;
    std::string savestates_path;
    int screenshots_dir_option;
    std::string screenshots_path;
    std::string last_open_path;
    int window_width;
    int window_height;
    bool status_messages;
    bool allow_screensaver;
    int mcp_tcp_port;
    std::string mcp_http_address;
    int tilt_source;
    int mouse_sensitivity_x;
    int mouse_sensitivity_y;
    bool mouse_invert_x;
    bool mouse_invert_y;
    int sensor_sensitivity_x;
    int sensor_sensitivity_y;
    bool sensor_invert_x;
    bool sensor_invert_y;
    int analog_sensitivity_x;
    int analog_sensitivity_y;
    bool analog_invert_x;
    bool analog_invert_y;
    bool capture_mouse;
};

struct config_Video
{
    int scale;
    int scale_manual;
    int ratio;
    bool fps;
    int palette;
    GB_Color color[config_max_custom_palettes][4];
    int sync_mode;
    bool color_correction;
    float background_color[config_Theme_Count][3];
    float background_color_debugger[config_Theme_Count][3];
    int shader_mode;
    std::string shader_preset_path;
};

struct config_Audio
{
    bool enable;
    bool sync;
    float master_volume;
    int buffer_count;
};

struct config_Rewind
{
    bool enabled;
    int buffer_seconds;
    int frames_per_snapshot;
    float speed;
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

    bool allow_up_down;
    bool gamepad;
    int gamepad_directional;
    bool gamepad_invert_x_axis;
    bool gamepad_invert_y_axis;
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
    config_HotkeyIndex_Rewind,
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
    config_HotkeyIndex_CaptureMouse,
    config_HotkeyIndex_SelectSlot1,
    config_HotkeyIndex_SelectSlot2,
    config_HotkeyIndex_SelectSlot3,
    config_HotkeyIndex_SelectSlot4,
    config_HotkeyIndex_SelectSlot5,
    config_HotkeyIndex_Mute,
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
    bool debug;
    bool show_screen;
    bool show_disassembler;
    bool show_processor;
    bool show_call_stack;
    bool show_breakpoints;
    bool show_symbols;
    bool show_memory;
    bool show_video;
    bool show_video_nametable;
    bool show_video_tiles;
    bool show_video_sprites;
    bool show_video_palettes;
    bool show_video_gbc_palettes;
    bool show_io;
    bool show_psg;
    bool show_trace_logger;
    bool show_rewind;
    bool show_sgb_state;
    bool show_sgb_video;
    bool show_sgb_palettes;
    bool show_sgb_system_palettes;
    bool show_sgb_border_palettes;
    bool trace_counter;
    bool trace_cycles;
    bool trace_bank;
    bool trace_registers;
    bool trace_flags;
    bool trace_bytes;
    bool trace_cpu_enabled;
    bool trace_cpu;
    bool trace_cpu_irq;
    bool trace_lcd;
    bool trace_input;
    bool trace_timer;
    bool trace_apu;
    bool trace_serial;
    bool trace_mapper;
    int trace_lcd_events;
    int trace_input_events;
    int trace_timer_events;
    int trace_apu_events;
    int trace_serial_events;
    int trace_mapper_events;
    int trace_output;
    int trace_capacity;
    int trace_disk_dir_option;
    int trace_disk_size;
    std::string trace_disk_path;
    bool dis_show_mem;
    bool dis_show_symbols;
    bool dis_show_segment;
    bool dis_show_bank;
    bool dis_show_auto_symbols;
    bool dis_dim_auto_symbols;
    bool dis_replace_symbols;
    bool dis_replace_labels;
    int dis_syntax;
    int dis_look_ahead_count;
    int font_size;
    int scale;
    bool multi_viewport;
    bool single_instance;
    bool auto_debug_settings;
    int mem_editor_bytes_per_row[config_memory_editor_count];
    int mem_editor_preview_data_type[config_memory_editor_count];
    int mem_editor_preview_endianess[config_memory_editor_count];
    bool mem_editor_uppercase_hex[config_memory_editor_count];
    bool mem_editor_gray_out_zeros[config_memory_editor_count];
};

EXTERN config_Emulator config_emulator;
EXTERN config_Video config_video;
EXTERN config_Audio config_audio;
EXTERN config_Rewind config_rewind;
EXTERN config_Input config_input;
EXTERN config_Input_Gamepad_Shortcuts config_input_gamepad_shortcuts;
EXTERN config_Hotkey config_hotkeys[config_HotkeyIndex_COUNT];
EXTERN config_Debug config_debug;

#endif /* CONFIG_DATA_H */

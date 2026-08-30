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

#include "config_macros.h"
#include "shader_preset.h"

static inline void process(config_Operation operation)
{
    //**************************************
    // Debug
    //**************************************

    // Debugger windows
    CONFIG_BOOL("Debug", "Debug", config_debug.debug, false);
    CONFIG_BOOL("Debug", "Disassembler", config_debug.show_disassembler, true);
    CONFIG_BOOL("Debug", "Screen", config_debug.show_screen, true);
    CONFIG_BOOL("Debug", "Memory", config_debug.show_memory, false);
    CONFIG_BOOL("Debug", "Processor", config_debug.show_processor, true);
    CONFIG_BOOL("Debug", "CallStack", config_debug.show_call_stack, false);
    CONFIG_BOOL("Debug", "Breakpoints", config_debug.show_breakpoints, false);
    CONFIG_BOOL("Debug", "Symbols", config_debug.show_symbols, false);
    CONFIG_BOOL("Debug", "Video", config_debug.show_video, false);
    CONFIG_BOOL("Debug", "VideoNameTable", config_debug.show_video_nametable, false);
    CONFIG_BOOL("Debug", "VideoTiles", config_debug.show_video_tiles, false);
    CONFIG_BOOL("Debug", "VideoSprites", config_debug.show_video_sprites, false);
    CONFIG_BOOL("Debug", "VideoPalettes", config_debug.show_video_palettes, false);
    CONFIG_BOOL("Debug", "VideoGBCPalettes", config_debug.show_video_gbc_palettes, false);
    CONFIG_BOOL("Debug", "IO", config_debug.show_io, false);
    CONFIG_BOOL("Debug", "PSG", config_debug.show_psg, false);
    CONFIG_BOOL("Debug", "LinkCable", config_debug.show_link_cable, false);
    CONFIG_BOOL("Debug", "LinkCableTransport", config_debug.show_link_cable_transport, false);
    CONFIG_BOOL("Debug", "TraceLogger", config_debug.show_trace_logger, false);
    CONFIG_BOOL("Debug", "Rewind", config_debug.show_rewind, false);
    CONFIG_BOOL("Debug", "SGBState", config_debug.show_sgb_state, false);
    CONFIG_BOOL("Debug", "SGBVideo", config_debug.show_sgb_video, false);
    CONFIG_BOOL("Debug", "SGBPalettes", config_debug.show_sgb_palettes, false);
    CONFIG_BOOL("Debug", "SGBSystemPalettes", config_debug.show_sgb_system_palettes, false);
    CONFIG_BOOL("Debug", "SGBBorderPalettes", config_debug.show_sgb_border_palettes, false);

    // Trace logger
    CONFIG_BOOL("Debug", "TraceCounter", config_debug.trace_counter, true);
    CONFIG_BOOL("Debug", "TraceCycles", config_debug.trace_cycles, false);
    CONFIG_BOOL("Debug", "TraceBank", config_debug.trace_bank, true);
    CONFIG_BOOL("Debug", "TraceRegisters", config_debug.trace_registers, true);
    CONFIG_BOOL("Debug", "TraceFlags", config_debug.trace_flags, true);
    CONFIG_BOOL("Debug", "TraceBytes", config_debug.trace_bytes, true);
    CONFIG_BOOL("Debug", "TraceCpuEnabled", config_debug.trace_cpu_enabled, true);
    CONFIG_BOOL("Debug", "TraceCpu", config_debug.trace_cpu, true);
    CONFIG_BOOL("Debug", "TraceCpuIrq", config_debug.trace_cpu_irq, true);
    CONFIG_BOOL("Debug", "TraceLcd", config_debug.trace_lcd, false);
    CONFIG_BOOL("Debug", "TraceInput", config_debug.trace_input, false);
    CONFIG_BOOL("Debug", "TraceTimer", config_debug.trace_timer, false);
    CONFIG_BOOL("Debug", "TraceApu", config_debug.trace_apu, false);
    CONFIG_BOOL("Debug", "TraceSerial", config_debug.trace_serial, false);
    CONFIG_BOOL("Debug", "TraceMapper", config_debug.trace_mapper, false);
    CONFIG_INT_RANGE("Debug", "TraceLcdEvents", config_debug.trace_lcd_events, TRACE_LCD_FILTER_ALL, 0, TRACE_LCD_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceInputEvents", config_debug.trace_input_events, TRACE_INPUT_FILTER_ALL, 0, TRACE_INPUT_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceTimerEvents", config_debug.trace_timer_events, TRACE_TIMER_FILTER_ALL, 0, TRACE_TIMER_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceApuEvents", config_debug.trace_apu_events, TRACE_APU_FILTER_ALL, 0, TRACE_APU_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceSerialEvents", config_debug.trace_serial_events, TRACE_SERIAL_FILTER_ALL, 0, TRACE_SERIAL_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceMapperEvents", config_debug.trace_mapper_events, TRACE_MAPPER_FILTER_ALL, 0, TRACE_MAPPER_FILTER_ALL);
    CONFIG_INT_RANGE("Debug", "TraceOutput", config_debug.trace_output, 0, 0, 1);
    CONFIG_INT_RANGE("Debug", "TraceCapacity", config_debug.trace_capacity, 0, 0, 4);
    CONFIG_INT_RANGE("Debug", "TraceDiskDirOption", config_debug.trace_disk_dir_option, 0, 0, 2);
    CONFIG_INT_RANGE("Debug", "TraceDiskSize", config_debug.trace_disk_size, 2, 0, 6);
    CONFIG_STRING_NOT_EMPTY("Debug", "TraceDiskPath", config_debug.trace_disk_path, config_root_path);

    // Disassembler
    CONFIG_BOOL("Debug", "DisMem", config_debug.dis_show_mem, true);
    CONFIG_BOOL("Debug", "DisSymbols", config_debug.dis_show_symbols, true);
    CONFIG_BOOL("Debug", "DisSegment", config_debug.dis_show_segment, true);
    CONFIG_BOOL("Debug", "DisBank", config_debug.dis_show_bank, true);
    CONFIG_BOOL("Debug", "DisAutoSymbols", config_debug.dis_show_auto_symbols, true);
    CONFIG_BOOL("Debug", "DisDimAutoSymbols", config_debug.dis_dim_auto_symbols, false);
    CONFIG_BOOL("Debug", "DisReplaceSymbols", config_debug.dis_replace_symbols, true);
    CONFIG_BOOL("Debug", "DisReplaceLabels", config_debug.dis_replace_labels, true);
    CONFIG_INT_RANGE("Debug", "DisSyntax", config_debug.dis_syntax, GB_Disassembler_Syntax_Gearboy, GB_Disassembler_Syntax_Gearboy, GB_Disassembler_Syntax_Count - 1);
    CONFIG_INT("Debug", "DisLookAheadCount", config_debug.dis_look_ahead_count, 20);

    // Interface
    CONFIG_INT_RANGE("Debug", "FontSize", config_debug.font_size, 0, 0, 3);
    CONFIG_INT("Debug", "Scale", config_debug.scale, 2);
    CONFIG_BOOL("Debug", "MultiViewport", config_debug.multi_viewport, false);
    CONFIG_BOOL("Debug", "SingleInstance", config_debug.single_instance, false);
    CONFIG_BOOL("Debug", "AutoDebugSettings", config_debug.auto_debug_settings, false);

    // Memory editors
    for (int i = 0; i < config_memory_editor_count; i++)
    {
        char section[32];
        snprintf(section, sizeof(section), "MemEditor_%d", i);
        CONFIG_INT(section, "BytesPerRow", config_debug.mem_editor_bytes_per_row[i], 16);
        CONFIG_INT(section, "PreviewDataType", config_debug.mem_editor_preview_data_type[i], 0);
        CONFIG_INT(section, "PreviewEndianess", config_debug.mem_editor_preview_endianess[i], 0);
        CONFIG_BOOL(section, "UppercaseHex", config_debug.mem_editor_uppercase_hex[i], true);
        CONFIG_BOOL(section, "GrayOutZeros", config_debug.mem_editor_gray_out_zeros[i], true);
    }

    //**************************************
    // Emulator
    //**************************************

    // Window and interface
    CONFIG_BOOL("Emulator", "Maximized", config_emulator.maximized, false);
    CONFIG_BOOL("Emulator", "FullScreen", config_emulator.fullscreen, false);
    CONFIG_INT("Emulator", "FullScreenMode", config_emulator.fullscreen_mode, 0);
    CONFIG_BOOL("Emulator", "AlwaysShowMenu", config_emulator.always_show_menu, false);
    CONFIG_INT_RANGE("Emulator", "Theme", config_emulator.theme, config_Theme_Dark, config_Theme_Light, config_Theme_Dark);
    CONFIG_INT("Emulator", "WindowWidth", config_emulator.window_width, 800);
    CONFIG_INT("Emulator", "WindowHeight", config_emulator.window_height, 700);
    CONFIG_BOOL("Emulator", "StatusMessages", config_emulator.status_messages, false);
    CONFIG_BOOL("Emulator", "AllowScreenSaver", config_emulator.allow_screensaver, false);

    // Emulation
    CONFIG_INT("Emulator", "FFWD", config_emulator.ffwd_speed, 1);
    CONFIG_INT_RANGE("Emulator", "RunAhead", config_emulator.runahead, 0, 0, 3);
    CONFIG_INT_RANGE("Emulator", "SaveSlot", config_emulator.save_slot, 0, 0, 4);
    CONFIG_BOOL("Emulator", "StartPaused", config_emulator.start_paused, false);
    CONFIG_BOOL("Emulator", "PauseWhenInactive", config_emulator.pause_when_inactive, true);
    CONFIG_BOOL("Emulator", "SoftPatching", config_emulator.softpatching, true);
    CONFIG_BOOL("Emulator", "ForceDMG", config_emulator.force_dmg, false);
    CONFIG_BOOL("Emulator", "ForceGBA", config_emulator.force_gba, false);
    CONFIG_BOOL("Emulator", "SGB", config_emulator.sgb, true);
    CONFIG_BOOL("Emulator", "SGBBorder", config_emulator.sgb_border, true);
    CONFIG_INT("Emulator", "MBC", config_emulator.mbc, 0);
    CONFIG_BOOL("Emulator", "DMGBootrom", config_emulator.dmg_bootrom, false);
    CONFIG_STRING("Emulator", "DMGBootromPath", config_emulator.dmg_bootrom_path, "");
    CONFIG_BOOL("Emulator", "GBCBootrom", config_emulator.gbc_bootrom, false);
    CONFIG_STRING("Emulator", "GBCBootromPath", config_emulator.gbc_bootrom_path, "");

    // Tilt controls
    CONFIG_INT("Emulator", "TiltSource", config_emulator.tilt_source, 0);
    CONFIG_INT("Emulator", "MouseSensitivityX", config_emulator.mouse_sensitivity_x, 5);
    CONFIG_INT("Emulator", "MouseSensitivityY", config_emulator.mouse_sensitivity_y, 5);
    CONFIG_BOOL("Emulator", "MouseInvertX", config_emulator.mouse_invert_x, false);
    CONFIG_BOOL("Emulator", "MouseInvertY", config_emulator.mouse_invert_y, false);
    CONFIG_INT("Emulator", "SensorSensitivityX", config_emulator.sensor_sensitivity_x, 5);
    CONFIG_INT("Emulator", "SensorSensitivityY", config_emulator.sensor_sensitivity_y, 5);
    CONFIG_BOOL("Emulator", "SensorInvertX", config_emulator.sensor_invert_x, false);
    CONFIG_BOOL("Emulator", "SensorInvertY", config_emulator.sensor_invert_y, false);
    CONFIG_INT("Emulator", "AnalogSensitivityX", config_emulator.analog_sensitivity_x, 5);
    CONFIG_INT("Emulator", "AnalogSensitivityY", config_emulator.analog_sensitivity_y, 5);
    CONFIG_BOOL("Emulator", "AnalogInvertX", config_emulator.analog_invert_x, false);
    CONFIG_BOOL("Emulator", "AnalogInvertY", config_emulator.analog_invert_y, false);
    CONFIG_BOOL("Emulator", "CaptureMouse", config_emulator.capture_mouse, false);

    // Files and paths
    CONFIG_INT("Emulator", "SaveFilesDirOption", config_emulator.savefiles_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "SaveFilesPath", config_emulator.savefiles_path, config_root_path);
    CONFIG_INT("Emulator", "SaveStatesDirOption", config_emulator.savestates_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "SaveStatesPath", config_emulator.savestates_path, config_root_path);
    CONFIG_INT("Emulator", "ScreenshotDirOption", config_emulator.screenshots_dir_option, 0);
    CONFIG_STRING_NOT_EMPTY("Emulator", "ScreenshotPath", config_emulator.screenshots_path, config_root_path);
    CONFIG_STRING("Emulator", "LastOpenPath", config_emulator.last_open_path, "");
    CONFIG_STRING_ARRAY("Emulator", "RecentROM%d", config_emulator.recent_roms, config_max_recent_roms, "");

    // Services
    CONFIG_INT("Emulator", "MCPTCPPort", config_emulator.mcp_tcp_port, 7777);
    CONFIG_STRING_NOT_EMPTY("Emulator", "MCPHTTPAddress", config_emulator.mcp_http_address, "127.0.0.1");
    CONFIG_INT_RANGE("Emulator", "LinkCableSession", config_emulator.link_cable_session, 1, 1, 255);
#if defined(_WIN32)
    CONFIG_INT_RANGE("Emulator", "LinkCableStallUs", config_emulator.link_cable_stall_us, 5000, 1000, 10000);
#elif defined(__APPLE__)
    CONFIG_INT_RANGE("Emulator", "LinkCableStallUs", config_emulator.link_cable_stall_us, 100, 50, 1000);
#else
    CONFIG_INT_RANGE("Emulator", "LinkCableStallUs", config_emulator.link_cable_stall_us, 250, 50, 2000);
#endif

    //**************************************
    // Video
    //**************************************

    // Display
    CONFIG_INT("Video", "Scale", config_video.scale, 0);
    CONFIG_INT_RANGE("Video", "ScaleManual", config_video.scale_manual, 1, 1, 20);
    CONFIG_INT("Video", "AspectRatio", config_video.ratio, 0);
    CONFIG_BOOL("Video", "FPS", config_video.fps, false);
    CONFIG_INT_RANGE("Video", "Palette", config_video.palette, 0, 0, 10);
    CONFIG_BOOL("Video", "ColorCorrection", config_video.color_correction, true);
    CONFIG_BOOL("Video", "SpriteLimit", config_video.sprite_limit, false);
    CONFIG_INT_RANGE("Video", "ShaderMode", config_video.shader_mode, config_ShaderMode_PixelPerfect, config_ShaderMode_PixelPerfect, config_ShaderMode_External);

    if (operation == config_Operation_Write)
    {
        std::string preset_file = get_filename(config_video.shader_preset_path.c_str());
        CONFIG_STRING("Video", "ShaderPresetFile", preset_file, "");
    }
    else
    {
        CONFIG_STRING("Video", "ShaderPresetFile", config_video.shader_preset_path, "");
    }

    CONFIG_INT_RANGE("Video", "SyncMode", config_video.sync_mode, config_VideoSync_Fixed, config_VideoSync_Disabled, config_VideoSync_VRR);

    // Background colors
    CONFIG_FLOAT("Video", "BackgroundColorR", config_video.background_color[config_Theme_Dark][0], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorG", config_video.background_color[config_Theme_Dark][1], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorB", config_video.background_color[config_Theme_Dark][2], 0.1f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerR", config_video.background_color_debugger[config_Theme_Dark][0], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerG", config_video.background_color_debugger[config_Theme_Dark][1], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerB", config_video.background_color_debugger[config_Theme_Dark][2], 0.2f);
    CONFIG_FLOAT("Video", "BackgroundColorLightR", config_video.background_color[config_Theme_Light][0], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorLightG", config_video.background_color[config_Theme_Light][1], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorLightB", config_video.background_color[config_Theme_Light][2], 128.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightR", config_video.background_color_debugger[config_Theme_Light][0], 233.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightG", config_video.background_color_debugger[config_Theme_Light][1], 232.0f / 255.0f);
    CONFIG_FLOAT("Video", "BackgroundColorDebuggerLightB", config_video.background_color_debugger[config_Theme_Light][2], 230.0f / 255.0f);

    // Custom palettes
    const GB_Color default_palettes[config_max_custom_palettes][4] = {
        {{0xC4, 0xF0, 0xC2}, {0x5A, 0xB9, 0xA8}, {0x1E, 0x60, 0x6E}, {0x2D, 0x1B, 0x00}},
        {{0xF8, 0xE3, 0xC4}, {0xCC, 0x34, 0x95}, {0x6B, 0x1F, 0xB1}, {0x0B, 0x06, 0x30}},
        {{0xEF, 0xF9, 0xD6}, {0xBA, 0x50, 0x44}, {0x7A, 0x1C, 0x4B}, {0x1B, 0x03, 0x26}},
        {{0xFF, 0xE4, 0xC2}, {0xDC, 0xA4, 0x56}, {0xA9, 0x60, 0x4C}, {0x42, 0x29, 0x36}},
        {{0xCE, 0xCE, 0xCE}, {0x6F, 0x9E, 0xDF}, {0x42, 0x67, 0x8E}, {0x10, 0x25, 0x33}}
    };

    for (int i = 0; i < config_max_custom_palettes; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            char key[32];

            int red = config_video.color[i][c].red;
            snprintf(key, sizeof(key), "CustomPalette%i%iR", i, c);
            CONFIG_INT("Video", key, red, default_palettes[i][c].red);
            config_video.color[i][c].red = (u8)red;

            int green = config_video.color[i][c].green;
            snprintf(key, sizeof(key), "CustomPalette%i%iG", i, c);
            CONFIG_INT("Video", key, green, default_palettes[i][c].green);
            config_video.color[i][c].green = (u8)green;

            int blue = config_video.color[i][c].blue;
            snprintf(key, sizeof(key), "CustomPalette%i%iB", i, c);
            CONFIG_INT("Video", key, blue, default_palettes[i][c].blue);
            config_video.color[i][c].blue = (u8)blue;
        }
    }

    //**************************************
    // Audio
    //**************************************

    CONFIG_BOOL("Audio", "Enable", config_audio.enable, true);
    CONFIG_BOOL("Audio", "Sync", config_audio.sync, true);
    CONFIG_FLOAT_RANGE("Audio", "MasterVolume", config_audio.master_volume, 1.0f, 0.0f, 2.0f);
    CONFIG_INT("Audio", "BufferCount", config_audio.buffer_count, 3);

    //**************************************
    // Rewind
    //**************************************

    CONFIG_BOOL("Rewind", "Enabled", config_rewind.enabled, true);
    CONFIG_INT("Rewind", "BufferSeconds", config_rewind.buffer_seconds, 10);
    CONFIG_INT("Rewind", "FramesPerSnapshot", config_rewind.frames_per_snapshot, 1);
    CONFIG_FLOAT("Rewind", "Speed", config_rewind.speed, 2.0f);

    //**************************************
    // Input
    //**************************************

    // Keyboard
    CONFIG_SCANCODE("Input", "KeyLeft", config_input.key_left, SDL_SCANCODE_LEFT);
    CONFIG_SCANCODE("Input", "KeyRight", config_input.key_right, SDL_SCANCODE_RIGHT);
    CONFIG_SCANCODE("Input", "KeyUp", config_input.key_up, SDL_SCANCODE_UP);
    CONFIG_SCANCODE("Input", "KeyDown", config_input.key_down, SDL_SCANCODE_DOWN);
    CONFIG_SCANCODE("Input", "KeyA", config_input.key_a, SDL_SCANCODE_S);
    CONFIG_SCANCODE("Input", "KeyB", config_input.key_b, SDL_SCANCODE_A);
    CONFIG_SCANCODE("Input", "KeyStart", config_input.key_start, SDL_SCANCODE_RETURN);
    CONFIG_SCANCODE("Input", "KeySelect", config_input.key_select, SDL_SCANCODE_SPACE);

    // Gamepad
    CONFIG_BOOL("Input", "AllowUpDown", config_input.allow_up_down, false);
    CONFIG_BOOL("Input", "Gamepad", config_input.gamepad, true);
    CONFIG_INT("Input", "GamepadDirectional", config_input.gamepad_directional, 0);
    CONFIG_BOOL("Input", "GamepadInvertX", config_input.gamepad_invert_x_axis, false);
    CONFIG_BOOL("Input", "GamepadInvertY", config_input.gamepad_invert_y_axis, false);
    CONFIG_INT("Input", "GamepadA", config_input.gamepad_a, SDL_GAMEPAD_BUTTON_EAST);
    CONFIG_INT("Input", "GamepadB", config_input.gamepad_b, SDL_GAMEPAD_BUTTON_SOUTH);
    CONFIG_INT("Input", "GamepadStart", config_input.gamepad_start, SDL_GAMEPAD_BUTTON_START);
    CONFIG_INT("Input", "GamepadSelect", config_input.gamepad_select, SDL_GAMEPAD_BUTTON_BACK);
    CONFIG_INT("Input", "GamepadX", config_input.gamepad_x_axis, SDL_GAMEPAD_AXIS_LEFTX);
    CONFIG_INT("Input", "GamepadY", config_input.gamepad_y_axis, SDL_GAMEPAD_AXIS_LEFTY);

    // Gamepad shortcuts
    CONFIG_INT_ARRAY("InputGamepadShortcuts", "Shortcut%d", config_input_gamepad_shortcuts.gamepad_shortcuts, config_HotkeyIndex_COUNT, SDL_GAMEPAD_BUTTON_INVALID);

    // Hotkeys
    CONFIG_HOTKEY("OpenROM", config_hotkeys[config_HotkeyIndex_OpenROM], SDL_SCANCODE_O, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("ReloadROM", config_hotkeys[config_HotkeyIndex_ReloadROM], SDL_SCANCODE_D, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Quit", config_hotkeys[config_HotkeyIndex_Quit], SDL_SCANCODE_Q, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Reset", config_hotkeys[config_HotkeyIndex_Reset], SDL_SCANCODE_R, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Pause", config_hotkeys[config_HotkeyIndex_Pause], SDL_SCANCODE_P, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("FFWD", config_hotkeys[config_HotkeyIndex_FFWD], SDL_SCANCODE_F, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Rewind", config_hotkeys[config_HotkeyIndex_Rewind], SDL_SCANCODE_BACKSPACE, SDL_KMOD_NONE);
    CONFIG_HOTKEY("SaveState", config_hotkeys[config_HotkeyIndex_SaveState], SDL_SCANCODE_S, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("LoadState", config_hotkeys[config_HotkeyIndex_LoadState], SDL_SCANCODE_L, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Screenshot", config_hotkeys[config_HotkeyIndex_Screenshot], SDL_SCANCODE_X, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Fullscreen", config_hotkeys[config_HotkeyIndex_Fullscreen], SDL_SCANCODE_F12, SDL_KMOD_NONE);
    CONFIG_HOTKEY("ShowMainMenu", config_hotkeys[config_HotkeyIndex_ShowMainMenu], SDL_SCANCODE_M, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("CaptureMouse", config_hotkeys[config_HotkeyIndex_CaptureMouse], SDL_SCANCODE_F1, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugStepInto", config_hotkeys[config_HotkeyIndex_DebugStepInto], SDL_SCANCODE_F11, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugStepOver", config_hotkeys[config_HotkeyIndex_DebugStepOver], SDL_SCANCODE_F10, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugStepOut", config_hotkeys[config_HotkeyIndex_DebugStepOut], SDL_SCANCODE_F11, SDL_KMOD_SHIFT);
    CONFIG_HOTKEY("DebugStepFrame", config_hotkeys[config_HotkeyIndex_DebugStepFrame], SDL_SCANCODE_F6, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugContinue", config_hotkeys[config_HotkeyIndex_DebugContinue], SDL_SCANCODE_F5, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugBreak", config_hotkeys[config_HotkeyIndex_DebugBreak], SDL_SCANCODE_F7, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugRunToCursor", config_hotkeys[config_HotkeyIndex_DebugRunToCursor], SDL_SCANCODE_F8, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugBreakpoint", config_hotkeys[config_HotkeyIndex_DebugBreakpoint], SDL_SCANCODE_F9, SDL_KMOD_NONE);
    CONFIG_HOTKEY("DebugGoBack", config_hotkeys[config_HotkeyIndex_DebugGoBack], SDL_SCANCODE_BACKSPACE, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot1", config_hotkeys[config_HotkeyIndex_SelectSlot1], SDL_SCANCODE_1, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot2", config_hotkeys[config_HotkeyIndex_SelectSlot2], SDL_SCANCODE_2, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot3", config_hotkeys[config_HotkeyIndex_SelectSlot3], SDL_SCANCODE_3, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot4", config_hotkeys[config_HotkeyIndex_SelectSlot4], SDL_SCANCODE_4, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("SelectSlot5", config_hotkeys[config_HotkeyIndex_SelectSlot5], SDL_SCANCODE_5, SDL_KMOD_CTRL);
    CONFIG_HOTKEY("Mute", config_hotkeys[config_HotkeyIndex_Mute], SDL_SCANCODE_U, SDL_KMOD_CTRL);
}

//**************************************
// Emulator-specific behavior
//**************************************

static void before_read(int file_version);
static void after_read(int file_version);
static void before_write(void);
static void after_write(void);
static void before_defaults(void);
static void after_defaults(void);
static void normalize(void);
static void migrate(int file_version);
static void sync_shader_preset_parameter_defaults(void);

static void before_read(int file_version)
{
    migrate(file_version);
}

static void after_read(int file_version)
{
    UNUSED(file_version);
    sync_shader_preset_parameter_defaults();
}

static void before_write(void)
{
    if (config_emulator.ffwd)
        config_audio.sync = true;
}

static void after_write(void)
{
    sync_shader_preset_parameter_defaults();
}

static void before_defaults(void)
{
}

static void after_defaults(void)
{
    config_emulator.paused = false;
    config_emulator.ffwd = false;
    config_emulator.show_info = false;
}

static void normalize(void)
{
#if defined(GEARBOY_DISABLE_DISASSEMBLER)
    config_debug.debug = false;
#endif
#if !defined(_WIN32)
    if (config_video.sync_mode == config_VideoSync_VRR)
        config_video.sync_mode = config_VideoSync_Fixed;
#endif
}

static void migrate(int file_version)
{
    std::string stored;

    if (file_version < 7)
    {
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        bool old_default = get_setting("Video", "BackgroundColorDebuggerLightR", &stored) &&
                           parse_float_string(stored, &red) &&
                           get_setting("Video", "BackgroundColorDebuggerLightG", &stored) &&
                           parse_float_string(stored, &green) &&
                           get_setting("Video", "BackgroundColorDebuggerLightB", &stored) &&
                           parse_float_string(stored, &blue) &&
                           std::fabs(red - (160.0f / 255.0f)) < 0.005f &&
                           std::fabs(green - (160.0f / 255.0f)) < 0.005f &&
                           std::fabs(blue - (160.0f / 255.0f)) < 0.005f;

        if (old_default)
        {
            write_float("Video", "BackgroundColorDebuggerLightR", 233.0f / 255.0f);
            write_float("Video", "BackgroundColorDebuggerLightG", 232.0f / 255.0f);
            write_float("Video", "BackgroundColorDebuggerLightB", 230.0f / 255.0f);
        }
    }

    if (file_version < 5)
    {
        write_bool("Debug", "TraceCycles", false);

        bool trace_cpu = read_bool("Debug", "TraceCpu", true);
        bool trace_cpu_irq = read_bool("Debug", "TraceCpuIrq", true);
        bool trace_lcd_write = read_bool("Debug", "TraceLcdWrite", true);
        bool trace_lcd_status = read_bool("Debug", "TraceLcdStatus", true);
        bool trace_apu_write = read_bool("Debug", "TraceApuWrite", true);
        bool trace_io_write = read_bool("Debug", "TraceIoWrite", true);
        bool trace_bank_switch = read_bool("Debug", "TraceBankSwitch", true);
        bool default_trace_filters = trace_cpu && trace_cpu_irq && trace_lcd_write &&
            trace_lcd_status && trace_apu_write && trace_io_write && trace_bank_switch;

        write_bool("Debug", "TraceCpuEnabled", trace_cpu || trace_cpu_irq);
        write_bool("Debug", "TraceLcd", default_trace_filters ? false : (trace_lcd_write || trace_lcd_status || trace_io_write));
        write_bool("Debug", "TraceInput", false);
        write_bool("Debug", "TraceTimer", false);
        write_bool("Debug", "TraceApu", default_trace_filters ? false : trace_apu_write);
        write_bool("Debug", "TraceSerial", false);
        write_bool("Debug", "TraceMapper", default_trace_filters ? false : trace_bank_switch);

        int lcd_events = 0;
        if (trace_lcd_write)
            lcd_events |= TRACE_LCD_FILTER_REGISTERS;
        if (trace_lcd_status)
            lcd_events |= TRACE_LCD_FILTER_INTERRUPTS | TRACE_LCD_FILTER_DMA;
        if (trace_io_write)
            lcd_events |= TRACE_LCD_FILTER_DMA;
        write_int("Debug", "TraceLcdEvents", lcd_events);
        write_int("Debug", "TraceInputEvents", TRACE_INPUT_FILTER_ALL);
        write_int("Debug", "TraceTimerEvents", TRACE_TIMER_FILTER_ALL);
        write_int("Debug", "TraceApuEvents", TRACE_APU_FILTER_ALL);
        write_int("Debug", "TraceSerialEvents", TRACE_SERIAL_FILTER_ALL);
        write_int("Debug", "TraceMapperEvents", TRACE_MAPPER_FILTER_ALL);
    }

    int sync_mode = -1;
    bool valid_sync_mode = get_setting("Video", "SyncMode", &stored) &&
        parse_int_string(stored, &sync_mode) && sync_mode >= config_VideoSync_Disabled &&
        sync_mode <= config_VideoSync_VRR;

    if (file_version < 4 || !valid_sync_mode)
    {
        bool sync = read_bool("Video", "Sync", true);
        bool vrr = read_bool("Video", "VRR", false);
        sync_mode = sync ? (vrr ? config_VideoSync_VRR : config_VideoSync_Fixed) : config_VideoSync_Disabled;
        write_int("Video", "SyncMode", sync_mode);
    }
}

static void sync_shader_preset_parameter_defaults(void)
{
    ShaderPresetInfo presets[SHADER_PRESET_MAX_DISCOVERED];
    int preset_count = shader_preset_scan_bundled(presets, SHADER_PRESET_MAX_DISCOVERED);

    for (int i = 0; i < preset_count; i++)
    {
        ShaderPreset preset;
        char error[512];
        if (!shader_preset_load(presets[i].path, &preset, error, sizeof(error)))
            continue;

        char preset_file[SHADER_PRESET_MAX_PATH];
        if (!shader_preset_get_config_path(preset.preset_path, preset_file, sizeof(preset_file)))
            continue;

        std::string section = shader_preset_section_name(preset_file);
        for (int j = 0; j < preset.parameter_count; j++)
        {
            ShaderPresetParameter* parameter = &preset.parameters[j];
            if (config_ini_data[section].has(parameter->name))
                continue;

            write_float(section.c_str(), parameter->name, parameter->default_value);
        }
    }
}

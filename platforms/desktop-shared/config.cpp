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

#include <SDL.h>
#include <iomanip>
#include "../../src/gearboy.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

#define CONFIG_IMPORT
#include "config.h"

static bool check_portable(void);
static int read_int(const char* group, const char* key, int default_value);
static void write_int(const char* group, const char* key, int integer);
static float read_float(const char* group, const char* key, float default_value);
static void write_float(const char* group, const char* key, float value);
static bool read_bool(const char* group, const char* key, bool default_value);
static void write_bool(const char* group, const char* key, bool boolean);
static std::string read_string(const char* group, const char* key);
static void write_string(const char* group, const char* key, std::string value);

void config_init(void)
{
    if (check_portable())
        config_root_path = SDL_GetBasePath();
    else
        config_root_path = SDL_GetPrefPath("Geardome", GEARBOY_TITLE);

    strcpy(config_emu_file_path, config_root_path);
    strcat(config_emu_file_path, "config.ini");

    strcpy(config_imgui_file_path, config_root_path);
    strcat(config_imgui_file_path, "imgui.ini");

    config_ini_file = new mINI::INIFile(config_emu_file_path);
}

void config_destroy(void)
{
    SafeDelete(config_ini_file)
    SDL_free(config_root_path);
}

void config_read(void)
{
    if (!config_ini_file->read(config_ini_data))
    {
        Log("Unable to load settings from %s", config_emu_file_path);
        return;
    }

    Log("Loading settings from %s", config_emu_file_path);

    config_debug.debug = read_bool("Debug", "Debug", false);
    config_debug.show_audio = read_bool("Debug", "Audio", false);
    config_debug.show_disassembler = read_bool("Debug", "Disassembler", true);
    config_debug.show_gameboy = read_bool("Debug", "GameBoy", true);
    config_debug.show_iomap = read_bool("Debug", "IOMap", false);
    config_debug.show_memory = read_bool("Debug", "Memory", true);
    config_debug.show_processor = read_bool("Debug", "Processor", true);
    config_debug.show_video = read_bool("Debug", "Video", false);
    config_debug.font_size = read_int("Debug", "FontSize", 0);
    config_debug.multi_viewport = read_bool("Debug", "MultiViewport", false);

    config_emulator.maximized = read_bool("Emulator", "Maximized", false);
    config_emulator.fullscreen = read_bool("Emulator", "FullScreen", false);
    config_emulator.always_show_menu = read_bool("Emulator", "AlwaysShowMenu", false);
    config_emulator.ffwd_speed = read_int("Emulator", "FFWD", 1);
    config_emulator.save_slot = read_int("Emulator", "SaveSlot", 0);
    config_emulator.start_paused = read_bool("Emulator", "StartPaused", false);
    config_emulator.pause_when_inactive = read_bool("Emulator", "PauseWhenInactive", true);
    config_emulator.force_dmg = read_bool("Emulator", "ForceDMG", false);
    config_emulator.force_gba = read_bool("Emulator", "ForceGBA", false);
    config_emulator.mbc = read_int("Emulator", "MBC", 0);
    config_emulator.dmg_bootrom = read_bool("Emulator", "DMGBootrom", false);
    config_emulator.dmg_bootrom_path = read_string("Emulator", "DMGBootromPath");
    config_emulator.gbc_bootrom = read_bool("Emulator", "GBCBootrom", false);
    config_emulator.gbc_bootrom_path = read_string("Emulator", "GBCBootromPath");
    config_emulator.savefiles_dir_option = read_int("Emulator", "SaveFilesDirOption", 0);
    config_emulator.savefiles_path = read_string("Emulator", "SaveFilesPath");
    config_emulator.savestates_dir_option = read_int("Emulator", "SaveStatesDirOption", 0);
    config_emulator.savestates_path = read_string("Emulator", "SaveStatesPath");
    config_emulator.last_open_path = read_string("Emulator", "LastOpenPath");
    config_emulator.window_width = read_int("Emulator", "WindowWidth", 800);
    config_emulator.window_height = read_int("Emulator", "WindowHeight", 700);
    config_emulator.status_messages = read_bool("Emulator", "StatusMessages", false);

    if (config_emulator.savefiles_path.empty())
    {
        config_emulator.savefiles_path = config_root_path;
    }
    if (config_emulator.savestates_path.empty())
    {
        config_emulator.savestates_path = config_root_path;
    }

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        config_emulator.recent_roms[i] = read_string("Emulator", item.c_str());
    }

    config_video.scale = read_int("Video", "Scale", 0);
    if (config_video.scale > 3)
        config_video.scale -= 3;
    config_video.scale_manual = read_int("Video", "ScaleManual", 1);
    config_video.ratio = read_int("Video", "AspectRatio", 0);
    config_video.fps = read_bool("Video", "FPS", false);
    config_video.bilinear = read_bool("Video", "Bilinear", false);
    config_video.mix_frames = read_bool("Video", "MixFrames", true);
    config_video.mix_frames_intensity = read_float("Video", "MixFramesIntensity", 0.75f);
    config_video.matrix = read_bool("Video", "Matrix", true);
    config_video.matrix_intensity = read_float("Video", "MatrixIntensity", 0.20f);
    config_video.palette = read_int("Video", "Palette", 0);
    for (int i = 0; i < config_max_custom_palettes; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            char pal_label_r[32];
            char pal_label_g[32];
            char pal_label_b[32];
            snprintf(pal_label_r, sizeof(pal_label_r), "CustomPalette%i%iR", i, c);
            snprintf(pal_label_g, sizeof(pal_label_g), "CustomPalette%i%iG", i, c);
            snprintf(pal_label_b, sizeof(pal_label_b), "CustomPalette%i%iB", i, c);
            config_video.color[i][c].red = read_int("Video", pal_label_r, config_video.color[i][c].red);
            config_video.color[i][c].green = read_int("Video", pal_label_g, config_video.color[i][c].green);
            config_video.color[i][c].blue = read_int("Video", pal_label_b, config_video.color[i][c].blue);
        }
    }
    config_video.sync = read_bool("Video", "Sync", true);
    config_video.color_correction = read_bool("Video", "ColorCorrection", true);
    
    config_audio.enable = read_bool("Audio", "Enable", true);
    config_audio.sync = read_bool("Audio", "Sync", true);

    config_input.key_left = (SDL_Scancode)read_int("Input", "KeyLeft", SDL_SCANCODE_LEFT);
    config_input.key_right = (SDL_Scancode)read_int("Input", "KeyRight", SDL_SCANCODE_RIGHT);
    config_input.key_up = (SDL_Scancode)read_int("Input", "KeyUp", SDL_SCANCODE_UP);
    config_input.key_down = (SDL_Scancode)read_int("Input", "KeyDown", SDL_SCANCODE_DOWN);
    config_input.key_a = (SDL_Scancode)read_int("Input", "KeyA", SDL_SCANCODE_A);
    config_input.key_b = (SDL_Scancode)read_int("Input", "KeyB", SDL_SCANCODE_S);
    config_input.key_start = (SDL_Scancode)read_int("Input", "KeyStart", SDL_SCANCODE_RETURN);
    config_input.key_select = (SDL_Scancode)read_int("Input", "KeySelect", SDL_SCANCODE_SPACE);

    config_input.gamepad = read_bool("Input", "Gamepad", true);
    config_input.gamepad_directional = read_int("Input", "GamepadDirectional", 0);
    config_input.gamepad_invert_x_axis = read_bool("Input", "GamepadInvertX", false);
    config_input.gamepad_invert_y_axis = read_bool("Input", "GamepadInvertY", false);
    config_input.gamepad_a = read_int("Input", "GamepadA", SDL_CONTROLLER_BUTTON_B);
    config_input.gamepad_b = read_int("Input", "GamepadB", SDL_CONTROLLER_BUTTON_A);
    config_input.gamepad_start = read_int("Input", "GamepadStart", SDL_CONTROLLER_BUTTON_START);
    config_input.gamepad_select = read_int("Input", "GamepadSelect", SDL_CONTROLLER_BUTTON_BACK);
    config_input.gamepad_x_axis = read_int("Input", "GamepadX", SDL_CONTROLLER_AXIS_LEFTX);
    config_input.gamepad_y_axis = read_int("Input", "GamepadY", SDL_CONTROLLER_AXIS_LEFTY);

    Debug("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    if (config_emulator.ffwd)
        config_audio.sync = true;

    write_bool("Debug", "Debug", config_debug.debug);
    write_bool("Debug", "Audio", config_debug.show_audio);
    write_bool("Debug", "Disassembler", config_debug.show_disassembler);
    write_bool("Debug", "GameBoy", config_debug.show_gameboy);
    write_bool("Debug", "IOMap", config_debug.show_iomap);
    write_bool("Debug", "Memory", config_debug.show_memory);
    write_bool("Debug", "Processor", config_debug.show_processor);
    write_bool("Debug", "Video", config_debug.show_video);
    write_int("Debug", "FontSize", config_debug.font_size);
    write_bool("Debug", "MultiViewport", config_debug.multi_viewport);

    write_bool("Emulator", "Maximized", config_emulator.maximized);
    write_bool("Emulator", "FullScreen", config_emulator.fullscreen);
    write_bool("Emulator", "AlwaysShowMenu", config_emulator.always_show_menu);
    write_int("Emulator", "FFWD", config_emulator.ffwd_speed);
    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_bool("Emulator", "PauseWhenInactive", config_emulator.pause_when_inactive);
    write_bool("Emulator", "ForceDMG", config_emulator.force_dmg);
    write_bool("Emulator", "ForceGBA", config_emulator.force_gba);
    write_int("Emulator", "MBC", config_emulator.mbc);
    write_bool("Emulator", "DMGBootrom", config_emulator.dmg_bootrom);
    write_string("Emulator", "DMGBootromPath", config_emulator.dmg_bootrom_path);
    write_bool("Emulator", "GBCBootrom", config_emulator.gbc_bootrom);
    write_string("Emulator", "GBCBootromPath", config_emulator.gbc_bootrom_path);
    write_int("Emulator", "SaveFilesDirOption", config_emulator.savefiles_dir_option);
    write_string("Emulator", "SaveFilesPath", config_emulator.savefiles_path);
    write_int("Emulator", "SaveStatesDirOption", config_emulator.savestates_dir_option);
    write_string("Emulator", "SaveStatesPath", config_emulator.savestates_path);
    write_string("Emulator", "LastOpenPath", config_emulator.last_open_path);
    write_int("Emulator", "WindowWidth", config_emulator.window_width);
    write_int("Emulator", "WindowHeight", config_emulator.window_height);
    write_bool("Emulator", "StatusMessages", config_emulator.status_messages);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        write_string("Emulator", item.c_str(), config_emulator.recent_roms[i]);
    }

    write_int("Video", "Scale", config_video.scale);
    write_int("Video", "ScaleManual", config_video.scale_manual);
    write_int("Video", "AspectRatio", config_video.ratio);
    write_bool("Video", "FPS", config_video.fps);
    write_bool("Video", "Bilinear", config_video.bilinear);
    write_bool("Video", "MixFrames", config_video.mix_frames);
    write_float("Video", "MixFramesIntensity", config_video.mix_frames_intensity);
    write_bool("Video", "Matrix", config_video.matrix);
    write_float("Video", "MatrixIntensity", config_video.matrix_intensity);
    write_int("Video", "Palette", config_video.palette);
    for (int i = 0; i < config_max_custom_palettes; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            char pal_label_r[32];
            char pal_label_g[32];
            char pal_label_b[32];
            snprintf(pal_label_r, sizeof(pal_label_r), "CustomPalette%i%iR", i, c);
            snprintf(pal_label_g, sizeof(pal_label_g), "CustomPalette%i%iG", i, c);
            snprintf(pal_label_b, sizeof(pal_label_b), "CustomPalette%i%iB", i, c);
            write_int("Video", pal_label_r, config_video.color[i][c].red);
            write_int("Video", pal_label_g, config_video.color[i][c].green);
            write_int("Video", pal_label_b, config_video.color[i][c].blue);
        }
    }
    write_bool("Video", "Sync", config_video.sync);
    write_bool("Video", "ColorCorrection", config_video.color_correction);

    write_bool("Audio", "Enable", config_audio.enable);
    write_bool("Audio", "Sync", config_audio.sync);

    write_int("Input", "KeyLeft", config_input.key_left);
    write_int("Input", "KeyRight", config_input.key_right);
    write_int("Input", "KeyUp", config_input.key_up);
    write_int("Input", "KeyDown", config_input.key_down);
    write_int("Input", "KeyA", config_input.key_a);
    write_int("Input", "KeyB", config_input.key_b);
    write_int("Input", "KeyStart", config_input.key_start);
    write_int("Input", "KeySelect", config_input.key_select);

    write_bool("Input", "Gamepad", config_input.gamepad);
    write_int("Input", "GamepadDirectional", config_input.gamepad_directional);
    write_bool("Input", "GamepadInvertX", config_input.gamepad_invert_x_axis);
    write_bool("Input", "GamepadInvertY", config_input.gamepad_invert_y_axis);
    write_int("Input", "GamepadA", config_input.gamepad_a);
    write_int("Input", "GamepadB", config_input.gamepad_b);
    write_int("Input", "GamepadStart", config_input.gamepad_start);
    write_int("Input", "GamepadSelect", config_input.gamepad_select);
    write_int("Input", "GamepadX", config_input.gamepad_x_axis);
    write_int("Input", "GamepadY", config_input.gamepad_y_axis);

    if (config_ini_file->write(config_ini_data, true))
    {
        Debug("Settings saved");
    }
}

static bool check_portable(void)
{
    char* base_path;
    char portable_file_path[260];

    base_path = SDL_GetBasePath();

    strcpy(portable_file_path, base_path);
    strcat(portable_file_path, "portable.ini");

    FILE* file = fopen(portable_file_path, "r");

    if (IsValidPointer(file))
    {
        fclose(file);
        return true;
    }

    return false;
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = 0;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = std::stoi(value);

    Debug("Load setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static float read_float(const char* group, const char* key, float default_value)
{
    float ret = 0.0f;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = strtof(value.c_str(), NULL);

    Debug("Load setting: [%s][%s]=%.2f", group, key, ret);
    return ret;
}

static void write_float(const char* group, const char* key, float value)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << value;
    std::string value_str = oss.str();
    config_ini_data[group][key] = oss.str();
    Debug("Save float setting: [%s][%s]=%s", group, key, value_str.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        std::istringstream(value) >> std::boolalpha >> ret;

    Debug("Load setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string read_string(const char* group, const char* key)
{
    std::string ret = config_ini_data[group][key];
    Debug("Load setting: [%s][%s]=%s", group, key, ret.c_str());
    return ret;
}

static void write_string(const char* group, const char* key, std::string value)
{
    config_ini_data[group][key] = value;
    Debug("Save setting: [%s][%s]=%s", group, key, value.c_str());
}
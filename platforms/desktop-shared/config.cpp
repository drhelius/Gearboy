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
#include "../../src/gearboy.h"
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

#define CONFIG_IMPORT
#include "config.h"

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

    config_emulator.debug = read_bool("Emulator", "Debug", false);
    config_emulator.ffwd_speed = read_int("Emulator", "FFWD", 1);
    config_emulator.save_slot = read_int("Emulator", "SaveSlot", 0);
    config_emulator.start_paused = read_bool("Emulator", "StartPaused", false);
    config_emulator.force_dmg = read_bool("Emulator", "ForceDMG", false);
    config_emulator.save_in_rom_folder = read_bool("Emulator", "SaveInROMFolder", false);
    config_emulator.mbc = read_int("Emulator", "MBC", 0);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        config_emulator.recent_roms[i] = read_string("Emulator", item.c_str());
    }

    config_video.scale = read_int("Video", "Scale", 0);
    config_video.ratio = read_int("Video", "AspectRatio", 0);
    config_video.fps = read_bool("Video", "FPS", false);
    config_video.bilinear = read_bool("Video", "Bilinear", false);
    config_video.mix_frames = read_bool("Video", "MixFrames", true);
    config_video.mix_frames_intensity = read_float("Video", "MixFramesIntensity", 0.50f);
    config_video.matrix = read_bool("Video", "Matrix", true);
    config_video.matrix_intensity = read_float("Video", "MatrixIntensity", 0.30f);
    config_video.palette = read_int("Video", "Palette", 0);
    config_video.color[0].red = read_int("Video", "CustomPalette0R", 0xC4);
    config_video.color[0].green = read_int("Video", "CustomPalette0G", 0xF0);
    config_video.color[0].blue = read_int("Video", "CustomPalette0B", 0xC2);
    config_video.color[1].red = read_int("Video", "CustomPalette1R", 0x5A);
    config_video.color[1].green = read_int("Video", "CustomPalette1G", 0xB9);
    config_video.color[1].blue = read_int("Video", "CustomPalette1B", 0xA8);
    config_video.color[2].red = read_int("Video", "CustomPalette2R", 0x1E);
    config_video.color[2].green = read_int("Video", "CustomPalette2G", 0x60);
    config_video.color[2].blue = read_int("Video", "CustomPalette2B", 0x6E);
    config_video.color[3].red = read_int("Video", "CustomPalette3R", 0x2D);
    config_video.color[3].green = read_int("Video", "CustomPalette3G", 0x1B);
    config_video.color[3].blue = read_int("Video", "CustomPalette3B", 0x00);
    config_video.sync = read_bool("Video", "Sync", true);
    
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

    Log("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    write_bool("Emulator", "Debug", config_emulator.debug);
    write_int("Emulator", "FFWD", config_emulator.ffwd_speed);
    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_bool("Emulator", "ForceDMG", config_emulator.force_dmg);
    write_bool("Emulator", "SaveInROMFolder", config_emulator.save_in_rom_folder);
    write_int("Emulator", "MBC", config_emulator.mbc);

    for (int i = 0; i < config_max_recent_roms; i++)
    {
        std::string item = "RecentROM" + std::to_string(i);
        write_string("Emulator", item.c_str(), config_emulator.recent_roms[i]);
    }

    write_int("Video", "Scale", config_video.scale);
    write_int("Video", "AspectRatio", config_video.ratio);
    write_bool("Video", "FPS", config_video.fps);
    write_bool("Video", "Bilinear", config_video.bilinear);
    write_bool("Video", "MixFrames", config_video.mix_frames);
    write_float("Video", "MixFramesIntensity", config_video.mix_frames_intensity);
    write_bool("Video", "Matrix", config_video.matrix);
    write_float("Video", "MatrixIntensity", config_video.matrix_intensity);
    write_int("Video", "Palette", config_video.palette);
    write_int("Video", "CustomPalette0R", config_video.color[0].red);
    write_int("Video", "CustomPalette0G", config_video.color[0].green);
    write_int("Video", "CustomPalette0B", config_video.color[0].blue);
    write_int("Video", "CustomPalette1R", config_video.color[1].red);
    write_int("Video", "CustomPalette1G", config_video.color[1].green);
    write_int("Video", "CustomPalette1B", config_video.color[1].blue);
    write_int("Video", "CustomPalette2R", config_video.color[2].red);
    write_int("Video", "CustomPalette2G", config_video.color[2].green);
    write_int("Video", "CustomPalette2B", config_video.color[2].blue);
    write_int("Video", "CustomPalette3R", config_video.color[3].red);
    write_int("Video", "CustomPalette3G", config_video.color[3].green);
    write_int("Video", "CustomPalette3B", config_video.color[3].blue);
    write_bool("Video", "Sync", config_video.sync);

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
        Log("Settings saved");
    }
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = 0;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = std::stoi(value);

    Log("Load setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static float read_float(const char* group, const char* key, float default_value)
{
    float ret = 0.0f;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        ret = strtof(value.c_str(), NULL);

    Log("Load setting: [%s][%s]=%.2f", group, key, ret);
    return ret;
}

static void write_float(const char* group, const char* key, float value)
{
    std::string value_str = std::to_string(value);
    config_ini_data[group][key] = value_str;
    Log("Save setting: [%s][%s]=%s", group, key, value_str.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret;

    std::string value = config_ini_data[group][key];

    if(value.empty())
        ret = default_value;
    else
        std::istringstream(value) >> std::boolalpha >> ret;

    Log("Load setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string read_string(const char* group, const char* key)
{
    std::string ret = config_ini_data[group][key];
    Log("Load setting: [%s][%s]=%s", group, key, ret.c_str());
    return ret;
}

static void write_string(const char* group, const char* key, std::string value)
{
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}
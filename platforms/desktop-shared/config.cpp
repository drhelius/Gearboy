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

static int read_int(const char* group, const char* key);
static void write_int(const char* group, const char* key, int integer);
static bool read_bool(const char* group, const char* key);
static void write_bool(const char* group, const char* key, bool boolean);

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
    SafeDeleteArray(config_root_path);
}

void config_read(void)
{
    if (config_ini_file->read(config_ini_data))
    {
        Log("Loading settings from %s", config_emu_file_path);

        config_emulator.save_slot = read_int("Emulator", "SaveSlot");
        config_emulator.start_paused = read_bool("Emulator", "StartPaused");
        config_emulator.force_dmg = read_bool("Emulator", "ForceDMG");
        config_emulator.save_in_rom_folder = read_bool("Emulator", "SaveInROMFolder");

        config_video.fps = read_bool("Video", "FPS");
        config_video.bilinear = read_bool("Video", "Bilinear");
        config_video.mix_frames = read_bool("Video", "MixFrames");
        config_video.matrix = read_bool("Video", "Matrix");

        config_audio.enable = read_bool("Audio", "Enable");
        config_audio.sync = read_bool("Audio", "Sync");

        config_input.key_left = (SDL_Scancode)read_int("Input", "KeyLeft");
        config_input.key_right = (SDL_Scancode)read_int("Input", "KeyRight");
        config_input.key_up = (SDL_Scancode)read_int("Input", "KeyUp");
        config_input.key_down = (SDL_Scancode)read_int("Input", "KeyDown");
        config_input.key_a = (SDL_Scancode)read_int("Input", "KeyA");
        config_input.key_b = (SDL_Scancode)read_int("Input", "KeyB");
        config_input.key_start = (SDL_Scancode)read_int("Input", "KeyStart");
        config_input.key_select = (SDL_Scancode)read_int("Input", "KeySelect");

        config_input.gamepad = read_bool("Input", "Gamepad");
        config_input.gamepad_invert_x_axis = read_bool("Input", "GamepadInvertX");
        config_input.gamepad_invert_y_axis = read_bool("Input", "GamepadInvertY");
        config_input.gamepad_a = (SDL_GameControllerButton)read_int("Input", "GamepadA");
        config_input.gamepad_b = (SDL_GameControllerButton)read_int("Input", "GamepadB");
        config_input.gamepad_start = (SDL_GameControllerButton)read_int("Input", "GamepadStart");
        config_input.gamepad_select = (SDL_GameControllerButton)read_int("Input", "GamepadSelect");
        config_input.gamepad_x_axis = read_int("Input", "GamepadX");
        config_input.gamepad_y_axis = read_int("Input", "GamepadY");

        Log("Settings loaded");
    }
    else
    {
        Log("Unable to load settings from %s", config_emu_file_path);
    }
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    write_int("Emulator", "SaveSlot", config_emulator.save_slot);
    write_bool("Emulator", "StartPaused", config_emulator.start_paused);
    write_bool("Emulator", "ForceDMG", config_emulator.force_dmg);
    write_bool("Emulator", "SaveInROMFolder", config_emulator.save_in_rom_folder);

    write_bool("Video", "FPS", config_video.fps);
    write_bool("Video", "Bilinear", config_video.bilinear);
    write_bool("Video", "MixFrames", config_video.mix_frames);
    write_bool("Video", "Matrix", config_video.matrix);

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

static int read_int(const char* group, const char* key)
{
    int ret = std::stoi(config_ini_data[group][key]);
    Log("Load setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Log("Save setting: [%s][%s]=%s", group, key, value.c_str());
}

static bool read_bool(const char* group, const char* key)
{
    bool ret;
    std::istringstream(config_ini_data[group][key]) >> std::boolalpha >> ret;
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
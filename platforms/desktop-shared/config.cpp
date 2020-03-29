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

        config_emulator_options.save_slot = std::stoi(config_ini_data["Emulator"]["SaveSlot"]);
        std::istringstream(config_ini_data["Emulator"]["StartPaused"]) >> std::boolalpha >> config_emulator_options.start_paused;
        std::istringstream(config_ini_data["Emulator"]["ForceDMG"]) >> std::boolalpha >> config_emulator_options.force_dmg;
        std::istringstream(config_ini_data["Emulator"]["SaveInROMFolder"]) >> std::boolalpha >> config_emulator_options.save_in_rom_folder;

        std::istringstream(config_ini_data["Video"]["FPS"]) >> std::boolalpha >> config_video_options.fps;
        std::istringstream(config_ini_data["Video"]["Bilinear"]) >> std::boolalpha >> config_video_options.bilinear;
        std::istringstream(config_ini_data["Video"]["MixFrames"]) >> std::boolalpha >> config_video_options.mix_frames;
        std::istringstream(config_ini_data["Video"]["Matrix"]) >> std::boolalpha >> config_video_options.matrix;

        std::istringstream(config_ini_data["Audio"]["Enable"]) >> std::boolalpha >> config_audio_options.enable;
        std::istringstream(config_ini_data["Audio"]["Sync"]) >> std::boolalpha >> config_audio_options.sync;

        std::istringstream(config_ini_data["Input"]["Gamepad"]) >> std::boolalpha >> config_input_options.gamepad;

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

    std::stringstream converter;

    config_ini_data["Emulator"]["SaveSlot"] = std::to_string(config_emulator_options.save_slot);

    converter << std::boolalpha << config_emulator_options.start_paused;
    config_ini_data["Emulator"]["StartPaused"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_emulator_options.force_dmg;
    config_ini_data["Emulator"]["ForceDMG"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_emulator_options.save_in_rom_folder;
    config_ini_data["Emulator"]["SaveInROMFolder"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_video_options.fps;
    config_ini_data["Video"]["FPS"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_video_options.bilinear;
    config_ini_data["Video"]["Bilinear"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_video_options.mix_frames;
    config_ini_data["Video"]["MixFrames"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_video_options.matrix;
    config_ini_data["Video"]["Matrix"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_audio_options.enable;
    config_ini_data["Audio"]["Enable"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_audio_options.sync;
    config_ini_data["Audio"]["Sync"] = converter.str();
    converter.str("");

    converter << std::boolalpha << config_input_options.gamepad;
    config_ini_data["Input"]["Gamepad"] = converter.str();
    converter.str("");

    if (config_ini_file->write(config_ini_data, true))
    {
        Log("Settings saved");
    }
}
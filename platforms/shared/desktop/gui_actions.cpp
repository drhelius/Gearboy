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

#define GUI_ACTIONS_IMPORT
#include "gui_actions.h"
#include "gui.h"
#include "gui_debug_trace_logger.h"
#include "config.h"
#include "emu.h"
#include "gearboy.h"
#include "application.h"
#include "display.h"

void gui_action_reset(void)
{
    gui_set_status_message("Resetting...", 3000);
    gui_debug_trace_logger_clear();

    emu_resume();
    emu_reset(config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);

    if (config_emulator.start_paused)
    {
        emu_pause();

        for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        {
            { emu_frame_buffer[i].red = 0; emu_frame_buffer[i].green = 0; emu_frame_buffer[i].blue = 0; }
        }
    }
}

void gui_action_reload_rom(void)
{
    if (!emu_is_empty())
    {
        char rom_path[4096];
        strcpy(rom_path, emu_get_core()->GetCartridge()->GetFilePath());
        gui_load_rom(rom_path);
    }
}

void gui_action_pause(void)
{
    if (emu_is_paused())
    {
        gui_set_status_message("Resumed", 3000);
        emu_resume();
    }
    else
    {
        gui_set_status_message("Paused", 3000);
        emu_pause();
    }
}

void gui_action_ffwd(void)
{
    config_audio.sync = !config_emulator.ffwd;

    if (config_emulator.ffwd)
    {
        gui_set_status_message("Fast Forward ON", 3000);
        display_set_vsync(false);
    }
    else
    {
        gui_set_status_message("Fast Forward OFF", 3000);
        display_set_vsync(config_video.sync);
        emu_audio_reset();
    }
}

void gui_action_save_screenshot(const char* path)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    time_t now = time(0);
    tm* ltm = localtime(&now);

    string date_time = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + to_string(ltm->tm_min) + to_string(ltm->tm_sec);

    string file_path;

    if (path != NULL)
    {
        file_path = path;
        if (file_path.find_last_of(".") == string::npos)
            file_path += ".png";
    }
    else
    {
        switch ((Directory_Location)config_emulator.screenshots_dir_option)
        {
            default:
            case Directory_Location_Default:
            {
                file_path = file_path.assign(config_root_path)+ "/" + string(emu_get_core()->GetCartridge()->GetFileName()) + " - " + date_time + ".png";
                break;
            }
            case Directory_Location_ROM:
            {
                file_path = file_path.assign(emu_get_core()->GetCartridge()->GetFilePath()) + " - " + date_time + ".png";
                break;
            }
            case Directory_Location_Custom:
            {
                file_path = file_path.assign(config_emulator.screenshots_path)+ "/" + string(emu_get_core()->GetCartridge()->GetFileName()) + " - " + date_time + ".png";
                break;
            }
        }
    }

    emu_save_screenshot(file_path.c_str());

    string message = "Screenshot saved to " + file_path;
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_sprite(const char* path, int index)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    emu_save_sprite(path, index);

    string message = "Sprite saved to " + string(path);
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_all_sprites(const char* folder_path)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    for (int i = 0; i < 40; i++)
    {
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/sprite_id%02d.png", folder_path, i);
        emu_save_sprite(file_path, i);
    }

    string message = "All sprites saved to " + string(folder_path);
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_background(const char* path)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    emu_save_background(path);

    string message = "Background saved to " + string(path);
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_tiles(const char* path)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    emu_save_tiles(path);

    string message = "Pattern table saved to " + string(path);
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_sgb_border(const char* path)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    emu_save_sgb_border(path);

    string message = "SGB border saved to " + string(path);
    gui_set_status_message(message.c_str(), 3000);
}

void gui_action_save_sgb_tiles(const char* path, int palette)
{
    using namespace std;

    if (!emu_get_core()->GetCartridge()->IsLoadedROM())
        return;

    emu_save_sgb_tiles(path, palette);

    string message = "SGB tiles saved to " + string(path);
    gui_set_status_message(message.c_str(), 3000);
}
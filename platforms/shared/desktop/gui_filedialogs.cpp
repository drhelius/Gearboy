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

#define GUI_FILEDIALOGS_IMPORT
#include "gui_filedialogs.h"

#include <SDL3/SDL.h>
#include <string>
#include <cstring>
#include "gui.h"
#include "gui_actions.h"
// #include "gui_debug_memory.h"
// #include "gui_debug_disassembler.h"
// #include "gui_debug_trace_logger.h"
// #include "gui_debug.h"
#include "gui_menus.h"
#include "application.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

enum FileDialogID
{
    FileDialog_None = 0,
    FileDialog_OpenROM,
    FileDialog_LoadRAM,
    FileDialog_SaveRAM,
    FileDialog_LoadState,
    FileDialog_SaveState,
    FileDialog_ChooseSavestatePath,
    FileDialog_ChooseScreenshotPath,
    FileDialog_ChooseSavesPath,
    FileDialog_LoadSymbols,
    FileDialog_SaveScreenshot,
    FileDialog_SaveVGM,
    FileDialog_SaveSprite,
    FileDialog_SaveAllSprites,
    FileDialog_SaveBackground,
    FileDialog_SaveTiles,
    FileDialog_SaveMemoryDumpBinary,
    FileDialog_SaveMemoryDumpText,
    FileDialog_SaveDisassemblerFull,
    FileDialog_SaveDisassemblerVisible,
    FileDialog_SaveLog,
    FileDialog_SaveDebugSettings,
    FileDialog_LoadDebugSettings,
    FileDialog_LoadDmgBootrom,
    FileDialog_LoadGbcBootrom,
};

static FileDialogID pending_dialog_id = FileDialog_None;
static std::string pending_dialog_path;
static bool dialog_active = false;
static bool pending_refocus_window = false;
static int pending_dialog_int_param1 = 0;
#if !defined(__APPLE__)
static bool was_exclusive_fullscreen = false;
#endif

static void SDLCALL file_dialog_callback(void* userdata, const char* const* filelist, int filter);
static void process_dialog_result(FileDialogID id, const char* path);

static bool begin_dialog(void)
{
    if (dialog_active)
        return false;
    dialog_active = true;

#if !defined(__APPLE__)
    if (config_emulator.fullscreen && config_emulator.fullscreen_mode == 1)
    {
        was_exclusive_fullscreen = true;
        application_trigger_fullscreen(false);
    }
#endif

    return true;
}

void gui_file_dialog_open_rom(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "ROM Files", "gb;gbc;cgb;sgb;dmg;rom;bin;zip" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_OpenROM, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_load_ram(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "RAM Files", "sav" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadRAM, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_save_ram(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "RAM Files", "sav" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveRAM, application_sdl_window, filters, 1, default_path);
}

void gui_file_dialog_load_state(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Save State Files", "state;state1;state2;state3;state4;state5" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadState, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_save_state(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Save State Files", "state" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveState, application_sdl_window, filters, 1, default_path);
}

void gui_file_dialog_choose_savestate_path(void)
{
    if (!begin_dialog())
        return;

    const char* default_path = config_emulator.savestates_path.empty() ? NULL : config_emulator.savestates_path.c_str();
    SDL_ShowOpenFolderDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_ChooseSavestatePath, application_sdl_window, default_path, false);
}

void gui_file_dialog_choose_screenshot_path(void)
{
    if (!begin_dialog())
        return;

    const char* default_path = config_emulator.screenshots_path.empty() ? NULL : config_emulator.screenshots_path.c_str();
    SDL_ShowOpenFolderDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_ChooseScreenshotPath, application_sdl_window, default_path, false);
}

void gui_file_dialog_load_symbols(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Symbol Files", "sym;noi" }, { "All Files", "*" } };
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadSymbols, application_sdl_window, filters, 2, NULL, false);
}

void gui_file_dialog_save_screenshot(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "PNG Files", "png" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveScreenshot, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_vgm(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "VGM Files", "vgm" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveVGM, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_sprite(int index)
{
    if (!begin_dialog())
        return;

    pending_dialog_int_param1 = index;
    SDL_DialogFileFilter filters[] = { { "PNG Files", "png" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveSprite, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_all_sprites(void)
{
    if (!begin_dialog())
        return;

    SDL_ShowOpenFolderDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveAllSprites, application_sdl_window, NULL, false);
}

void gui_file_dialog_save_background(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "PNG Files", "png" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveBackground, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_tiles(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "PNG Files", "png" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveTiles, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_memory_dump(bool binary)
{
    if (!begin_dialog())
        return;

    FileDialogID id = binary ? FileDialog_SaveMemoryDumpBinary : FileDialog_SaveMemoryDumpText;
    SDL_DialogFileFilter filters[] = { { "Memory Dump Files", binary ? "bin" : "txt" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)id, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_disassembler(bool full)
{
    if (!begin_dialog())
        return;

    FileDialogID id = full ? FileDialog_SaveDisassemblerFull : FileDialog_SaveDisassemblerVisible;
    SDL_DialogFileFilter filters[] = { { "Disassembler Files", "txt" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)id, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_log(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Log Files", "txt" } };
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveLog, application_sdl_window, filters, 1, NULL);
}

void gui_file_dialog_save_debug_settings(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Debug Settings Files", "gbdebug" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowSaveFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_SaveDebugSettings, application_sdl_window, filters, 1, default_path);
}

void gui_file_dialog_load_debug_settings(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "Debug Settings Files", "gbdebug" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadDebugSettings, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_choose_saves_path(void)
{
    if (!begin_dialog())
        return;

    const char* default_path = config_emulator.savefiles_path.empty() ? NULL : config_emulator.savefiles_path.c_str();
    SDL_ShowOpenFolderDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_ChooseSavesPath, application_sdl_window, default_path, false);
}

void gui_file_dialog_load_dmg_bootrom(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "BIOS Files", "bin;rom;bios;gb" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadDmgBootrom, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_load_gbc_bootrom(void)
{
    if (!begin_dialog())
        return;

    SDL_DialogFileFilter filters[] = { { "BIOS Files", "bin;rom;bios;gbc" } };
    const char* default_path = config_emulator.last_open_path.empty() ? NULL : config_emulator.last_open_path.c_str();
    SDL_ShowOpenFileDialog(file_dialog_callback, (void*)(intptr_t)FileDialog_LoadGbcBootrom, application_sdl_window, filters, 1, default_path, false);
}

void gui_file_dialog_process_results(void)
{
#if !defined(__APPLE__)
    if (was_exclusive_fullscreen && !dialog_active)
    {
        was_exclusive_fullscreen = false;
        application_trigger_fullscreen(true);
    }
#endif

    if (pending_refocus_window && !dialog_active)
    {
        pending_refocus_window = false;
        SDL_RaiseWindow(application_sdl_window);
    }

    if (pending_dialog_id != FileDialog_None)
    {
        FileDialogID id = pending_dialog_id;
        std::string path = pending_dialog_path;
        pending_dialog_id = FileDialog_None;
        pending_dialog_path.clear();
        process_dialog_result(id, path.c_str());
    }
}

bool gui_file_dialog_is_active(void)
{
    return dialog_active;
}

static void SDLCALL file_dialog_callback(void* userdata, const char* const* filelist, int filter)
{
    (void)filter;
    dialog_active = false;

    FileDialogID id = (FileDialogID)(intptr_t)userdata;

    if (!filelist || !filelist[0])
    {
        pending_refocus_window = true;
        return;
    }

    pending_dialog_id = id;
    pending_dialog_path = filelist[0];
}

static void process_dialog_result(FileDialogID id, const char* path)
{
    switch (id)
    {
        case FileDialog_OpenROM:
        {
            std::string str_path = path;
            std::string::size_type pos = str_path.find_last_of("\\/");
            config_emulator.last_open_path.assign(str_path.substr(0, pos + 1));
            gui_load_rom(path);
            break;
        }
        case FileDialog_LoadRAM:
        {
            emu_load_ram(path, config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);
            break;
        }
        case FileDialog_SaveRAM:
        {
            emu_save_ram(path);
            break;
        }
        case FileDialog_LoadState:
        {
            std::string message("Loading state from ");
            message += path;
            gui_set_status_message(message.c_str(), 3000);
            emu_load_state_file(path);
            break;
        }
        case FileDialog_SaveState:
        {
            std::string message("Saving state to ");
            message += path;
            gui_set_status_message(message.c_str(), 3000);
            emu_save_state_file(path);
            break;
        }
        case FileDialog_ChooseSavestatePath:
        {
            strncpy_fit(gui_savestates_path, path, sizeof(gui_savestates_path));
            config_emulator.savestates_path.assign(path);
            update_savestates_data();
            break;
        }
        case FileDialog_ChooseScreenshotPath:
        {
            strncpy_fit(gui_screenshots_path, path, sizeof(gui_screenshots_path));
            config_emulator.screenshots_path.assign(path);
            break;
        }
        case FileDialog_LoadSymbols:
        {
            // gui_debug_reset_symbols();
            // gui_debug_load_symbols_file(path);
            break;
        }
        case FileDialog_SaveScreenshot:
        {
            gui_action_save_screenshot(path);
            break;
        }
        case FileDialog_SaveVGM:
        {
            emu_start_vgm_recording(path);
            gui_set_status_message("VGM recording started", 3000);
            break;
        }
        case FileDialog_SaveSprite:
        {
            gui_action_save_sprite(path, pending_dialog_int_param1);
            break;
        }
        case FileDialog_SaveAllSprites:
        {
            gui_action_save_all_sprites(path);
            break;
        }
        case FileDialog_SaveBackground:
        {
            gui_action_save_background(path);
            break;
        }
        case FileDialog_SaveTiles:
        {
            gui_action_save_tiles(path);
            break;
        }
        case FileDialog_SaveMemoryDumpBinary:
        {
            // gui_debug_memory_save_dump(path, true);
            break;
        }
        case FileDialog_SaveMemoryDumpText:
        {
            // gui_debug_memory_save_dump(path, false);
            break;
        }
        case FileDialog_SaveDisassemblerFull:
        {
            // gui_debug_save_disassembler(path, true);
            break;
        }
        case FileDialog_SaveDisassemblerVisible:
        {
            // gui_debug_save_disassembler(path, false);
            break;
        }
        case FileDialog_SaveLog:
        {
            // gui_debug_save_log(path);
            break;
        }
        case FileDialog_SaveDebugSettings:
        {
            // gui_debug_save_settings(path);
            gui_set_status_message("Debug settings saved", 3000);
            break;
        }
        case FileDialog_LoadDebugSettings:
        {
            // gui_debug_load_settings(path);
            gui_set_status_message("Debug settings loaded", 3000);
            break;
        }
        case FileDialog_ChooseSavesPath:
        {
            strncpy_fit(gui_savefiles_path, path, sizeof(gui_savefiles_path));
            config_emulator.savefiles_path.assign(path);
            break;
        }
        case FileDialog_LoadDmgBootrom:
        {
            strncpy_fit(gui_dmg_bootrom_path, path, sizeof(gui_dmg_bootrom_path));
            config_emulator.dmg_bootrom_path.assign(path);
            emu_load_bootrom_dmg(gui_dmg_bootrom_path);
            break;
        }
        case FileDialog_LoadGbcBootrom:
        {
            strncpy_fit(gui_gbc_bootrom_path, path, sizeof(gui_gbc_bootrom_path));
            config_emulator.gbc_bootrom_path.assign(path);
            emu_load_bootrom_gbc(gui_gbc_bootrom_path);
            break;
        }
        default:
            break;
    }
}

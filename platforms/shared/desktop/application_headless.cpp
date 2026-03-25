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

#include <SDL3/SDL.h>
#include <signal.h>
#include "application_headless.h"
#include "config.h"
#include "emu.h"
#include "gui.h"
#include "gui_debug.h"
#include "gui_debug_disassembler.h"
#include "log.h"

static volatile bool headless_running = true;

static void headless_signal_handler(int sig)
{
    (void)sig;
    headless_running = false;
}

int application_headless_init(const char* rom_file, const char* symbol_file, int mcp_mode, int mcp_tcp_port)
{
    Log("\n%s", GEARBOY_TITLE_ASCII);
    Log("%s %s Headless Mode", GEARBOY_TITLE, GEARBOY_VERSION);

    if (mcp_mode < 0)
    {
        Error("Headless mode requires --mcp-stdio or --mcp-http");
        return 1;
    }

    if (!SDL_Init(0))
    {
        Error("Failed to initialize SDL (headless)");
        return 1;
    }

    if (!emu_init())
    {
        Error("Failed to initialize emulator");
        return 2;
    }

    config_debug.debug = true;

    emu_audio_volume(0.0f);

    if (!config_emulator.dmg_bootrom_path.empty())
    {
        Log("Loading DMG bootrom: %s", config_emulator.dmg_bootrom_path.c_str());
        emu_load_bootrom_dmg(config_emulator.dmg_bootrom_path.c_str());
    }
    if (!config_emulator.gbc_bootrom_path.empty())
    {
        Log("Loading GBC bootrom: %s", config_emulator.gbc_bootrom_path.c_str());
        emu_load_bootrom_gbc(config_emulator.gbc_bootrom_path.c_str());
    }
    emu_enable_bootrom_dmg(config_emulator.dmg_bootrom);
    emu_enable_bootrom_gbc(config_emulator.gbc_bootrom);
    emu_color_correction(config_video.color_correction);

    gui_debug_init();

    if (IsValidPointer(rom_file) && (strlen(rom_file) > 0))
    {
        Log("Rom file argument: %s", rom_file);
        gui_load_rom(rom_file);
    }

    if (IsValidPointer(symbol_file) && (strlen(symbol_file) > 0))
    {
        Log("Symbol file argument: %s", symbol_file);
        gui_debug_reset_symbols();
        gui_debug_load_symbols_file(symbol_file);
    }

    Log("Starting MCP server (mode: %s, port: %d)...", mcp_mode == 0 ? "stdio" : "http", mcp_tcp_port);
    emu_mcp_set_transport(mcp_mode, mcp_tcp_port);
    emu_mcp_start();

    signal(SIGINT, headless_signal_handler);
    signal(SIGTERM, headless_signal_handler);

    return 0;
}

void application_headless_destroy(void)
{
    gui_debug_destroy();
    emu_destroy();
    SDL_Quit();
}

void application_headless_mainloop(void)
{
    Log("Running headless main loop...");

    while (headless_running)
    {
        Uint64 frame_start = SDL_GetPerformanceCounter();

        emu_update();

        if (!emu_mcp_is_running())
        {
            Log("MCP server stopped, exiting headless mode");
            break;
        }

        Uint64 frame_end = SDL_GetPerformanceCounter();
        float elapsed_ms = (float)(frame_end - frame_start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

        float target_ms = 16.666f;

        if (elapsed_ms < target_ms)
            SDL_Delay((Uint32)(target_ms - elapsed_ms));
    }
}

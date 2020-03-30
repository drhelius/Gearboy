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

#include "../../src/gearboy.h"
#include "../audio-shared/Sound_Queue.h"

#define EMU_IMPORT
#include "emu.h"

static GearboyCore* gearboy;
static Sound_Queue* sound_queue;
static bool save_files_in_rom_dir = false;
static s16* audio_buffer;
static char base_save_path[260];
static bool audio_enabled;

static void save_ram(void);
static void load_ram(void);

void emu_init(const char* save_path)
{
    strcpy(base_save_path, save_path);

    emu_frame_buffer = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        emu_frame_buffer[i] = 0;

    gearboy = new GearboyCore();
    gearboy->Init();

    sound_queue = new Sound_Queue();
    sound_queue->start(44100, 2);

    audio_buffer = new s16[AUDIO_BUFFER_SIZE];

    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;

    audio_enabled = true;
    emu_audio_sync = true;
}

void emu_destroy(void)
{
    save_ram();
    SafeDeleteArray(audio_buffer);
    SafeDelete(sound_queue);
    SafeDelete(gearboy);
    SafeDeleteArray(emu_frame_buffer);
}

void emu_load_rom(const char* file_path, bool force_dmg, bool save_in_rom_dir)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearboy->LoadROM(file_path, force_dmg);
    load_ram();
}

void emu_run_to_vblank(void)
{
    if (!emu_is_empty())
    {
        int sampleCount = 0;

        gearboy->RunToVBlank(emu_frame_buffer, audio_buffer, &sampleCount);

        if (audio_enabled && (sampleCount > 0))
        {
            sound_queue->write(audio_buffer, sampleCount, emu_audio_sync);
        }
    }
}

void emu_key_pressed(Gameboy_Keys key)
{
    gearboy->KeyPressed(key);
}

void emu_key_released(Gameboy_Keys key)
{
    gearboy->KeyReleased(key);
}

void emu_pause(void)
{
    gearboy->Pause(true);
    audio_enabled = false;
}

void emu_resume(void)
{
    gearboy->Pause(false);
    audio_enabled = true;
}

bool emu_is_paused(void)
{
    return gearboy->IsPaused();
}

bool emu_is_empty(void)
{
    return !gearboy->GetCartridge()->IsLoadedROM();
}

void emu_reset(bool force_dmg, bool save_in_rom_dir)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearboy->ResetROM(force_dmg);
    load_ram();
}

void emu_memory_dump(void)
{
    gearboy->GetMemory()->MemoryDump("memdump.txt");
}

void emu_audio_settings(bool enabled, int rate)
{
    audio_enabled = enabled && !gearboy->IsPaused();
    gearboy->SetSoundSampleRate(rate);
    sound_queue->stop();
    sound_queue->start(rate, 2);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

void emu_dmg_palette(GB_Color& color1, GB_Color& color2, GB_Color& color3, GB_Color& color4)
{
    gearboy->SetDMGPalette(color1, color2, color3, color4);
}

bool emu_is_cgb(void)
{
    return gearboy->GetCartridge()->IsCGB();
}

void emu_save_ram(const char* file_path)
{
    if (!emu_is_empty())
        gearboy->SaveRam(file_path, true);
}

void emu_load_ram(const char* file_path, bool force_dmg, bool save_in_rom_dir)
{
    if (!emu_is_empty())
    {
        save_files_in_rom_dir = save_in_rom_dir;
        save_ram();
        gearboy->ResetROM(force_dmg);
        gearboy->LoadRam(file_path, true);
    }
}

void emu_save_state_slot(int index)
{
    if (!emu_is_empty())
        gearboy->SaveState(index);
}

void emu_load_state_slot(int index)
{
    if (!emu_is_empty())
        gearboy->LoadState(index);
}

void emu_save_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearboy->SaveState(file_path, -1);
}

void emu_load_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearboy->LoadState(file_path, -1);
}

static void save_ram(void)
{
    if (save_files_in_rom_dir)
        gearboy->SaveRam();
    else
        gearboy->SaveRam(base_save_path);
}

static void load_ram(void)
{
    if (save_files_in_rom_dir)
        gearboy->LoadRam();
    else
        gearboy->LoadRam(base_save_path);
}


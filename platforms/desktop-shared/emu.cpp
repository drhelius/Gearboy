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

// red, green, blue
static GB_Color original_palette[4] = {{0x87, 0x96, 0x03},{0x4D, 0x6B, 0x03},{0x2B, 0x55, 0x03},{0x14, 0x44, 0x03}};
static GB_Color sharp_palette[4] = {{0xF5, 0xFA, 0xEF},{0x86, 0xC2, 0x70},{0x2F, 0x69, 0x57},{0x0B, 0x19, 0x20}};
static GB_Color bw_palette[4] = {{0xFF, 0xFF, 0xFF},{0xAA, 0xAA, 0xAA},{0x55, 0x55, 0x55},{0x00, 0x00, 0x00}};
static GB_Color autumn_palette[4] = {{0xFF, 0xF6, 0xD3},{0xF9, 0xA8, 0x75},{0xEB, 0x6B, 0x6F},{0x7C, 0x3F, 0x58}};
static GB_Color soft_palette[4] = {{0xE0, 0xE0, 0xAA},{0xB0, 0xB8, 0x7C},{0x72, 0x82, 0x5B},{0x39, 0x34, 0x17}};
static GB_Color slime_palette[4] = {{0xD4, 0xEB, 0xA5},{0x62, 0xB8, 0x7C},{0x27, 0x76, 0x5D},{0x1D, 0x39, 0x39}};

static GearboyCore* gearboy;
static Sound_Queue* sound_queue;
static bool save_files_in_rom_dir = false;
static u16* frame_buffer_565;
static s16* audio_buffer;
static char base_save_path[260];
static bool audio_enabled;

static void save_ram(void);
static void load_ram(void);
static void generate_24bit_buffer(void);
static const char* get_mbc(Cartridge::CartridgeTypes type);

void emu_init(const char* save_path)
{
    strcpy(base_save_path, save_path);

    frame_buffer_565 = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    emu_frame_buffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
    {
        emu_frame_buffer[i].red = 0;
        emu_frame_buffer[i].green = 0;
        emu_frame_buffer[i].blue = 0;
        frame_buffer_565[i] = 0;
    }

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

void emu_load_rom(const char* file_path, bool force_dmg, bool save_in_rom_dir, Cartridge::CartridgeTypes mbc)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearboy->LoadROM(file_path, force_dmg, mbc);
    load_ram();
}

void emu_run_to_vblank(void)
{
    if (!emu_is_empty())
    {
        int sampleCount = 0;

        gearboy->RunToVBlank(frame_buffer_565, audio_buffer, &sampleCount);

        generate_24bit_buffer();

        if ((sampleCount > 0) && !gearboy->IsPaused())
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
}

void emu_resume(void)
{
    gearboy->Pause(false);
}

bool emu_is_paused(void)
{
    return gearboy->IsPaused();
}

bool emu_is_empty(void)
{
    return !gearboy->GetCartridge()->IsLoadedROM();
}

void emu_reset(bool force_dmg, bool save_in_rom_dir, Cartridge::CartridgeTypes mbc)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearboy->ResetROM(force_dmg, mbc);
    load_ram();
}

void emu_memory_dump(void)
{
    gearboy->GetMemory()->MemoryDump("memdump.txt");
}

void emu_audio_volume(float volume)
{
    audio_enabled = (volume > 0.0f);
    gearboy->SetSoundVolume(volume);
}

void emu_audio_reset(void)
{
    sound_queue->stop();
    sound_queue->start(44100, 2);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

void emu_dmg_palette(GB_Color& color1, GB_Color& color2, GB_Color& color3, GB_Color& color4)
{
    gearboy->SetDMGPalette(color1, color2, color3, color4);
}

void emu_dmg_predefined_palette(int palette)
{
    GB_Color* predefined;

    switch (palette)
    {
        case 0:
            predefined = original_palette;
            break;
        case 1:
            predefined = sharp_palette;
            break;
        case 2:
            predefined = bw_palette;
            break;
        case 3:
            predefined = autumn_palette;
            break;
        case 4:
            predefined = soft_palette;
            break;
        case 5:
            predefined = slime_palette;
            break;
        default:
            predefined = NULL;
    }

    if (predefined)
    {
        gearboy->SetDMGPalette(predefined[0], predefined[1], predefined[2], predefined[3]);
    }
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

void emu_load_ram(const char* file_path, bool force_dmg, bool save_in_rom_dir, Cartridge::CartridgeTypes mbc)
{
    if (!emu_is_empty())
    {
        save_files_in_rom_dir = save_in_rom_dir;
        save_ram();
        gearboy->ResetROM(force_dmg, mbc);
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

void emu_add_cheat(const char* cheat)
{
    gearboy->SetCheat(cheat);
}

void emu_clear_cheats()
{
    gearboy->ClearCheats();
}

void emu_get_info(char* info)
{
    if (!emu_is_empty())
    {
        Cartridge* cart = gearboy->GetCartridge();

        const char* filename = cart->GetFileName();
        const char* gbc = cart->IsCGB() ? "YES" : "NO";
        const char* sgb = cart->IsSGB() ? "YES" : "NO";
        const char* battery = cart->HasBattery() ? "YES" : "NO";
        const char* rtc = cart->IsRTCPresent() ? "YES" : "NO";
        const char* rumble = cart->IsRumblePresent() ? "YES" : "NO";
        const char* name = cart->GetName();
        int version = cart->GetVersion();
        int rom_banks = cart->GetROMBankCount();
        int ram_banks = cart->GetRAMBankCount();

        const char* mbc = get_mbc(cart->GetType());

        sprintf(info, "File Name: %s\nMBC: %s\nGame Boy Color: %s\nSuper Game Boy: %s\nCartridge Name: %s\nCartridge Version: %d\nROM Banks: %d\nRAM Banks: %d\nBattery: %s\nReal Time Clock: %s\nRumble: %s\n", filename, mbc, gbc, sgb, name, version, rom_banks, ram_banks, battery, rtc, rumble);
    }
    else
    {
        sprintf(info, "No data!");
    }
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

static void generate_24bit_buffer(void)
{
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
    {
        emu_frame_buffer[i].red = (((frame_buffer_565[i] >> 11) & 0x1F ) * 255 + 15) / 31;
        emu_frame_buffer[i].green = (((frame_buffer_565[i] >> 5) & 0x3F ) * 255 + 31) / 63;
        emu_frame_buffer[i].blue = ((frame_buffer_565[i] & 0x1F ) * 255 + 15) / 31;
    }
}

static const char* get_mbc(Cartridge::CartridgeTypes type)
{
    switch (type)
    {
    case Cartridge::CartridgeNoMBC:
        return "ROM Only";
        break;
    case Cartridge::CartridgeMBC1:
        return "MBC 1";
        break;
    case Cartridge::CartridgeMBC1Multi:
        return "MBC 1 Multi 64";
        break;
    case Cartridge::CartridgeMBC2:
        return "MBC 2";
        break;
    case Cartridge::CartridgeMBC3:
        return "MBC 3";
        break;
    case Cartridge::CartridgeMBC5:
        return "MBC 5";
        break;
    case Cartridge::CartridgeNotSupported:
        return "Not Supported";
        break;
    default:
        return "Undefined";
        break;
    }
}
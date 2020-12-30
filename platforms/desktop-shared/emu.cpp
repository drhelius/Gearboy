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
static u16* debug_background_buffer_565;
static u16* debug_tile_buffers_565[2];
static u16* debug_oam_buffers_565[40];
static s16* audio_buffer;
static char base_save_path[260];
static bool audio_enabled;
static bool debugging = false;
static bool debug_step = false;
static bool debug_next_frame = false;
static bool color_correction = false;

static void save_ram(void);
static void load_ram(void);
static void generate_24bit_buffer(GB_Color* dest, u16* src, int size);
static const char* get_mbc(Cartridge::CartridgeTypes type);
static void init_debug(void);
static void update_debug(void);
static void update_debug_background_buffer(void);
static void update_debug_tile_buffers(void);
static void update_debug_oam_buffers(void);

void emu_init(const char* save_path)
{
    strcpy(base_save_path, save_path);

    frame_buffer_565 = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    emu_frame_buffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];

    init_debug();

    gearboy = new GearboyCore();
    gearboy->Init();

    sound_queue = new Sound_Queue();
    sound_queue->start(44100, 2);

    audio_buffer = new s16[AUDIO_BUFFER_SIZE];

    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;

    audio_enabled = true;
    emu_audio_sync = true;
    emu_debug_disable_breakpoints = false;
    emu_debug_background_tile_address = -1;
    emu_debug_background_map_address = -1;
    emu_debug_tile_dmg_palette = 0;
    emu_debug_tile_color_palette = 0;
}

void emu_destroy(void)
{
    save_ram();
    SafeDeleteArray(audio_buffer);
    SafeDelete(sound_queue);
    SafeDelete(gearboy);
    SafeDeleteArray(frame_buffer_565);
    SafeDeleteArray(emu_frame_buffer);
    SafeDeleteArray(debug_background_buffer_565);
    SafeDeleteArray(emu_debug_background_buffer);
    for (int b = 0; b < 2; b++)
    {
        SafeDeleteArray(debug_tile_buffers_565[b]);
        SafeDeleteArray(emu_debug_tile_buffers[b]);
    }
    for (int s = 0; s < 40; s++)
    {
        SafeDeleteArray(debug_oam_buffers_565[s]);
        SafeDeleteArray(emu_debug_oam_buffers[s]);
    }
}

void emu_load_rom(const char* file_path, bool force_dmg, bool save_in_rom_dir, Cartridge::CartridgeTypes mbc)
{
    save_files_in_rom_dir = save_in_rom_dir;
    save_ram();
    gearboy->LoadROM(file_path, force_dmg, mbc);
    load_ram();
    emu_debug_continue();
}

void emu_update(void)
{
    if (!emu_is_empty())
    {
        int sampleCount = 0;

        if (!debugging || debug_step || debug_next_frame)
        {
            bool breakpoints = !emu_debug_disable_breakpoints || IsValidPointer(gearboy->GetMemory()->GetRunToBreakpoint());

            if (gearboy->RunToVBlank(frame_buffer_565, audio_buffer, &sampleCount, false, debug_step, breakpoints))
            {
                debugging = true;
            }

            debug_next_frame = false;
            debug_step = false;
        }

        generate_24bit_buffer(emu_frame_buffer, frame_buffer_565, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);

        update_debug();

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
    gearboy->SaveMemoryDump();
}

void emu_dissasemble_rom(void)
{
    gearboy->SaveDisassembledROM();
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

void emu_clear_cheats(void)
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
        const char* checksum = cart->IsValidROM() ? "VALID" : "FAILED";
        int version = cart->GetVersion();
        int rom_banks = cart->GetROMBankCount();
        int ram_banks = cart->GetRAMBankCount();

        const char* mbc = get_mbc(cart->GetType());

        sprintf(info, "File Name: %s\nMBC: %s\nGame Boy Color: %s\nSuper Game Boy: %s\nCartridge Name: %s\nCartridge Version: %d\nCartridge Checksum: %s\nROM Banks: %d\nRAM Banks: %d\nBattery: %s\nReal Time Clock: %s\nRumble: %s\n", filename, mbc, gbc, sgb, name, version, checksum, rom_banks, ram_banks, battery, rtc, rumble);
    }
    else
    {
        sprintf(info, "No data!");
    }
}

GearboyCore* emu_get_core(void)
{
    return gearboy;
}

void emu_color_correction(bool correction)
{
    color_correction = correction;
}

void emu_debug_step(void)
{
    debugging = debug_step = true;
    debug_next_frame = false;
    gearboy->Pause(false);
}

void emu_debug_continue(void)
{
    debugging = debug_step = debug_next_frame = false;
    gearboy->Pause(false);
}

void emu_debug_next_frame(void)
{
    debugging = debug_next_frame = true;
    debug_step = false;
    gearboy->Pause(false);
}

static void save_ram(void)
{
#ifdef DEBUG_GEARBOY
    emu_dissasemble_rom();
#endif

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

static void generate_24bit_buffer(GB_Color* dest, u16* src, int size)
{
    for (int i=0; i < size; i++)
    {
        dest[i].red = (((src[i] >> 11) & 0x1F ) * 255 + 15) / 31;
        dest[i].green = (((src[i] >> 5) & 0x3F ) * 255 + 31) / 63;
        dest[i].blue = ((src[i] & 0x1F ) * 255 + 15) / 31;

        if (gearboy->IsCGB() && color_correction)
        {
            u8 red = (u8)(((dest[i].red * 0.8125f) + (dest[i].green * 0.125f) + (dest[i].blue * 0.0625f)) * 0.95f);
            u8 green = (u8)(((dest[i].green * 0.75f) + (dest[i].blue * 0.25f)) * 0.95f);
            u8 blue = (u8)((((dest[i].red * 0.1875f) + (dest[i].green * 0.125f) + (dest[i].blue * 0.6875f))) * 0.95f);

            dest[i].red = red;
            dest[i].green = green;
            dest[i].blue = blue;
        }
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

static void init_debug(void)
{
    debug_background_buffer_565 = new u16[256 * 256];
    emu_debug_background_buffer = new GB_Color[256 * 256];

    for (int b = 0; b < 2; b++)
    {
        debug_tile_buffers_565[b] = new u16[16 * 24 * 64];
        emu_debug_tile_buffers[b] = new GB_Color[16 * 24 * 64];

        for (int i=0; i < (16 * 24 * 64); i++)
        {
            emu_debug_tile_buffers[b][i].red = 0;
            emu_debug_tile_buffers[b][i].green = 0;
            emu_debug_tile_buffers[b][i].blue = 0;
            debug_tile_buffers_565[b][i] = 0;
        }
    }

    for (int s = 0; s < 40; s++)
    {
        debug_oam_buffers_565[s] = new u16[8 * 16];
        emu_debug_oam_buffers[s] = new GB_Color[8 * 16];

        for (int i=0; i < (8 * 16); i++)
        {
            emu_debug_oam_buffers[s][i].red = 0;
            emu_debug_oam_buffers[s][i].green = 0;
            emu_debug_oam_buffers[s][i].blue = 0;
            debug_oam_buffers_565[s][i] = 0;
        }
    }
    
    for (int i=0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
    {
        emu_frame_buffer[i].red = 0;
        emu_frame_buffer[i].green = 0;
        emu_frame_buffer[i].blue = 0;
        frame_buffer_565[i] = 0;
    }

    for (int i=0; i < (256 * 256); i++)
    {
        emu_debug_background_buffer[i].red = 0;
        emu_debug_background_buffer[i].green = 0;
        emu_debug_background_buffer[i].blue = 0;
        debug_background_buffer_565[i] = 0;
    }
}

static void update_debug(void)
{
    update_debug_background_buffer();
    update_debug_tile_buffers();
    update_debug_oam_buffers();

    generate_24bit_buffer(emu_debug_background_buffer, debug_background_buffer_565, 256 * 256);

    for (int b = 0; b < 2; b++)
    {
        generate_24bit_buffer(emu_debug_tile_buffers[b], debug_tile_buffers_565[b], 16 * 24 * 64);
    }

    for (int s = 0; s < 40; s++)
    {
        generate_24bit_buffer(emu_debug_oam_buffers[s], debug_oam_buffers_565[s], 8 * 16);
    }

}

static void update_debug_background_buffer(void)
{
    Video* video = gearboy->GetVideo();
    Memory* memory = gearboy->GetMemory();
    u16* dmg_palette = gearboy->GetDMGInternalPalette();
    u8 lcdc = memory->Retrieve(0xFF40);

    for (int line = 0; line < 256; line++)
    {
        int line_width = (line * 256);

        for (int pixel = 0; pixel < 256; pixel++)
        {
            int offset_x = pixel & 0x7;
            int screen_tile = pixel >> 3;
            int tile_start_addr = emu_debug_background_tile_address >= 0 ? emu_debug_background_tile_address : IsSetBit(lcdc, 4) ? 0x8000 : 0x8800;
            int map_start_addr = emu_debug_background_map_address >= 0 ? emu_debug_background_map_address : IsSetBit(lcdc, 3) ? 0x9C00 : 0x9800;
            int line_32 = (line >> 3) << 5;
            int tile_pixel_y = line & 0x7;
            int tile_pixel_y_2 = tile_pixel_y << 1;
            int tile_pixel_y_flip_2 = (7 - tile_pixel_y) << 1;
            u8 palette = memory->Retrieve(0xFF47);

            int screen_pixel_x = (screen_tile << 3) + offset_x;
            u8 map_pixel_x = screen_pixel_x;
            int map_tile_x = map_pixel_x >> 3;
            int map_tile_offset_x = map_pixel_x & 0x7;
            u16 map_tile_addr = map_start_addr + line_32 + map_tile_x;
            int map_tile = 0;

            if (tile_start_addr == 0x8800)
            {
                map_tile = static_cast<s8> (memory->Retrieve(map_tile_addr));
                map_tile += 128;
            }
            else
            {
                map_tile = memory->Retrieve(map_tile_addr);
            }

            u8 cgb_tile_attr = gearboy->IsCGB() ? memory->ReadCGBLCDRAM(map_tile_addr, true) : 0;
            u8 cgb_tile_pal = gearboy->IsCGB() ? (cgb_tile_attr & 0x07) : 0;
            bool cgb_tile_bank = gearboy->IsCGB() ? IsSetBit(cgb_tile_attr, 3) : false;
            bool cgb_tile_xflip = gearboy->IsCGB() ? IsSetBit(cgb_tile_attr, 5) : false;
            bool cgb_tile_yflip = gearboy->IsCGB() ? IsSetBit(cgb_tile_attr, 6) : false;
            int map_tile_16 = map_tile << 4;
            u8 byte1 = 0;
            u8 byte2 = 0;
            int final_pixely_2 = cgb_tile_yflip ? tile_pixel_y_flip_2 : tile_pixel_y_2;
            int tile_address = tile_start_addr + map_tile_16 + final_pixely_2;

            if (cgb_tile_bank)
            {
                byte1 = memory->ReadCGBLCDRAM(tile_address, true);
                byte2 = memory->ReadCGBLCDRAM(tile_address + 1, true);
            }
            else
            {
                byte1 = memory->Retrieve(tile_address);
                byte2 = memory->Retrieve(tile_address + 1);
            }

            int pixel_x_in_tile = map_tile_offset_x;

            if (cgb_tile_xflip)
            {
                pixel_x_in_tile = 7 - pixel_x_in_tile;
            }
            int pixel_x_in_tile_bit = 0x1 << (7 - pixel_x_in_tile);
            int pixel_data = (byte1 & pixel_x_in_tile_bit) ? 1 : 0;
            pixel_data |= (byte2 & pixel_x_in_tile_bit) ? 2 : 0;

            int index = line_width + screen_pixel_x;

            if (gearboy->IsCGB())
            {
                PaletteMatrix bg_palettes = video->GetCGBBackgroundPalettes();
                debug_background_buffer_565[index] = (*bg_palettes)[cgb_tile_pal][pixel_data][1];
            }
            else
            {
                u8 color = (palette >> (pixel_data << 1)) & 0x03;
                debug_background_buffer_565[index] = dmg_palette[color];
            }
        }
    }
}

static void update_debug_tile_buffers(void)
{
    Memory* memory = gearboy->GetMemory();
    Video* video = gearboy->GetVideo();
    u16* dmg_palette = gearboy->GetDMGInternalPalette();
    PaletteMatrix bg_palettes = video->GetCGBBackgroundPalettes();
    PaletteMatrix sprite_palettes = video->GetCGBSpritePalettes();

    for (int b = 0; b < 2; b++)
    {
        for (int pixel = 0; pixel < (16 * 24 * 64); pixel++)
        {
            int tilex = (pixel >> 3) & 0xF;
            int tile_offset_x = pixel & 0x7;
            int tiley = (pixel >> 10); 
            int tile_offset_y = (pixel >> 7) & 0x7; 
            int tile = (tiley << 4) + tilex;
            int tile_address = 0x8000 + (tile << 4) + (tile_offset_y << 1);
            u8 byte1 = 0;
            u8 byte2 = 0;

            if (b == 0)
            {
                byte1 = memory->Retrieve(tile_address);
                byte2 = memory->Retrieve(tile_address + 1);
            }
            else
            {
                byte1 = memory->ReadCGBLCDRAM(tile_address, true);
                byte2 = memory->ReadCGBLCDRAM(tile_address + 1, true);
            }

            int tile_bit = 0x1 << (7 - tile_offset_x);
            int pixel_data = (byte1 & tile_bit) ? 1 : 0;
            pixel_data |= (byte2 & tile_bit) ? 2 : 0;

            if (gearboy->IsCGB())
            {
                if (emu_debug_tile_color_palette > 7)
                {
                    pixel_data = (*sprite_palettes)[emu_debug_tile_color_palette - 8][pixel_data][1];
                }
                else
                {
                    pixel_data = (*bg_palettes)[emu_debug_tile_color_palette][pixel_data][1];
                }
                debug_tile_buffers_565[b][pixel] = pixel_data;
            }
            else
            {
                u8 palette = memory->Retrieve(0xFF47 + emu_debug_tile_dmg_palette);
                pixel_data = (palette >> (pixel_data << 1)) & 0x03;
                debug_tile_buffers_565[b][pixel] = dmg_palette[pixel_data];
            }
        }
    }
}

static void update_debug_oam_buffers(void)
{
    Memory* memory = gearboy->GetMemory();
    Video* video = gearboy->GetVideo();
    u16 address = 0xFE00;
    u16* dmg_palette = gearboy->GetDMGInternalPalette();
    PaletteMatrix sprite_palettes = video->GetCGBSpritePalettes();
    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_16 = IsSetBit(lcdc, 2);

    for (int s = 0; s < 40; s++)
    {
        u8 tile = memory->Retrieve(address + 2);
        u8 flags = memory->Retrieve(address + 3);
        int palette = IsSetBit(flags, 4) ? 1 : 0;
        bool xflip = IsSetBit(flags, 5);
        bool yflip = IsSetBit(flags, 6);
        bool cgb_bank = IsSetBit(flags, 3);
        int cgb_pal = flags & 0x07;

        for (int pixel = 0; pixel < (8 * 16); pixel++)
        {
            u16 tile_addr = 0x8000 + (tile * 16);

            int pixel_x = pixel & 0x7;
            int pixel_y = pixel / 8;

            u16 line_addr = tile_addr + (2 * pixel_y);

            if (xflip)
                pixel_x = 7 - pixel_x;
            if (yflip)
                line_addr = (sprites_16 ? 15 : 7) - line_addr;

            u8 byte1 = 0;
            u8 byte2 = 0;

            if (gearboy->IsCGB() && cgb_bank)
            {
                byte1 = memory->ReadCGBLCDRAM(line_addr, true);
                byte2 = memory->ReadCGBLCDRAM(line_addr + 1, true);
            }
            else
            {
                byte1 = memory->Retrieve(line_addr);
                byte2 = memory->Retrieve(line_addr + 1);
            }

            int tile_bit = 0x1 << (7 - pixel_x);
            int pixel_data = (byte1 & tile_bit) ? 1 : 0;
            pixel_data |= (byte2 & tile_bit) ? 2 : 0;

            if (gearboy->IsCGB())
            {
                pixel_data = (*sprite_palettes)[cgb_pal][pixel_data][1];
                debug_oam_buffers_565[s][pixel] = pixel_data;
            }
            else
            {
                u8 final_palette = memory->Retrieve(0xFF48 + palette);
                pixel_data = (final_palette >> (pixel_data << 1)) & 0x03;
                debug_oam_buffers_565[s][pixel] = dmg_palette[pixel_data];
            }
        }

        address += 4;
    }
}

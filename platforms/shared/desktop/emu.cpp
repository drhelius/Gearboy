/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */

#define EMU_IMPORT
#include "emu.h"

#include <string.h>
#include <thread>
#include <atomic>
#include "gearboy.h"
#include "sound_queue.h"
#include "config.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(_WIN32)
#define STBIW_WINDOWS_UTF8
#endif
#include "stb_image_write.h"

static GB_Color original_palette[4] = {{0x87,0x96,0x03},{0x4D,0x6B,0x03},{0x2B,0x55,0x03},{0x14,0x44,0x03}};
static GB_Color sharp_palette[4] = {{0xF5,0xFA,0xEF},{0x86,0xC2,0x70},{0x2F,0x69,0x57},{0x0B,0x19,0x20}};
static GB_Color bw_palette[4] = {{0xFF,0xFF,0xFF},{0xAA,0xAA,0xAA},{0x55,0x55,0x55},{0x00,0x00,0x00}};
static GB_Color autumn_palette[4] = {{0xFF,0xF6,0xD3},{0xF9,0xA8,0x75},{0xEB,0x6B,0x6F},{0x7C,0x3F,0x58}};
static GB_Color soft_palette[4] = {{0xE0,0xE0,0xAA},{0xB0,0xB8,0x7C},{0x72,0x82,0x5B},{0x39,0x34,0x17}};
static GB_Color slime_palette[4] = {{0xD4,0xEB,0xA5},{0x62,0xB8,0x7C},{0x27,0x76,0x5D},{0x1D,0x39,0x39}};

static GearboyCore* gearboy;
static u16* frame_buffer_565;
static s16* audio_buffer;
static bool audio_enabled;
static u16* debug_background_buffer_565;
static u16* debug_tile_buffers_565[2];
static u16* debug_oam_buffers_565[40];

enum Loading_State
{
    Loading_State_None = 0,
    Loading_State_Loading,
    Loading_State_Finished
};

static std::atomic<int> loading_state(Loading_State_None);
static std::thread loading_thread;
static bool loading_thread_active = false;
static bool loading_result;
static char loading_file_path[4096];
static bool loading_force_dmg;
static Cartridge::CartridgeTypes loading_mbc;
static bool loading_force_gba;

static void save_ram(void);
static void load_ram(void);
static void reset_buffers(void);
static void generate_24bit_buffer(GB_Color* dest, u16* src, int size);
static const char* get_mbc(Cartridge::CartridgeTypes type);
static void init_debug(void);
static void update_debug(void);
static void debug_step_instruction(void);
static void update_debug_background_buffer(void);
static void update_debug_tile_buffers(void);
static void update_debug_oam_buffers(void);

bool emu_init(void)
{
    frame_buffer_565 = new u16[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    emu_frame_buffer = new GB_Color[GAMEBOY_WIDTH * GAMEBOY_HEIGHT];
    init_debug();
    gearboy = new GearboyCore();
    gearboy->Init();
    sound_queue_init();
    audio_buffer = new s16[AUDIO_BUFFER_SIZE];
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;
    audio_enabled = true;
    emu_audio_sync = true;
    emu_debug_disable_breakpoints = false;
    emu_debug_irq_breakpoints = false;
    emu_debug_step_frames_pending = 0;
    emu_debug_background_tile_address = -1;
    emu_debug_background_map_address = -1;
    emu_debug_tile_dmg_palette = 0;
    emu_debug_tile_color_palette = 0;
    emu_debug_command = Debug_Command_None;
    emu_debug_pc_changed = false;
    gearboy->SetFrameBuffer(reinterpret_cast<u8*>(emu_frame_buffer));
    for (int i = 0; i < 5; i++)
    {
        memset(&emu_savestates[i], 0, sizeof(GB_SaveState_Header));
        emu_savestates_screenshots[i].data = NULL;
        emu_savestates_screenshots[i].size = 0;
        emu_savestates_screenshots[i].width = 0;
        emu_savestates_screenshots[i].height = 0;
    }
    return true;
}

void emu_destroy(void)
{
    if (loading_thread_active)
    {
        loading_thread.join();
        loading_thread_active = false;
    }
    loading_state.store(Loading_State_None);

    save_ram();
    for (int i = 0; i < 5; i++)
        SafeDeleteArray(emu_savestates_screenshots[i].data);

    SafeDeleteArray(audio_buffer);
    sound_queue_destroy();
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

void emu_load_rom(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba)
{
    emu_audio_reset();
    save_ram();
    gearboy->LoadROM(file_path, force_dmg, mbc, force_gba);
    load_ram();
    emu_debug_continue();
}

static void load_rom_thread_func(void)
{
    loading_result = gearboy->LoadROM(loading_file_path, loading_force_dmg, loading_mbc, loading_force_gba);
    loading_state.store(Loading_State_Finished);
}

void emu_load_rom_async(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba)
{
    if (loading_state.load() != Loading_State_None)
        return;

    emu_debug_command = Debug_Command_None;
    reset_buffers();
    save_ram();

    strncpy(loading_file_path, file_path, sizeof(loading_file_path) - 1);
    loading_file_path[sizeof(loading_file_path) - 1] = '\0';
    loading_result = false;
    loading_force_dmg = force_dmg;
    loading_mbc = mbc;
    loading_force_gba = force_gba;
    loading_state.store(Loading_State_Loading);
    if (loading_thread_active)
        loading_thread.join();
    loading_thread = std::thread(load_rom_thread_func);
    loading_thread_active = true;
}

bool emu_is_rom_loading(void)
{
    return loading_state.load() == Loading_State_Loading;
}

bool emu_finish_rom_loading(void)
{
    if (loading_state.load() != Loading_State_Finished)
        return false;

    if (loading_thread_active)
    {
        loading_thread.join();
        loading_thread_active = false;
    }

    loading_state.store(Loading_State_None);

    if (!loading_result)
        return false;

    emu_audio_reset();
    load_ram();

    return true;
}

void emu_update(void)
{
    if (loading_state.load() != Loading_State_None)
        return;

    if (emu_is_empty())
        return;

    int sampleCount = 0;

    if (config_debug.debug)
    {
        bool breakpoint_hit = false;
        GearboyCore::GB_Debug_Run debug_run;
        debug_run.step_debugger = (emu_debug_command == Debug_Command_Step);
        debug_run.stop_on_breakpoint = !emu_debug_disable_breakpoints;
        debug_run.stop_on_run_to_breakpoint = true;
        debug_run.stop_on_irq = emu_debug_irq_breakpoints;

        if (emu_debug_command != Debug_Command_None)
            breakpoint_hit = gearboy->RunToVBlank(frame_buffer_565, audio_buffer, &sampleCount, false, &debug_run);

        if (breakpoint_hit || emu_debug_command == Debug_Command_StepFrame || emu_debug_command == Debug_Command_Step)
        {
            emu_debug_pc_changed = true;

            if (config_debug.dis_look_ahead_count > 0)
                gearboy->GetProcessor()->DisassembleAhead(config_debug.dis_look_ahead_count);
        }

        if (breakpoint_hit)
            emu_debug_command = Debug_Command_None;

        if (emu_debug_command == Debug_Command_StepFrame && emu_debug_step_frames_pending > 0)
        {
            emu_debug_step_frames_pending--;
            if (emu_debug_step_frames_pending > 0)
                emu_debug_command = Debug_Command_StepFrame;
            else
                emu_debug_command = Debug_Command_None;
        }
        else if (emu_debug_command != Debug_Command_Continue)
            emu_debug_command = Debug_Command_None;

        update_debug();
    }
    else
        gearboy->RunToVBlank(frame_buffer_565, audio_buffer, &sampleCount, false, NULL);

    generate_24bit_buffer(emu_frame_buffer, frame_buffer_565, GAMEBOY_WIDTH * GAMEBOY_HEIGHT);

    if ((sampleCount > 0) && !gearboy->IsPaused())
    {
        sound_queue_write(audio_buffer, sampleCount, emu_audio_sync);
    }
    else if (gearboy->IsPaused())
    {
        int silence_count = GB_AUDIO_QUEUE_SIZE;
        memset(audio_buffer, 0, silence_count * sizeof(s16));
        sound_queue_write(audio_buffer, silence_count, false);
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

bool emu_is_debug_idle(void)
{
    return config_debug.debug && (emu_debug_command == Debug_Command_None);
}
bool emu_is_empty(void)
{
    if (loading_state.load() != Loading_State_None)
        return true;
    return !gearboy->GetCartridge()->IsLoadedROM();
}

void emu_reset(bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba)
{
    emu_debug_command = Debug_Command_None;
    emu_audio_reset();
    save_ram();
    gearboy->ResetROM(force_dmg, mbc, force_gba);
    load_ram();
}

void emu_audio_volume(float volume)
{
    audio_enabled = (volume > 0.0f);
    gearboy->SetSoundVolume(volume);
}

void emu_audio_reset(void)
{
    sound_queue_stop();
    sound_queue_start(GB_AUDIO_SAMPLE_RATE, 2, GB_AUDIO_QUEUE_SIZE, config_audio.buffer_count);
}

bool emu_is_audio_enabled(void)
{
    return audio_enabled;
}

bool emu_is_audio_open(void)
{
    return sound_queue_is_open();
}

void emu_dmg_palette(GB_Color& c1, GB_Color& c2, GB_Color& c3, GB_Color& c4)
{
    gearboy->SetDMGPalette(c1, c2, c3, c4);
}

void emu_dmg_predefined_palette(int palette)
{
    GB_Color* p = NULL;

    switch (palette)
    {
        case 0:
            p = original_palette;
            break;
        case 1:
            p = sharp_palette;
            break;
        case 2:
            p = bw_palette;
            break;
        case 3:
            p = autumn_palette;
            break;
        case 4:
            p = soft_palette;
            break;
        case 5:
            p = slime_palette;
            break;
    }

    if (p)
        gearboy->SetDMGPalette(p[0], p[1], p[2], p[3]);
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

void emu_load_ram(const char* file_path, bool force_dmg, Cartridge::CartridgeTypes mbc, bool force_gba)
{
    if (!emu_is_empty())
    {
        save_ram();
        gearboy->ResetROM(force_dmg, mbc, force_gba);
        gearboy->LoadRam(file_path, true);
    }
}

static const char* get_configurated_dir(int option, const char* path)
{
    switch ((Directory_Location)option)
    {
        default:
        case Directory_Location_Default:
            return config_root_path;
        case Directory_Location_ROM:
            return NULL;
        case Directory_Location_Custom:
            return path;
    }
}

void emu_save_state_slot(int index)
{
    if (!emu_is_empty())
    {
        const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());
        gearboy->SaveState(dir, index, true);
        update_savestates_data();
    }
}

void emu_load_state_slot(int index)
{
    if (!emu_is_empty())
    {
        const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());
        gearboy->LoadState(dir, index, false);
    }
}

void emu_save_state_file(const char* file_path)
{
    if (!emu_is_empty()) {
        gearboy->SaveState(file_path, -1, true);
    }
}

void emu_load_state_file(const char* file_path)
{
    if (!emu_is_empty())
        gearboy->LoadState(file_path, -1, false);
}

void update_savestates_data(void)
{
    if (emu_is_empty())
        return;

    const char* dir = get_configurated_dir(config_emulator.savestates_dir_option, config_emulator.savestates_path.c_str());

    for (int i = 0; i < 5; i++)
    {
        emu_savestates[i].rom_name[0] = 0;
        SafeDeleteArray(emu_savestates_screenshots[i].data);
        emu_savestates_screenshots[i].size = 0;
        emu_savestates_screenshots[i].width = 0;
        emu_savestates_screenshots[i].height = 0;

        if (!gearboy->GetSaveStateHeader(i + 1, dir, &emu_savestates[i]))
            continue;

        if (emu_savestates[i].screenshot_size > 0)
        {
            emu_savestates_screenshots[i].data = new u8[emu_savestates[i].screenshot_size];
            emu_savestates_screenshots[i].size = emu_savestates[i].screenshot_size;
            gearboy->GetSaveStateScreenshot(i + 1, dir, &emu_savestates_screenshots[i]);
        }
    }
}

void emu_add_cheat(const char* cheat)
{
    gearboy->SetCheat(cheat);
}

void emu_clear_cheats(void)
{
    gearboy->ClearCheats();
}

void emu_get_info(char* info, int buffer_size)
{
    if (!emu_is_empty())
    {
        Cartridge* cart = gearboy->GetCartridge();
        snprintf(info, buffer_size, "File Name: %s\nMBC: %s\nGame Boy Color: %s\nSuper Game Boy: %s\nCartridge Name: %s\nVersion: %d\nChecksum: %s\nROM Banks: %d\nRAM Banks: %d\nBattery: %s\nRTC: %s\nRumble: %s\n",
            cart->GetFileName(), get_mbc(cart->GetType()), cart->IsCGB()?"YES":"NO", cart->IsSGB()?"YES":"NO",
            cart->GetName(), cart->GetVersion(), cart->IsValidROM()?"VALID":"FAILED",
            cart->GetROMBankCount(), cart->GetRAMBankCount(), cart->HasBattery()?"YES":"NO",
            cart->IsRTCPresent()?"YES":"NO", cart->IsRumblePresent()?"YES":"NO");
    }
    else
    {
        snprintf(info, buffer_size, "No data!");
    }
}

GearboyCore* emu_get_core(void)
{
    return gearboy;
}

void emu_color_correction(bool correction)
{
    gearboy->EnableColorCorrection(correction);
}

void emu_debug_step_over(void)
{
    Processor* processor = emu_get_core()->GetProcessor();
    Processor::ProcessorState* proc_state = processor->GetState();
    Memory* memory = emu_get_core()->GetMemory();
    u16 pc = proc_state->PC->GetValue();
    GB_Disassembler_Record* record = memory->GetDisassemblerRecord(pc);

    if (IsValidPointer(record) && record->subroutine)
    {
        u16 return_address = pc + record->size;
        processor->AddRunToBreakpoint(return_address);
        emu_debug_command = Debug_Command_Continue;
    }
    else
    {
        debug_step_instruction();
        return;
    }

    gearboy->Pause(false);
}

void emu_debug_step_into(void)
{
    debug_step_instruction();
}

void emu_debug_step_out(void)
{
    Processor* processor = emu_get_core()->GetProcessor();
    std::stack<Processor::GB_CallStackEntry>* call_stack = processor->GetDisassemblerCallStack();

    if (call_stack->size() > 0)
    {
        Processor::GB_CallStackEntry entry = call_stack->top();
        u16 return_address = entry.back;
        processor->AddRunToBreakpoint(return_address);
        emu_debug_command = Debug_Command_Continue;
    }
    else
    {
        debug_step_instruction();
        return;
    }

    gearboy->Pause(false);
}

void emu_debug_step_frame(void)
{
    gearboy->Pause(false);
    emu_debug_step_frames_pending++;
    emu_debug_command = Debug_Command_StepFrame;
}

void emu_debug_break(void)
{
    gearboy->Pause(false);
    if (emu_debug_command == Debug_Command_Continue)
        emu_debug_command = Debug_Command_Step;
}

void emu_debug_continue(void)
{
    gearboy->Pause(false);
    emu_debug_command = Debug_Command_Continue;
}

void emu_load_bootrom_dmg(const char* file_path)
{
    gearboy->GetMemory()->LoadBootromDMG(file_path);
}

void emu_load_bootrom_gbc(const char* file_path)
{
    gearboy->GetMemory()->LoadBootromGBC(file_path);
}

void emu_enable_bootrom_dmg(bool enable)
{
    gearboy->GetMemory()->EnableBootromDMG(enable);
}

void emu_enable_bootrom_gbc(bool enable)
{
    gearboy->GetMemory()->EnableBootromGBC(enable);
}

void emu_save_screenshot(const char* file_path)
{
    if (!gearboy->GetCartridge()->IsLoadedROM())
        return;
    Log("Saving screenshot to %s", file_path);
    stbi_write_png(file_path, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 3, emu_frame_buffer, GAMEBOY_WIDTH * 3);
}

void emu_save_sprite(const char* file_path, int index)
{
    if (!gearboy->GetCartridge()->IsLoadedROM())
        return;

    update_debug_oam_buffers();

    Memory* memory = gearboy->GetMemory();
    u8 lcdc = memory->Retrieve(0xFF40);
    bool sprites_16 = IsSetBit(lcdc, 2);
    int height = sprites_16 ? 16 : 8;

    generate_24bit_buffer(emu_debug_oam_buffers[index], debug_oam_buffers_565[index], 8 * 16);

    stbi_write_png(file_path, 8, height, 3, emu_debug_oam_buffers[index], 8 * 3);

    Log("Sprite saved to %s", file_path);
}

void emu_save_background(const char* file_path)
{
    if (!gearboy->GetCartridge()->IsLoadedROM())
        return;

    update_debug_background_buffer();
    generate_24bit_buffer(emu_debug_background_buffer, debug_background_buffer_565, 256 * 256);

    stbi_write_png(file_path, 256, 256, 3, emu_debug_background_buffer, 256 * 3);

    Log("Background saved to %s", file_path);
}

void emu_save_tiles(const char* file_path)
{
    if (!gearboy->GetCartridge()->IsLoadedROM())
        return;

    update_debug_tile_buffers();

    for (int b = 0; b < 2; b++)
        generate_24bit_buffer(emu_debug_tile_buffers[b], debug_tile_buffers_565[b], 16 * 24 * 64);

    int width = 16 * 8;
    int height = 24 * 8;
    int total_pixels = width * height;
    GB_Color* combined = new GB_Color[total_pixels * 2];

    for (int y = 0; y < height; y++)
    {
        memcpy(&combined[y * width * 2], &emu_debug_tile_buffers[0][y * width], width * sizeof(GB_Color));
        memcpy(&combined[y * width * 2 + width], &emu_debug_tile_buffers[1][y * width], width * sizeof(GB_Color));
    }

    stbi_write_png(file_path, width * 2, height, 3, combined, width * 2 * 3);

    delete[] combined;

    Log("Tiles saved to %s", file_path);
}

void emu_start_vgm_recording(const char* file_path)
{
    if (!gearboy->GetCartridge()->IsLoadedROM())
        return;
    if (gearboy->GetAudio()->IsVgmRecording())
        emu_stop_vgm_recording();
    if (gearboy->GetAudio()->StartVgmRecording(file_path, 4194304, false))
        Log("VGM recording started: %s", file_path);
}

void emu_stop_vgm_recording(void)
{
    if (gearboy->GetAudio()->IsVgmRecording()) { gearboy->GetAudio()->StopVgmRecording(); Log("VGM recording stopped"); }
}

bool emu_is_vgm_recording(void) { return gearboy->GetAudio()->IsVgmRecording(); }

static void reset_buffers(void)
{
    for (int i = 0; i < GAMEBOY_WIDTH * GAMEBOY_HEIGHT; i++)
    {
        emu_frame_buffer[i].red = 0;
        emu_frame_buffer[i].green = 0;
        emu_frame_buffer[i].blue = 0;
        frame_buffer_565[i] = 0;
    }
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        audio_buffer[i] = 0;
}

static void save_ram(void)
{
    const char* dir = get_configurated_dir(config_emulator.savefiles_dir_option, config_emulator.savefiles_path.c_str());
    if (dir != NULL)
        gearboy->SaveRam(dir);
    else
        gearboy->SaveRam();
}

static void load_ram(void)
{
    const char* dir = get_configurated_dir(config_emulator.savefiles_dir_option, config_emulator.savefiles_path.c_str());
    if (dir != NULL)
        gearboy->LoadRam(dir);
    else
        gearboy->LoadRam();
}

static void generate_24bit_buffer(GB_Color* dest, u16* src, int size)
{
    for (int i = 0; i < size; i++)
    {
        dest[i].red = (((src[i] >> 11) & 0x1F) * 255 + 15) / 31;
        dest[i].green = (((src[i] >> 5) & 0x3F) * 255 + 31) / 63;
        dest[i].blue = ((src[i] & 0x1F) * 255 + 15) / 31;
    }
}

static const char* get_mbc(Cartridge::CartridgeTypes type)
{
    switch (type)
    {
        case Cartridge::CartridgeNoMBC:
            return "ROM Only";
        case Cartridge::CartridgeMBC1:
            return "MBC 1";
        case Cartridge::CartridgeMBC1Multi:
            return "MBC 1 Multi 64";
        case Cartridge::CartridgeMBC2:
            return "MBC 2";
        case Cartridge::CartridgeMBC3:
            return "MBC 3";
        case Cartridge::CartridgeMBC5:
            return "MBC 5";
        case Cartridge::CartridgeNotSupported:
            return "Not Supported";
        default:
            return "Undefined";
    }
}

static void debug_step_instruction(void)
{
    emu_debug_command = Debug_Command_Step;
    gearboy->Pause(false);
}

static void init_debug(void)
{
    debug_background_buffer_565 = new u16[256 * 256];
    emu_debug_background_buffer = new GB_Color[256 * 256];

    for (int b = 0; b < 2; b++)
    {
        debug_tile_buffers_565[b] = new u16[16 * 24 * 64];
        emu_debug_tile_buffers[b] = new GB_Color[16 * 24 * 64];
        memset(debug_tile_buffers_565[b], 0, 16 * 24 * 64 * sizeof(u16));
        memset(emu_debug_tile_buffers[b], 0, 16 * 24 * 64 * sizeof(GB_Color));
    }

    for (int s = 0; s < 40; s++)
    {
        debug_oam_buffers_565[s] = new u16[8 * 16];
        emu_debug_oam_buffers[s] = new GB_Color[8 * 16];
        memset(debug_oam_buffers_565[s], 0, 8 * 16 * sizeof(u16));
        memset(emu_debug_oam_buffers[s], 0, 8 * 16 * sizeof(GB_Color));
    }

    memset(frame_buffer_565, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT * sizeof(u16));
    memset(emu_frame_buffer, 0, GAMEBOY_WIDTH * GAMEBOY_HEIGHT * sizeof(GB_Color));
    memset(debug_background_buffer_565, 0, 256 * 256 * sizeof(u16));
    memset(emu_debug_background_buffer, 0, 256 * 256 * sizeof(GB_Color));
}

static void update_debug(void)
{
    update_debug_background_buffer();
    update_debug_tile_buffers();
    update_debug_oam_buffers();

    generate_24bit_buffer(emu_debug_background_buffer, debug_background_buffer_565, 256 * 256);

    for (int b = 0; b < 2; b++)
        generate_24bit_buffer(emu_debug_tile_buffers[b], debug_tile_buffers_565[b], 16 * 24 * 64);

    for (int s = 0; s < 40; s++)
        generate_24bit_buffer(emu_debug_oam_buffers[s], debug_oam_buffers_565[s], 8 * 16);
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
                map_tile = static_cast<s8>(memory->Retrieve(map_tile_addr));
                map_tile += 128;
            }
            else
                map_tile = memory->Retrieve(map_tile_addr);
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
                pixel_x_in_tile = 7 - pixel_x_in_tile;
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
                    pixel_data = (*sprite_palettes)[emu_debug_tile_color_palette - 8][pixel_data][1];
                else
                    pixel_data = (*bg_palettes)[emu_debug_tile_color_palette][pixel_data][1];
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
            u16 line_addr = tile_addr + (2 * (yflip ? (sprites_16 ? 15 : 7) - pixel_y : pixel_y));
            if (xflip)
                pixel_x = 7 - pixel_x;
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

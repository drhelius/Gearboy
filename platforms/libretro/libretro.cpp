/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez
 * Copyright (C) 2017  Andrés Suárez
 * Copyright (C) 2017  Brad Parker

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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#if defined(_WIN32) && !defined(_XBOX)
#include <windows.h>
#endif
#include "libretro.h"

#include "../../src/gearboy.h"

#define VIDEO_WIDTH 160
#define VIDEO_HEIGHT 144
#define VIDEO_PIXELS (VIDEO_WIDTH * VIDEO_HEIGHT)

static u16* gearboy_frame_buf;

static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static char retro_base_directory[4096];
static char retro_game_path[4096];

static s16 audio_buf[AUDIO_BUFFER_SIZE];
static int audio_sample_count;

static bool force_dmg = false;
static bool allow_up_down = false;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

GearboyCore* core;

static retro_environment_t environ_cb;

static const struct retro_variable vars[] = {
    { "gearboy_model", "Emulated Model (restart); Auto|Game Boy DMG" },
    { "gearboy_palette", "Palette; Original|Sharp|B/W|Autumn|Soft|Slime" },
    { "gearboy_up_down_allowed", "Allow Up+Down / Left+Right; Disabled|Enabled" },

    { NULL }
};

// red, green, blue
static GB_Color original_palette[4] = {{0x87, 0x96, 0x03},{0x4D, 0x6B, 0x03},{0x2B, 0x55, 0x03},{0x14, 0x44, 0x03}};
static GB_Color sharp_palette[4] = {{0xF5, 0xFA, 0xEF},{0x86, 0xC2, 0x70},{0x2F, 0x69, 0x57},{0x0B, 0x19, 0x20}};
static GB_Color bw_palette[4] = {{0xFF, 0xFF, 0xFF},{0xAA, 0xAA, 0xAA},{0x55, 0x55, 0x55},{0x00, 0x00, 0x00}};
static GB_Color autumn_palette[4] = {{0xF8, 0xE8, 0xC8},{0xD8, 0x90, 0x48},{0xA8, 0x34, 0x20},{0x30, 0x18, 0x50}};
static GB_Color soft_palette[4] = {{0xE0, 0xE0, 0xAA},{0xB0, 0xB8, 0x7C},{0x72, 0x82, 0x5B},{0x39, 0x34, 0x17}};
static GB_Color slime_palette[4] = {{0xD4, 0xEB, 0xA5},{0x62, 0xB8, 0x7C},{0x27, 0x76, 0x5D},{0x1D, 0x39, 0x39}};

static GB_Color* current_palette = original_palette;

void retro_init(void)
{
    const char *dir = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
    }

    core = new GearboyCore();

#ifdef PS2
    core->Init(GB_PIXEL_BGR555);
#else
    core->Init(GB_PIXEL_RGB565);
#endif  

    gearboy_frame_buf = new u16[VIDEO_WIDTH * VIDEO_HEIGHT];

    audio_sample_count = 0;
}

void retro_deinit(void)
{
    SafeDeleteArray(gearboy_frame_buf);
    SafeDelete(core);
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = GEARBOY_TITLE;
    info->library_version  = GEARBOY_VERSION;
    info->need_fullpath    = false;
    info->valid_extensions = "gb|dmg|gbc|cgb|sgb";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect                = (float)VIDEO_WIDTH/VIDEO_HEIGHT;
    info->geometry.base_width   = VIDEO_WIDTH;
    info->geometry.base_height  = VIDEO_HEIGHT;
    info->geometry.max_width    = VIDEO_WIDTH;
    info->geometry.max_height   = VIDEO_HEIGHT;
    info->geometry.aspect_ratio = aspect;
    info->timing.fps            = 4194304.0 / 70224.0;
    info->timing.sample_rate    = 44100.0f;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    static const struct retro_controller_description controllers[] = {
        { "Nintendo Gameboy", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0) },
    };

    static const struct retro_controller_info ports[] = {
        { controllers, 1 },
        { NULL, 0 },
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);

    environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void *)vars);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static void update_input(void)
{
    input_poll_cb();

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
    {
        if (allow_up_down || !input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
            core->KeyPressed(Up_Key);
    }
    else
        core->KeyReleased(Up_Key);

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
    {
        if (allow_up_down || !input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
            core->KeyPressed(Down_Key);
    }
    else
        core->KeyReleased(Down_Key);

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
    {
        if (allow_up_down || !input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
            core->KeyPressed(Left_Key);
    }
    else
        core->KeyReleased(Left_Key);

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
    {
        if (allow_up_down || !input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
            core->KeyPressed(Right_Key);
    }
    else
        core->KeyReleased(Right_Key);

    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
        core->KeyPressed(B_Key);
    else
        core->KeyReleased(B_Key);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
        core->KeyPressed(A_Key);
    else
        core->KeyReleased(A_Key);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
        core->KeyPressed(Start_Key);
    else
        core->KeyReleased(Start_Key);
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT))
        core->KeyPressed(Select_Key);
    else
        core->KeyReleased(Select_Key);
}

static void check_variables(void)
{
    struct retro_variable var = {0};

    var.key = "gearboy_model";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Game Boy DMG") == 0)
            force_dmg = true;
        else
            force_dmg = false;
    }

    var.key = "gearboy_palette";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Original") == 0)
            current_palette = original_palette;
        else if (strcmp(var.value, "Sharp") == 0)
            current_palette = sharp_palette;
        else if (strcmp(var.value, "B/W") == 0)
            current_palette = bw_palette;
        else if (strcmp(var.value, "Autumn") == 0)
            current_palette = autumn_palette;
        else if (strcmp(var.value, "Soft") == 0)
            current_palette = soft_palette;
        else if (strcmp(var.value, "Slime") == 0)
            current_palette = slime_palette;
        else
            current_palette = original_palette;
    }

    var.key = "gearboy_up_down_allowed";
    var.value = NULL;

    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
    {
        if (strcmp(var.value, "Enabled") == 0)
            allow_up_down = true;
        else
            allow_up_down = false;
    }
}

void retro_run(void)
{
    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
    {
        check_variables();
        core->SetDMGPalette(current_palette[0], current_palette[1], current_palette[2], current_palette[3]);
    }

    update_input();

    core->RunToVBlank(gearboy_frame_buf, audio_buf, &audio_sample_count);

    video_cb((uint8_t*)gearboy_frame_buf, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * sizeof(u16));

    if (audio_sample_count > 0)
        audio_batch_cb(audio_buf, audio_sample_count / 2);

    audio_sample_count = 0;
}

void retro_reset(void)
{
    check_variables();

    core->SetDMGPalette(current_palette[0], current_palette[1], current_palette[2], current_palette[3]);

    core->ResetROMPreservingRAM(force_dmg);
}


bool retro_load_game(const struct retro_game_info *info)
{
    check_variables();

    core->SetDMGPalette(current_palette[0], current_palette[1], current_palette[2], current_palette[3]);

    core->LoadROMFromBuffer(reinterpret_cast<const u8*>(info->data), info->size, force_dmg);

    struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
        { 0 },
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "RETRO_PIXEL_FORMAT_RGB565 is not supported.\n");
        return false;
    }

    snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path);

    struct retro_memory_descriptor descs[11];

    memset(descs, 0, sizeof(descs));

    // IE
    descs[0].ptr   = core->GetMemory()->GetMemoryMap() + 0xFFFF;
    descs[0].start = 0xFFFF;
    descs[0].len   = 1;
    // HRAM
    descs[1].ptr   = core->GetMemory()->GetMemoryMap() + 0xFF80;
    descs[1].start = 0xFF80;
    descs[1].len   = 0x0080;
    // RAM bank 0
    descs[2].ptr   = core->IsCGB() ? core->GetMemory()->GetCGBRAM() : (core->GetMemory()->GetMemoryMap() + 0xC000);
    descs[2].start = 0xC000;
    descs[2].len   = 0x1000;
    // RAM bank 1
    descs[3].ptr   = core->IsCGB() ? (core->GetMemory()->GetCGBRAM() + (0x1000)) : (core->GetMemory()->GetMemoryMap() + 0xD000);
    descs[3].start = 0xD000;
    descs[3].len   = 0x1000;
    // CART RAM
    descs[4].ptr   = core->GetMemory()->GetCurrentRule()->GetCurrentRamBank();
    descs[4].start = 0xA000;
    descs[4].len   = 0x2000;
    // VRAM
    descs[5].ptr   = core->GetMemory()->GetMemoryMap() + 0x8000; // todo: fix GBC
    descs[5].start = 0x8000;
    descs[5].len   = 0x2000;
    // ROM bank 0
    descs[6].ptr   = core->GetMemory()->GetCurrentRule()->GetRomBank0();
    descs[6].start = 0x0000;
    descs[6].len   = 0x4000;
    // ROM bank x
    descs[7].ptr   = core->GetMemory()->GetCurrentRule()->GetCurrentRomBank1();
    descs[7].start = 0x4000;
    descs[7].len   = 0x4000;
    // OAM
    descs[8].ptr   = core->GetMemory()->GetMemoryMap() + 0xFE00;
    descs[8].start = 0xFE00;
    descs[8].len   = 0x00A0;
    descs[8].select= 0xFFFFFF00;
    // CGB RAM banks 2-7
    descs[9].ptr   = core->IsCGB() ? (core->GetMemory()->GetCGBRAM() + 0x2000) : (core->GetMemory()->GetMemoryMap() + 0xD000);
    descs[9].start = 0x10000;
    descs[9].len   = core->IsCGB() ? 0x6000 : 0;
    descs[9].select= 0xFFFF0000;
    // IO PORTS
    descs[10].ptr   = core->GetMemory()->GetMemoryMap() + 0xFF00;
    descs[10].start = 0xFF00;
    descs[10].len   = 0x0080;
    descs[10].select= 0xFFFFFF00;

    struct retro_memory_map mmaps;
    mmaps.descriptors = descs;
    mmaps.num_descriptors = sizeof(descs) / sizeof(descs[0]);
    environ_cb(RETRO_ENVIRONMENT_SET_MEMORY_MAPS, &mmaps);

    bool achievements = true;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS, &achievements);

    return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    size_t size;
    core->SaveState(NULL, size);
    return size;
}

bool retro_serialize(void *data, size_t size)
{
    return core->SaveState(reinterpret_cast<u8*>(data), size);
}

bool retro_unserialize(const void *data, size_t size)
{
    return core->LoadState(reinterpret_cast<const u8*>(data), size);
}

void *retro_get_memory_data(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return core->GetMemory()->GetCurrentRule()->GetRamBanks();
        case RETRO_MEMORY_RTC:
            return core->GetMemory()->GetCurrentRule()->GetRTCMemory();
        case RETRO_MEMORY_SYSTEM_RAM:
            return core->IsCGB() ? core->GetMemory()->GetCGBRAM() : core->GetMemory()->GetMemoryMap() + 0xC000;
    }

    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
           return core->GetMemory()->GetCurrentRule()->GetRamSize();
        case RETRO_MEMORY_RTC:
           return core->GetMemory()->GetCurrentRule()->GetRTCSize();
        case RETRO_MEMORY_SYSTEM_RAM:
           return core->IsCGB() ? 0x8000 : 0x2000;
    }

    return 0;
}

void retro_cheat_reset(void)
{
    core->ClearCheats();
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    core->SetCheat(code);
}

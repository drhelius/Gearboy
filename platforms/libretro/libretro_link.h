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

#ifndef LIBRETRO_LINK_H
#define LIBRETRO_LINK_H

#include "../../src/GearboyCore.h"
#include "libretro.h"

#define GEARBOY_LINK_SUBSYSTEM 0x101
#define GEARBOY_LINK_RAM_1 ((1 << 8) | RETRO_MEMORY_SAVE_RAM)
#define GEARBOY_LINK_RTC_1 ((1 << 8) | RETRO_MEMORY_RTC)
#define GEARBOY_LINK_RAM_2 ((2 << 8) | RETRO_MEMORY_SAVE_RAM)
#define GEARBOY_LINK_RTC_2 ((2 << 8) | RETRO_MEMORY_RTC)

struct LibretroInstance
{
    GearboyCore* core;
    u16 frame_buffer[SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT];
    s16 audio_buffer[AUDIO_BUFFER_SIZE];
    int sample_count;
};

class LibretroLink
{
public:
    LibretroLink(LibretroInstance* instances);
    ~LibretroLink();
    void Reset();
    void RunFrame();
    void Geometry(bool vertical, int selection, unsigned* width, unsigned* height);
    const u16* Video(bool vertical, bool switched, int selection);
    const s16* Audio(int selection, int* count);
    size_t StateSize();
    bool SaveState(void* data, size_t size);
    bool LoadState(const void* data, size_t size);
    void SetSavePath(const char* content_path, const char* save_directory);
    void PersistentMemory(bool write, const retro_vfs_interface* vfs);
    bool PersistentMemory(const char* path, unsigned id, bool write, const retro_vfs_interface* vfs);

private:
    struct SerialState
    {
        u64 cycle;
        u8 sb;
        u8 sc;
    };

    struct Runtime
    {
        u64 frame_cycle;
        u64 origin[2];
        SerialState states[2][16];
        u32 state_count[2];
        GB_LinkCableTransfer transfer[2];
        GB_LinkCableTransfer start[2];
        bool start_pending[2];
        bool pending[2];
    };

    struct Endpoint
    {
        LibretroLink* link;
        unsigned index;
    };

    static void StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent, void* data);
    static void StartCallback(u64 cycle, u64 first_shift, u32 bit_cycles, u8 outgoing, u32 transfer_id, u8* incoming, void* data);
    static bool PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* data);
    u64 Cycle(unsigned index);
    void ResolveTransfers();
    bool ReadState(const u8* data, size_t size);

    LibretroInstance* m_instances;
    Endpoint m_endpoint[2];
    Runtime m_runtime;
    u16 m_video[2 * 160 * 144];
    s16 m_mix[AUDIO_BUFFER_SIZE];
    char m_save_path[4096];
    u32 m_rom_hash[2];
    bool m_has_rom_identity;
};

#endif /* LIBRETRO_LINK_H */

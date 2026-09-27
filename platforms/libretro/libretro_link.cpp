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

#include "libretro_link.h"
#include "../../src/Memory.h"
#include "../../src/Processor.h"
#include "../../src/memory_stream.h"
#include <string.h>
#include <sstream>

static const size_t frame_size = GAMEBOY_WIDTH * GAMEBOY_HEIGHT * sizeof(u16);

static u32 StateChecksum(const u8* data, size_t size);

LibretroLink::LibretroLink(LibretroInstance* instances)
{
    m_instances = instances;
    m_save_path[0] = 0;
    m_has_rom_identity = false;
    memset(m_rom_hash, 0, sizeof(m_rom_hash));
    memset(&m_runtime, 0, sizeof(m_runtime));

    for (unsigned i = 0; i < 2; i++)
    {
        m_endpoint[i].link = this;
        m_endpoint[i].index = i;
        m_instances[i].core->SetLinkCableCallbacks(StateCallback, StartCallback, PollCallback, NULL, &m_endpoint[i]);
    }
}

LibretroLink::~LibretroLink()
{
    for (unsigned i = 0; i < 2; i++)
    {
        m_instances[i].core->SetLinkCableConnected(false);
        m_instances[i].core->SetLinkCableCallbacks(NULL, NULL, NULL, NULL, NULL);
    }
}

void LibretroLink::Reset()
{
    memset(&m_runtime, 0, sizeof(m_runtime));

    for (unsigned i = 0; i < 2; i++)
    {
        m_runtime.origin[i] = m_instances[i].core->GetLinkCableCycle();
        if (!m_has_rom_identity)
            m_rom_hash[i] = StateChecksum(m_instances[i].core->GetCartridge()->GetTheROM(), m_instances[i].core->GetCartridge()->GetTotalSize());
    }

    m_has_rom_identity = true;

    for (unsigned i = 0; i < 2; i++)
        m_instances[i].core->SetLinkCableConnected(true);
}

u64 LibretroLink::Cycle(unsigned index)
{
    return m_instances[index].core->GetLinkCableCycle() - m_runtime.origin[index];
}

void LibretroLink::StateCallback(u64 cycle, u8 sb, u8 sc, GB_SerialEvent, void* data)
{
    Endpoint* endpoint = static_cast<Endpoint*>(data);
    Runtime& runtime = endpoint->link->m_runtime;

    unsigned index = endpoint->index;

    SerialState& state = runtime.states[index][runtime.state_count[index]++ % 16];
    state.cycle = cycle - runtime.origin[index];
    state.sb = sb;
    state.sc = sc;

    if ((sc & 0x81) != 0x80)
        runtime.pending[index] = false;
}

void LibretroLink::StartCallback(u64 cycle, u64 first_shift, u32 bit_cycles, u8 outgoing, u32 transfer_id, u8* incoming, void* data)
{
    Endpoint* endpoint = static_cast<Endpoint*>(data);
    LibretroLink* link = endpoint->link;
    Runtime& runtime = link->m_runtime;

    unsigned index = endpoint->index;

    *incoming = 0xFF;

    GB_LinkCableTransfer& start = runtime.start[index];
    start.request_cycle = cycle - runtime.origin[index];
    start.first_shift_cycle = first_shift - runtime.origin[index];
    start.bit_cycles = bit_cycles;
    start.transfer_id = transfer_id;
    start.incoming_byte = outgoing;
    runtime.start_pending[index] = true;
}

void LibretroLink::ResolveTransfers()
{
    for (unsigned index = 0; index < 2; index++)
    {
        unsigned peer = index ^ 1;

        const GB_LinkCableTransfer& start = m_runtime.start[index];

        if (!m_runtime.start_pending[index] || Cycle(peer) < start.request_cycle)
            continue;

        m_runtime.start_pending[index] = false;

        u32 count = MIN(m_runtime.state_count[peer], 16u);

        for (u32 i = 0; i < count; i++)
        {
            const SerialState& state = m_runtime.states[peer][(m_runtime.state_count[peer] - 1 - i) % 16];

            if (state.cycle > start.request_cycle)
                continue;

            if ((state.sc & 0x81) == 0x80)
            {
                m_instances[index].core->GetProcessor()->SetLinkCableIncomingByte(start.transfer_id, state.sb);
                GB_LinkCableTransfer& transfer = m_runtime.transfer[peer];
                transfer = start;
                transfer.request_cycle += m_runtime.origin[peer];
                transfer.first_shift_cycle += m_runtime.origin[peer];
                transfer.local_byte = state.sb;
                m_runtime.pending[peer] = true;
            }
            break;
        }
    }
}

bool LibretroLink::PollCallback(u64 cycle, GB_LinkCableTransfer* transfer, void* data)
{
    Endpoint* endpoint = static_cast<Endpoint*>(data);
    Runtime& runtime = endpoint->link->m_runtime;

    unsigned index = endpoint->index;

    if (!runtime.pending[index] || cycle < runtime.transfer[index].request_cycle)
        return false;

    *transfer = runtime.transfer[index];
    runtime.pending[index] = false;
    return true;
}

void LibretroLink::RunFrame()
{
    m_runtime.frame_cycle += GAMEBOY_CLOCKS_PER_FRAME;

    while (Cycle(0) < m_runtime.frame_cycle || Cycle(1) < m_runtime.frame_cycle)
    {
        unsigned index = Cycle(0) <= Cycle(1) ? 0 : 1;
        unsigned int clocks;

        if (m_instances[index].core->RunCycle(m_instances[index].frame_buffer, clocks))
            m_instances[index].core->RenderFrameBuffer(m_instances[index].frame_buffer);

        ResolveTransfers();
    }

    for (unsigned i = 0; i < 2; i++)
        m_instances[i].core->EndFrame(m_instances[i].audio_buffer, &m_instances[i].sample_count);
}

void LibretroLink::Geometry(bool vertical, int selection, unsigned* width, unsigned* height)
{
    *width = (selection == 0 && !vertical) ? 320 : 160;
    *height = (selection == 0 && vertical) ? 288 : 144;
}

const u16* LibretroLink::Video(bool vertical, bool switched, int selection)
{
    if (selection != 0)
        return m_instances[selection - 1].frame_buffer;

    unsigned first = switched ? 1 : 0;

    if (vertical)
    {
        memcpy(m_video, m_instances[first].frame_buffer, frame_size);
        memcpy(m_video + 160 * 144, m_instances[first ^ 1].frame_buffer, frame_size);
    }
    else
    {
        for (unsigned y = 0; y < 144; y++)
        {
            memcpy(m_video + y * 320, m_instances[first].frame_buffer + y * 160, 160 * sizeof(u16));
            memcpy(m_video + y * 320 + 160, m_instances[first ^ 1].frame_buffer + y * 160, 160 * sizeof(u16));
        }
    }

    return m_video;
}

const s16* LibretroLink::Audio(int selection, int* count)
{
    if (selection < 2)
    {
        *count = m_instances[selection].sample_count;
        return m_instances[selection].audio_buffer;
    }

    *count = MAX(m_instances[0].sample_count, m_instances[1].sample_count);

    for (int i = 0; i < *count; i++)
    {
        int first = i < m_instances[0].sample_count ? m_instances[0].audio_buffer[i] : 0;
        int second = i < m_instances[1].sample_count ? m_instances[1].audio_buffer[i] : 0;
        m_mix[i] = (s16)((first + second) / 2);
    }

    return m_mix;
}

struct LinkStateHeader
{
    u32 magic;
    u32 version;
    u32 core_size[2];
    u32 runtime_size;
    u32 cgb_mode[2];
    u32 mapper[2];
    u32 rom_hash[2];
    u32 checksum;
};

static u32 StateChecksum(const u8* data, size_t size)
{
    u32 hash = 2166136261u;

    for (size_t i = 0; i < size; i++)
        hash = (hash ^ data[i]) * 16777619u;

    return hash;
}

size_t LibretroLink::StateSize()
{
    size_t size = sizeof(LinkStateHeader) + sizeof(m_runtime) + 2 * frame_size;

    for (unsigned i = 0; i < 2; i++)
    {
        size_t core_size = 0;
        if (!m_instances[i].core->SaveState(NULL, core_size))
            return 0;

        std::ostringstream stream;
        m_instances[i].core->SaveLinkCableState(stream);
        size += core_size + stream.str().size();
    }

    return size;
}

bool LibretroLink::SaveState(void* data, size_t size)
{
    size_t required = StateSize();
    if (!data || !required || size < required)
        return false;

    u8* bytes = static_cast<u8*>(data);
    memset(bytes, 0, size);

    LinkStateHeader header = {};
    header.magic = 0x4B4C4247; // GBLK
    header.version = 1;
    header.runtime_size = sizeof(m_runtime);

    size_t offset = sizeof(header);
    memcpy(bytes + offset, &m_runtime, sizeof(m_runtime));
    offset += sizeof(m_runtime);

    for (unsigned i = 0; i < 2; i++)
    {
        memcpy(bytes + offset, m_instances[i].frame_buffer, frame_size);
        offset += frame_size;
    }

    for (unsigned i = 0; i < 2; i++)
    {
        size_t core_size = required - offset;

        if (!m_instances[i].core->SaveState(bytes + offset, core_size))
            return false;

        header.core_size[i] = (u32)core_size;
        header.rom_hash[i] = m_rom_hash[i];
        header.cgb_mode[i] = m_instances[i].core->IsCGB() ? 1 : 0;
        header.mapper[i] = (u32)m_instances[i].core->GetMemory()->GetCurrentRule()->GetMapperType();
        offset += core_size;

        memory_stream stream(reinterpret_cast<char*>(bytes + offset), required - offset);

        m_instances[i].core->SaveLinkCableState(stream);

        if (!stream.good())
            return false;

        offset += stream.size();
    }

    header.checksum = StateChecksum(bytes + sizeof(header), required - sizeof(header));
    memcpy(bytes, &header, sizeof(header));

    return true;
}

bool LibretroLink::ReadState(const u8* data, size_t size)
{
    LinkStateHeader header;
    memcpy(&header, data, sizeof(header));

    size_t offset = sizeof(header);
    memcpy(&m_runtime, data + offset, sizeof(m_runtime));
    offset += sizeof(m_runtime);

    for (unsigned i = 0; i < 2; i++)
    {
        memcpy(m_instances[i].frame_buffer, data + offset, frame_size);
        offset += frame_size;
        m_instances[i].sample_count = 0;
    }

    for (unsigned i = 0; i < 2; i++)
    {
        if (!m_instances[i].core->LoadState(data + offset, header.core_size[i]))
            return false;

        offset += header.core_size[i];
        memory_input_stream stream(reinterpret_cast<const char*>(data + offset), size - offset);

        m_instances[i].core->LoadLinkCableState(stream);

        if (!stream.good())
            return false;

        offset += (size_t)stream.tellg();
    }

    return true;
}

bool LibretroLink::LoadState(const void* data, size_t size)
{
    size_t required = StateSize();

    if (!data || !required || size < required)
        return false;

    LinkStateHeader header;
    memcpy(&header, data, sizeof(header));

    if (header.magic != 0x4B4C4247 || header.version != 1 || header.runtime_size != sizeof(m_runtime))
        return false;

    const u8* bytes = static_cast<const u8*>(data);

    if (header.checksum != StateChecksum(bytes + sizeof(header), required - sizeof(header)))
        return false;

    for (unsigned i = 0; i < 2; i++)
    {
        size_t core_size = 0;
        m_instances[i].core->SaveState(NULL, core_size);

        if (header.core_size[i] != core_size || header.rom_hash[i] != m_rom_hash[i] ||
            header.cgb_mode[i] != (m_instances[i].core->IsCGB() ? 1u : 0u) ||
            header.mapper[i] != (u32)m_instances[i].core->GetMemory()->GetCurrentRule()->GetMapperType())
            return false;
    }

    u8* backup = new u8[required];

    bool saved = SaveState(backup, required);
    bool loaded = saved && ReadState(bytes, required);

    if (saved && !loaded)
        ReadState(backup, required);

    delete[] backup;
    return loaded;
}

void LibretroLink::SetSavePath(const char* content_path, const char* save_directory)
{
    m_save_path[0] = 0;

    if (!content_path || !content_path[0])
        return;

    const char* filename = strrchr(content_path, '/');
    const char* backslash = strrchr(content_path, '\\');

    if (backslash && (!filename || backslash > filename))
        filename = backslash;

    filename = filename ? filename + 1 : content_path;
    int count;

    if (save_directory && save_directory[0])
        count = snprintf(m_save_path, sizeof(m_save_path), "%s/%s", save_directory, filename);
    else
        count = snprintf(m_save_path, sizeof(m_save_path), "%s", content_path);

    if (count < 0 || (size_t)count >= sizeof(m_save_path))
    {
        m_save_path[0] = 0;
        Log("Screen 2 save path is too long");
        return;
    }

    char* extension = strrchr(m_save_path, '.');
    char* separator = strrchr(m_save_path, '/');
    char* windows_separator = strrchr(m_save_path, '\\');

    if (windows_separator && (!separator || windows_separator > separator))
        separator = windows_separator;

    if (extension && (!separator || extension > separator))
        *extension = 0;
}

void LibretroLink::PersistentMemory(bool write, const retro_vfs_interface* vfs)
{
    if (!m_save_path[0])
        return;

    const char* extensions[] = { "srm2", "rtc2" };
    const unsigned ids[] = { RETRO_MEMORY_SAVE_RAM, RETRO_MEMORY_RTC };

    for (unsigned i = 0; i < 2; i++)
    {
        char path[4112];
        snprintf(path, sizeof(path), "%s.%s", m_save_path, extensions[i]);

        if (!PersistentMemory(path, ids[i], write, vfs) && write)
            Log("Could not save screen 2 memory: %s", path);
    }
}

bool LibretroLink::PersistentMemory(const char* path, unsigned id, bool write, const retro_vfs_interface* vfs)
{
    MemoryRule* rule = m_instances[1].core->GetMemory()->GetCurrentRule();
    size_t size = id == RETRO_MEMORY_SAVE_RAM ? rule->GetRamSize() : rule->GetRTCSize();
    void* data = id == RETRO_MEMORY_SAVE_RAM ? rule->GetRamBanks() : rule->GetRTCMemory();

    if (!size || !data)
        return true;

    u8* buffer = new u8[size];

    if (write)
        memcpy(buffer, data, size);

    size_t total = 0;
    bool closed = false;

    if (vfs)
    {
        if (write && !vfs->write)
        {
            delete[] buffer;
            return false;
        }

        retro_vfs_file_handle* file = vfs->open(path, write ? RETRO_VFS_FILE_ACCESS_WRITE : RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

        if (file)
        {
            while (total < size)
            {
                int64_t count = write ? vfs->write(file, buffer + total, size - total) : vfs->read(file, buffer + total, size - total);

                if (count <= 0 || (uint64_t)count > size - total)
                    break;

                total += (size_t)count;
            }

            closed = vfs->close(file) == 0;
        }
    }
    else
    {
        FILE* file = fopen(path, write ? "wb" : "rb");

        if (file)
        {
            total = write ? fwrite(buffer, 1, size, file) : fread(buffer, 1, size, file);
            closed = fclose(file) == 0;
        }
    }

    bool success = closed && total == size;

    if (!write && success)
        memcpy(data, buffer, size);

    delete[] buffer;
    return success;
}

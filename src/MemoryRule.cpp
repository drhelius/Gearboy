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

#include "MemoryRule.h"
#include "Cartridge.h"

MemoryRule::MemoryRule(Processor* pProcessor, Memory* pMemory,
        Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio)
{
    m_pProcessor = pProcessor;
    m_pMemory = pMemory;
    m_pVideo = pVideo;
    m_pInput = pInput;
    m_pCartridge = pCartridge;
    m_pAudio = pAudio;
    m_bCGB = false;
    InitPointer(m_pRamChangedCallback);
    InitPointer(m_pTraceLogger);
}

MemoryRule::~MemoryRule()
{
}

bool MemoryRule::MapsROMDirectly()
{
    return false;
}

u8 MemoryRule::GetMapperType()
{
    return (u8)m_pCartridge->GetType();
}

void MemoryRule::SetTraceLogger(TraceLogger* pTraceLogger)
{
    m_pTraceLogger = pTraceLogger;
}

void MemoryRule::LogTraceMapperEvent(u16 address, u8 value, u8 event, u8 flags, bool flags_valid)
{
#if !defined(GEARBOY_DISABLE_DISASSEMBLER)
    GB_Trace_Entry e = {};
    e.type = TRACE_MAPPER;
    e.mapper.address = address;
    u8 mapper = GetMapperType();

    if (mapper == Cartridge::CartridgeMBC6)
    {
        e.mapper.rom_bank0 = GetCurrentRomBankIndex(0x4000);
        e.mapper.rom_bank1 = GetCurrentRomBankIndex(0x6000);
        e.mapper.ram_bank = (s16)((GetCurrentRamBankIndex(0xA000) << 8) | GetCurrentRamBankIndex(0xB000));
    }
    else
    {
        e.mapper.rom_bank0 = (u16)GetCurrentRomBank0Index();
        e.mapper.rom_bank1 = (u16)GetCurrentRomBank1Index();
        e.mapper.ram_bank = (s16)GetCurrentRamBankIndex();
    }

    e.mapper.value = value;
    e.mapper.mapper = mapper;
    e.mapper.event = event;
    e.mapper.flags = flags;
    e.mapper.flags_valid = flags_valid ? 1 : 0;
    m_pTraceLogger->TraceLog(e);
#else
    UNUSED(address);
    UNUSED(value);
    UNUSED(event);
    UNUSED(flags);
    UNUSED(flags_valid);
#endif
}

bool MemoryRule::NeedsHighMemoryAccessNotifications()
{
    return false;
}

void MemoryRule::NotifyHighMemoryRead(u16 address)
{
    UNUSED(address);
}

void MemoryRule::NotifyHighMemoryWrite(u16 address, u8 value)
{
    UNUSED(address);
    UNUSED(value);
}

void MemoryRule::SaveRam(std::ostream&)
{
    Debug("MemoryRule::SaveRam not implemented");
}

bool MemoryRule::LoadRam(std::istream&, s32)
{
    Debug("MemoryRule::LoadRam not implemented");
    return false;
}

void MemoryRule::SetRamChangedCallback(RamChangedCallback callback)
{
    m_pRamChangedCallback = callback;
}

size_t MemoryRule::GetRamSize()
{
    Debug("MemoryRule::GetRamSize not implemented");
    return 0;
}

size_t MemoryRule::GetRTCSize()
{
    Debug("MemoryRule::GetRTCSize not implemented");
    return 0;
}

u8* MemoryRule::GetRamBanks()
{
    Debug("MemoryRule::GetRamBanks not implemented");
    return NULL;
}

u8* MemoryRule::GetCurrentRamBank()
{
    Debug("MemoryRule::GetCurrentRamBank not implemented");
    return NULL;
}

int MemoryRule::GetCurrentRamBankIndex()
{
    Debug("MemoryRule::GetCurrentRamBankIndex not implemented");
    return 0;
}

int MemoryRule::GetCurrentRamBankIndex(u16 address)
{
    UNUSED(address);
    return GetCurrentRamBankIndex();
}

u8* MemoryRule::GetRomBank0()
{
    Debug("MemoryRule::GetRomBank0 not implemented");
    return NULL;
}

int MemoryRule::GetCurrentRomBank0Index()
{
    Debug("MemoryRule::GetCurrentRomBank0Index not implemented");
    return 0;
}

u8* MemoryRule::GetCurrentRomBank1()
{
    Debug("MemoryRule::GetCurrentRomBank1 not implemented");
    return NULL;
}

int MemoryRule::GetCurrentRomBank1Index()
{
    Debug("MemoryRule::GetCurrentRomBank1Index not implemented");
    return 1;
}

u16 MemoryRule::GetCurrentRomBankIndex(u16 address)
{
    if (address < 0x4000)
        return (u16)GetCurrentRomBank0Index();

    return (u16)GetCurrentRomBank1Index();
}

u32 MemoryRule::GetPhysicalROMAddress(u16 address)
{
    return GetPhysicalROMAddress(address, GetCurrentRomBankIndex(address));
}

u32 MemoryRule::GetPhysicalROMAddress(u16 address, u16 bank)
{
    if (address < 0x4000)
        return (u32)(0x4000 * bank) + address;

    return (u32)(0x4000 * bank) + (address & 0x3FFF);
}

u8* MemoryRule::GetRTCMemory()
{
    Debug("MemoryRule::GetRTCMemory not implemented");
    return NULL;
}

void MemoryRule::SaveState(std::ostream&)
{
    Debug("MemoryRule::SaveState not implemented");
}

void MemoryRule::LoadState(std::istream&)
{
    Debug("MemoryRule::LoadState not implemented");
}

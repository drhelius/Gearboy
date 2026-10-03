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

#include "RocketMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

// Logo verification data from NewRisingSun's CC0 Rocket Games mapper in hhugboy
// https://github.com/tzlion/hhugboy

static const u8 kRocketLogoXor[48] =
{
    0xDF, 0xCE, 0x97, 0x78, 0xCD, 0x2F, 0xF0, 0x0B, 0x0B, 0xEA, 0x78, 0x83,
    0x08, 0x1D, 0x9A, 0x45, 0x11, 0x2B, 0xE1, 0x11, 0xF8, 0x88, 0xF8, 0x8E,
    0xFE, 0x88, 0x2A, 0xC4, 0xFF, 0xFC, 0xD9, 0x87, 0x22, 0xAB, 0x67, 0x7D,
    0x77, 0x2C, 0xA8, 0xEE, 0xFF, 0x9B, 0x99, 0x91, 0xAA, 0x9B, 0x33, 0x3E
};

RocketMemoryRule::RocketMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

RocketMemoryRule::~RocketMemoryRule()
{
}

void RocketMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_LockMode = m_pMemory->IsBootromEnabled() ? LockModeDMG : LockModeUnlocked;
    m_UnlockCount = 0;
    m_ROMBank = 1;
    m_OuterBank = 0;
    UpdateBanks();
}

void RocketMemoryRule::UpdateBanks()
{
    int mask = MAX(m_pCartridge->GetROMBankCount(), 1) - 1;
    m_iCurrentROM0Bank = m_OuterBank & mask;
    m_iCurrentROMBank = (m_OuterBank | m_ROMBank) & mask;
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

u8 RocketMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        if (m_LockMode != LockModeUnlocked)
        {
            if (!m_pMemory->IsBootromRegistryEnabled())
                m_LockMode = LockModeUnlocked;
            else if (m_UnlockCount == 0x30)
            {
                m_LockMode = (m_LockMode == LockModeDMG) ? LockModeCGB : LockModeUnlocked;
                m_UnlockCount = 0;
            }

            m_UnlockCount++;
        }

        int offset = (address < 0x4000) ? m_CurrentROM0Address : m_CurrentROMAddress;
        u8 value = m_pCartridge->GetTheROM()[offset + (address & 0x3FFF)];

        if ((m_LockMode == LockModeCGB) && (address >= 0x0104) && (address < 0x0134))
            value ^= kRocketLogoXor[address - 0x0104];

        return value;
    }

    if ((address & 0xE000) == 0xA000)
    {
        size_t size = GetRamSize();
        return size ? GetRamBanks()[(address - 0xA000) & (size - 1)] : 0xFF;
    }

    return m_pMemory->Retrieve(address);
}

void RocketMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address == 0x3F00)
    {
        m_ROMBank = value ? value : 1;
        UpdateBanks();
        TraceMapperEvent(address, value);
    }
    else if (address == 0x3FC0)
    {
        m_OuterBank = static_cast<u8>(value << 4);
        UpdateBanks();
        TraceMapperEvent(address, value);
    }
    else if ((address & 0xE000) == 0xA000)
    {
        size_t size = GetRamSize();
        if (size > 0)
            GetRamBanks()[(address - 0xA000) & (size - 1)] = value;
    }
    else if (address >= 0x8000)
        m_pMemory->Load(address, value);
}

bool RocketMemoryRule::NeedsHighMemoryAccessNotifications()
{
    return true;
}

void RocketMemoryRule::NotifyHighMemoryWrite(u16 address, u8 value)
{
    UNUSED(value);

    if ((m_LockMode == LockModeDMG) && ((address & 0xE000) == 0xC000))
    {
        m_LockMode = LockModeCGB;
        m_UnlockCount = 0;
    }
}

void RocketMemoryRule::SaveRam(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(GetRamBanks()), GetRamSize());
}

bool RocketMemoryRule::LoadRam(std::istream& stream, s32 fileSize)
{
    size_t size = GetRamSize();
    if ((fileSize > 0) && (static_cast<size_t>(fileSize) != size))
        return false;

    stream.read(reinterpret_cast<char*>(GetRamBanks()), size);
    return !stream.fail();
}

size_t RocketMemoryRule::GetRamSize()
{
    if (m_pCartridge->GetRAMSize() == 0)
        return 0;

    return (m_pCartridge->GetRAMSize() == 1) ? 0x800 : 0x2000;
}

u8* RocketMemoryRule::GetRamBanks()
{
    return m_pMemory->GetMemoryMap() + 0xA000;
}

u8* RocketMemoryRule::GetCurrentRamBank()
{
    return GetRamBanks();
}

int RocketMemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

u8* RocketMemoryRule::GetRomBank0()
{
    return m_pCartridge->GetTheROM() + m_CurrentROM0Address;
}

int RocketMemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* RocketMemoryRule::GetCurrentRomBank1()
{
    return m_pCartridge->GetTheROM() + m_CurrentROMAddress;
}

int RocketMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void RocketMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_LockMode), sizeof(m_LockMode));
    stream.write(reinterpret_cast<const char*>(&m_UnlockCount), sizeof(m_UnlockCount));
    stream.write(reinterpret_cast<const char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.write(reinterpret_cast<const char*>(&m_OuterBank), sizeof(m_OuterBank));
}

void RocketMemoryRule::LoadState(std::istream& stream, u32)
{
    stream.read(reinterpret_cast<char*>(&m_LockMode), sizeof(m_LockMode));
    stream.read(reinterpret_cast<char*>(&m_UnlockCount), sizeof(m_UnlockCount));
    stream.read(reinterpret_cast<char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.read(reinterpret_cast<char*>(&m_OuterBank), sizeof(m_OuterBank));
    UpdateBanks();
}

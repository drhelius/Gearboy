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

#include "NTOldMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

NTOldMemoryRule::NTOldMemoryRule(Processor* pProcessor, Memory* pMemory,
        Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_bType2 = false;
    Reset(false);
}

NTOldMemoryRule::~NTOldMemoryRule()
{
}

void NTOldMemoryRule::SetVariant(bool type2)
{
    m_bType2 = type2;
    m_bRumbleEnabled = type2 && (m_pCartridge->GetROMBankCount() <= 32);
    m_bRumbleActive = false;
}

u8 NTOldMemoryRule::GetMapperType()
{
    return m_bType2 ? Cartridge::CartridgeNTOld2 : Cartridge::CartridgeNTOld1;
}

bool NTOldMemoryRule::MapsROMDirectly()
{
    return true;
}

void NTOldMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_bSwapMode = false;
    m_bConfigLocked = false;
    m_bRamEnabled = false;
    m_bRumbleEnabled = m_bType2 && (m_pCartridge->GetROMBankCount() <= 32);
    m_bRumbleActive = false;
    m_ROMBank = 1;
    m_ROMBase = 0;
    m_ROMBaseLatch = 0;
    m_ROMBankMask = MAX(m_pCartridge->GetROMBankCount(), 2) - 1;
    memset(m_RAM, 0xFF, sizeof(m_RAM));
    UpdateBanks();
}

void NTOldMemoryRule::SelectBank(u8 value)
{
    if (!m_bType2)
        value &= 0x1F;

    if (value == 0)
        value = 1;

    if (m_bSwapMode)
    {
        if (m_bType2)
            value = (value & 0xF8) | ((value & 0x01) << 2) | ((value & 0x06) >> 1);
        else
            value = (value & 0xE1) | ((value & 0x0A) << 1) | ((value & 0x14) >> 1);
    }

    m_ROMBank = value;
    UpdateBanks();
}

void NTOldMemoryRule::UpdateBanks()
{
    int mask = MAX(m_pCartridge->GetROMBankCount(), 2) - 1;
    m_iCurrentROM0Bank = (m_ROMBase << 1) & mask;
    m_iCurrentROMBank = (m_iCurrentROM0Bank + (m_ROMBank & m_ROMBankMask)) & mask;
}

u8 NTOldMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        int bank = address < 0x4000 ? m_iCurrentROM0Bank : m_iCurrentROMBank;
        return m_pCartridge->GetTheROM()[bank * 0x4000 + (address & 0x3FFF)];
    }

    if ((address & 0xE000) == 0xA000)
        return (m_bRamEnabled && GetRamSize() > 0) ? m_RAM[address & 0x1FFF] : 0xFF;

    return m_pMemory->Retrieve(address);
}

void NTOldMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x2000)
    {
        bool previous = m_bRamEnabled;
        m_bRamEnabled = ((value & 0x0A) == 0x0A) && (GetRamSize() > 0);
        if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            (*m_pRamChangedCallback)();
    }
    else if (address < 0x4000)
    {
        SelectBank(value);
        TraceMapperEvent(address, value);
        return;
    }
    else if (address < 0x6000)
    {
        if (m_bType2)
        {
            if (address == 0x5001)
                m_bRumbleEnabled = (value & 0x80) != 0;

            m_bRumbleActive = m_bRumbleEnabled && ((value & (m_bSwapMode ? 0x08 : 0x02)) != 0);
        }

        if (address >= 0x5000)
        {
            switch (address & 0x03)
            {
                case 1:
                    if (!m_bConfigLocked)
                        m_ROMBaseLatch = value & 0x3F;
                    break;
                case 2:
                    if (!m_bConfigLocked)
                    {
                        switch (value & 0x0F)
                        {
                            case 0x08: m_ROMBankMask = 0x0F; break;
                            case 0x0C: m_ROMBankMask = 0x07; break;
                            case 0x0E: m_ROMBankMask = 0x03; break;
                            case 0x0F: m_ROMBankMask = 0x01; break;
                            default: m_ROMBankMask = 0x1F; break;
                        }

                        if (m_ROMBaseLatch != 0)
                        {
                            m_ROMBase = m_ROMBaseLatch;
                            m_ROMBank = 1;
                            m_bConfigLocked = true;
                        }

                        UpdateBanks();
                    }
                    break;
                case 3:
                    m_bSwapMode = (value & 0x10) != 0;
                    SelectBank(m_ROMBank);
                    break;
            }
        }
    }
    else if (address < 0x8000)
        return;
    else if ((address & 0xE000) == 0xA000)
    {
        if (m_bRamEnabled && GetRamSize() > 0)
            m_RAM[address & 0x1FFF] = value;
        return;
    }
    else
    {
        m_pMemory->Load(address, value);
        return;
    }

    if (IsTraceMapperEventEnabled(TRACE_MAPPER_CONTROL))
    {
        LogTraceMapperEvent(address, value, TRACE_MAPPER_CONTROL,
                (m_bRamEnabled ? TRACE_MAPPER_FLAG_RAM_ENABLED : 0) |
                (m_bRumbleActive ? TRACE_MAPPER_FLAG_RUMBLE : 0), true);
    }
}

void NTOldMemoryRule::SaveRam(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(m_RAM), GetRamSize());
}

bool NTOldMemoryRule::LoadRam(std::istream& stream, s32 fileSize)
{
    size_t size = GetRamSize();
    if ((fileSize > 0) && (fileSize != (s32)size))
        return false;

    stream.read(reinterpret_cast<char*>(m_RAM), size);
    return !stream.fail();
}

size_t NTOldMemoryRule::GetRamSize()
{
    return (m_pCartridge->GetROMBankCount() > 32 || m_pCartridge->HasRam()) ? sizeof(m_RAM) : 0;
}

u8* NTOldMemoryRule::GetRamBanks()
{
    return m_RAM;
}

u8* NTOldMemoryRule::GetCurrentRamBank()
{
    return m_RAM;
}

int NTOldMemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

u8* NTOldMemoryRule::GetRomBank0()
{
    return m_pCartridge->GetTheROM() + m_iCurrentROM0Bank * 0x4000;
}

int NTOldMemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* NTOldMemoryRule::GetCurrentRomBank1()
{
    return m_pCartridge->GetTheROM() + m_iCurrentROMBank * 0x4000;
}

int NTOldMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void NTOldMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_bSwapMode), sizeof(m_bSwapMode));
    stream.write(reinterpret_cast<const char*>(&m_bConfigLocked), sizeof(m_bConfigLocked));
    stream.write(reinterpret_cast<const char*>(&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*>(&m_bRumbleEnabled), sizeof(m_bRumbleEnabled));
    stream.write(reinterpret_cast<const char*>(&m_bRumbleActive), sizeof(m_bRumbleActive));
    stream.write(reinterpret_cast<const char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.write(reinterpret_cast<const char*>(&m_ROMBase), sizeof(m_ROMBase));
    stream.write(reinterpret_cast<const char*>(&m_ROMBaseLatch), sizeof(m_ROMBaseLatch));
    stream.write(reinterpret_cast<const char*>(&m_ROMBankMask), sizeof(m_ROMBankMask));
    stream.write(reinterpret_cast<const char*>(m_RAM), sizeof(m_RAM));
}

void NTOldMemoryRule::LoadState(std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&m_bSwapMode), sizeof(m_bSwapMode));
    stream.read(reinterpret_cast<char*>(&m_bConfigLocked), sizeof(m_bConfigLocked));
    stream.read(reinterpret_cast<char*>(&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*>(&m_bRumbleEnabled), sizeof(m_bRumbleEnabled));
    stream.read(reinterpret_cast<char*>(&m_bRumbleActive), sizeof(m_bRumbleActive));
    stream.read(reinterpret_cast<char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.read(reinterpret_cast<char*>(&m_ROMBase), sizeof(m_ROMBase));
    stream.read(reinterpret_cast<char*>(&m_ROMBaseLatch), sizeof(m_ROMBaseLatch));
    stream.read(reinterpret_cast<char*>(&m_ROMBankMask), sizeof(m_ROMBankMask));
    stream.read(reinterpret_cast<char*>(m_RAM), sizeof(m_RAM));
    m_ROMBase &= 0x3F;
    m_ROMBaseLatch &= 0x3F;
    UpdateBanks();
}

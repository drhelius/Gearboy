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

#include "NTNewMemoryRule.h"
#include "Cartridge.h"

NTNewMemoryRule::NTNewMemoryRule(Processor* pProcessor, Memory* pMemory, Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio) :
        MBC5MemoryRule(pProcessor, pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

NTNewMemoryRule::~NTNewMemoryRule()
{
}

bool NTNewMemoryRule::MapsROMDirectly()
{
    return false;
}

u8 NTNewMemoryRule::GetMapperType()
{
    return Cartridge::CartridgeNTNew;
}

void NTNewMemoryRule::Reset(bool bCGB)
{
    MBC5MemoryRule::Reset(bCGB);
    m_bSplitMode = false;
    m_ROMBank = 1;
    m_iCurrentROMBankA = 2;
    m_iCurrentROMBankB = 3;
}

u8 NTNewMemoryRule::PerformRead(u16 address)
{
    if ((address >= 0x4000) && (address < 0x8000))
        return m_pCartridge->GetTheROM()[GetPhysicalROMAddress(address)];

    return MBC5MemoryRule::PerformRead(address);
}

void NTNewMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (((address & 0xFF00) == 0x1400) && (value == 0x55))
    {
        m_bSplitMode = true;
        TraceMapperEvent(address, value);
        return;
    }

    if ((address >= 0x2000) && (address < 0x4000))
    {
        if (m_bSplitMode && (((address & 0xFF00) == 0x2000) || ((address & 0xFF00) == 0x2400)))
        {
            int bank = value & ((m_pCartridge->GetROMBankCount() * 2) - 1);
            if (bank < 2)
                bank += 2;

            if ((address & 0xFF00) == 0x2000)
                m_iCurrentROMBankA = bank;
            else
                m_iCurrentROMBankB = bank;
        }
        else
        {
            if (address < 0x3000)
                m_ROMBank = (m_ROMBank & 0x100) | value;
            else
                m_ROMBank = (m_ROMBank & 0xFF) | ((value & 0x01) << 8);

            int bank = m_ROMBank & (m_pCartridge->GetROMBankCount() - 1);
            m_iCurrentROMBankA = bank * 2;
            m_iCurrentROMBankB = m_iCurrentROMBankA + 1;
        }

        TraceMapperEvent(address, value);
        return;
    }

    MBC5MemoryRule::PerformWrite(address, value);
}

size_t NTNewMemoryRule::GetRamSize()
{
    return 0x8000;
}

u8* NTNewMemoryRule::GetCurrentRomBank1()
{
    u8* rom = m_pCartridge->GetTheROM();
    memcpy(m_ROMView, rom + m_iCurrentROMBankA * 0x2000, 0x2000);
    memcpy(m_ROMView + 0x2000, rom + m_iCurrentROMBankB * 0x2000, 0x2000);
    return m_ROMView;
}

int NTNewMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBankA;
}

u16 NTNewMemoryRule::GetCurrentRomBankIndex(u16 address)
{
    if (address < 0x4000)
        return 0;

    return (u16)((address < 0x6000) ? m_iCurrentROMBankA : m_iCurrentROMBankB);
}

u32 NTNewMemoryRule::GetPhysicalROMAddress(u16 address)
{
    return GetPhysicalROMAddress(address, GetCurrentRomBankIndex(address));
}

u32 NTNewMemoryRule::GetPhysicalROMAddress(u16 address, u16 bank)
{
    if (address < 0x4000)
        return address;

    return (u32)bank * 0x2000 + (address & 0x1FFF);
}

void NTNewMemoryRule::SaveState(std::ostream& stream)
{
    MBC5MemoryRule::SaveState(stream);
    stream.write(reinterpret_cast<const char*>(&m_bSplitMode), sizeof(m_bSplitMode));
    stream.write(reinterpret_cast<const char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBankA), sizeof(m_iCurrentROMBankA));
    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBankB), sizeof(m_iCurrentROMBankB));
}

void NTNewMemoryRule::LoadState(std::istream& stream)
{
    MBC5MemoryRule::LoadState(stream);
    stream.read(reinterpret_cast<char*>(&m_bSplitMode), sizeof(m_bSplitMode));
    stream.read(reinterpret_cast<char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBankA), sizeof(m_iCurrentROMBankA));
    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBankB), sizeof(m_iCurrentROMBankB));

    m_ROMBank &= 0x1FF;
    int bank_mask = (m_pCartridge->GetROMBankCount() * 2) - 1;
    m_iCurrentROMBankA &= bank_mask;
    m_iCurrentROMBankB &= bank_mask;
}

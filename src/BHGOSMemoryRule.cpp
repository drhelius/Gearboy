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

#include "BHGOSMemoryRule.h"
#include "Memory.h"
#include "Cartridge.h"

BHGOSMemoryRule::BHGOSMemoryRule(Memory* pMemory, Cartridge* pCartridge) : MemoryRule(pMemory, pCartridge)
{
    Reset(false);
}

BHGOSMemoryRule::~BHGOSMemoryRule()
{
}

void BHGOSMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_ROMBank = 1;
    m_ROMBase = 0;
    m_RAMBank = 0;
    m_UnlockState = 0;
    memset(m_RAM, 0xFF, sizeof(m_RAM));
    UpdateBanks();
}

void BHGOSMemoryRule::UpdateBanks()
{
    int mask = MAX(m_pCartridge->GetROMBankCount(), 1) - 1;
    m_iCurrentROM0Bank = (m_ROMBase << 1) & mask;
    m_iCurrentROMBank = (m_iCurrentROM0Bank + m_ROMBank) & mask;
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

u8 BHGOSMemoryRule::PerformRead(u16 address)
{
    if (address < 0x8000)
    {
        int offset = (address < 0x4000) ? m_CurrentROM0Address : m_CurrentROMAddress;
        return m_pCartridge->GetTheROM()[offset + (address & 0x3FFF)];
    }

    if ((address & 0xE000) == 0xA000)
        return m_RAM[(m_RAMBank * 0x2000) + (address & 0x1FFF)];

    return m_pMemory->Retrieve(address);
}

void BHGOSMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (value == 0x5A)
                m_UnlockState = 1;
            else if ((value == 0xA5) && (m_UnlockState == 1))
                m_UnlockState = 2;
            else
                m_UnlockState = 0;

            TraceMapperEvent(address, value);
            break;
        }
        case 0x2000:
        {
            if (address < 0x3000)
            {
                m_ROMBank = value ? value : 1;
                UpdateBanks();
            }
            TraceMapperEvent(address, value);
            break;
        }
        case 0x4000:
        {
            m_RAMBank = value & 0x03;
            TraceMapperEvent(address, value);
            break;
        }
        case 0x6000:
        {
            // Each configuration write needs the menu's $5A/$A5 unlock sequence
            if ((m_UnlockState == 2) && (address == 0x6000))
            {
                m_ROMBase = value;
                m_ROMBank = 1;
                UpdateBanks();
            }
            m_UnlockState = 0;
            TraceMapperEvent(address, value);
            break;
        }
        case 0xA000:
        {
            m_RAM[(m_RAMBank * 0x2000) + (address & 0x1FFF)] = value;
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void BHGOSMemoryRule::SaveRam(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(m_RAM), sizeof(m_RAM));
}

bool BHGOSMemoryRule::LoadRam(std::istream& stream, s32 fileSize)
{
    if ((fileSize > 0) && (fileSize != (s32)sizeof(m_RAM)))
        return false;

    stream.read(reinterpret_cast<char*>(m_RAM), sizeof(m_RAM));
    return !stream.fail();
}

size_t BHGOSMemoryRule::GetRamSize()
{
    return sizeof(m_RAM);
}

u8* BHGOSMemoryRule::GetRamBanks()
{
    return m_RAM;
}

u8* BHGOSMemoryRule::GetCurrentRamBank()
{
    return m_RAM + (m_RAMBank * 0x2000);
}

int BHGOSMemoryRule::GetCurrentRamBankIndex()
{
    return m_RAMBank;
}

u8* BHGOSMemoryRule::GetRomBank0()
{
    return m_pCartridge->GetTheROM() + m_CurrentROM0Address;
}

int BHGOSMemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* BHGOSMemoryRule::GetCurrentRomBank1()
{
    return m_pCartridge->GetTheROM() + m_CurrentROMAddress;
}

int BHGOSMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void BHGOSMemoryRule::SaveState(std::ostream& stream)
{
    stream.write(reinterpret_cast<const char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.write(reinterpret_cast<const char*>(&m_ROMBase), sizeof(m_ROMBase));
    stream.write(reinterpret_cast<const char*>(&m_RAMBank), sizeof(m_RAMBank));
    stream.write(reinterpret_cast<const char*>(&m_UnlockState), sizeof(m_UnlockState));
    stream.write(reinterpret_cast<const char*>(m_RAM), sizeof(m_RAM));
}

void BHGOSMemoryRule::LoadState(std::istream& stream, u32)
{
    stream.read(reinterpret_cast<char*>(&m_ROMBank), sizeof(m_ROMBank));
    stream.read(reinterpret_cast<char*>(&m_ROMBase), sizeof(m_ROMBase));
    stream.read(reinterpret_cast<char*>(&m_RAMBank), sizeof(m_RAMBank));
    stream.read(reinterpret_cast<char*>(&m_UnlockState), sizeof(m_UnlockState));
    stream.read(reinterpret_cast<char*>(m_RAM), sizeof(m_RAM));
    m_RAMBank &= 0x03;
    UpdateBanks();
}

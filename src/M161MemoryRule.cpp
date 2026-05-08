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

#include "M161MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

M161MemoryRule::M161MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

M161MemoryRule::~M161MemoryRule()
{
}

void M161MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentROMBank = 0;
    m_CurrentROMAddress = 0;
    m_bLocked = false;
}

void M161MemoryRule::UpdateBanks()
{
    int bankCount = m_pCartridge->GetTotalSize() / 0x8000;

    if (bankCount <= 0)
        bankCount = 1;

    m_iCurrentROMBank &= 0x07;
    m_iCurrentROMBank %= bankCount;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x8000;
}

u8 M161MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[address + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            Debug("--> ** Attempting to read from RAM without ram in cart %X", address);
            return 0xFF;
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void M161MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        case 0x4000:
        case 0x6000:
        {
            if (!m_bLocked)
            {
                m_iCurrentROMBank = value & 0x07;
                UpdateBanks();
                m_bLocked = true;
                TraceBankSwitch(address, static_cast<u8>(m_iCurrentROMBank));
            }
            break;
        }
        case 0xA000:
        {
            Debug("--> ** Attempting to write to RAM without ram in cart  %X %X", address, value);
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

u8* M161MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int M161MemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROMBank << 1;
}

u8* M161MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress + 0x4000];
}

int M161MemoryRule::GetCurrentRomBank1Index()
{
    return (m_iCurrentROMBank << 1) + 1;
}

void M161MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_bLocked), sizeof(m_bLocked));
}

void M161MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_bLocked), sizeof(m_bLocked));
    UpdateBanks();
}
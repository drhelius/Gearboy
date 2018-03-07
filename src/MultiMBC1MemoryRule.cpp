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

#include "MultiMBC1MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MultiMBC1MemoryRule::MultiMBC1MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

MultiMBC1MemoryRule::~MultiMBC1MemoryRule()
{

}

void MultiMBC1MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iMode = 0;
    m_iCurrentROMBank = 1;
    m_iFinalROMBank0 = 0;
    m_iFinalROMBank = 1;
    m_bRamEnabled = false;
}

u8 MultiMBC1MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[address + (0x4000 * m_iFinalROMBank0)];
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + (0x4000 * m_iFinalROMBank)];
        }
        case 0xA000:
        {
            if (m_bRamEnabled)
            {
                return m_pMemory->Retrieve(address);
            }
            else
            {
                Log("--> ** Attempting to read from disabled ram %X", address);
                return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void MultiMBC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            bool previous = m_bRamEnabled;
            m_bRamEnabled = ((value & 0x0F) == 0x0A);

            if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            {
                (*m_pRamChangedCallback)();
            }
            break;
        }
        case 0x2000:
        {
            if (m_iMode == 0)
            {
                m_iFinalROMBank = (m_iCurrentROMBank & 0x1F) ? m_iCurrentROMBank : (m_iCurrentROMBank | 1);
                m_iFinalROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            }
            else
            {
                int rombank = ((m_iCurrentROMBank >> 1) & 0x30) | (m_iCurrentROMBank & 0xF);
                m_iFinalROMBank = (rombank & 0x1F) ? rombank : (rombank | 1);
            }
            break;
        }
        case 0x4000:
        {
            m_iCurrentROMBank = ((value << 5) & 0x60) | (m_iCurrentROMBank & 0x1F);
            SetRomBank();
            break;
        }
        case 0x6000:
        {
            m_iMode = value & 0x01;
            SetRomBank();
            break;
        }
        case 0xA000:
        {
            if (m_bRamEnabled)
            {
                m_pMemory->Load(address, value);
            }
            else
            {
                Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
            }
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void MultiMBC1MemoryRule::SetRomBank()
{
    if (m_iMode == 0)
    {
        m_iFinalROMBank0 = 0;
        m_iFinalROMBank = (m_iCurrentROMBank & 0x1F) ? m_iCurrentROMBank : (m_iCurrentROMBank | 1);
        m_iFinalROMBank &= (m_pCartridge->GetROMBankCount() - 1);
    }
    else
    {
        int rombank = ((m_iCurrentROMBank >> 1) & 0x30) | (m_iCurrentROMBank & 0xF);
        m_iFinalROMBank0 = rombank & 0x30;
        m_iFinalROMBank = (rombank & 0x1F) ? rombank : (rombank | 1);
    }
}

size_t MultiMBC1MemoryRule::GetRamSize()
{
    return 0x2000;
}

u8* MultiMBC1MemoryRule::GetRamBanks()
{
    return m_pMemory->GetMemoryMap() + 0xA000;
}

u8* MultiMBC1MemoryRule::GetCurrentRamBank()
{
    return m_pMemory->GetMemoryMap() + 0xA000;
}

u8* MultiMBC1MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[0x4000 * m_iFinalROMBank0];
}

u8* MultiMBC1MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[0x4000 * m_iFinalROMBank];
}

void MultiMBC1MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iMode), sizeof(m_iMode));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_iFinalROMBank0), sizeof(m_iFinalROMBank0));
    stream.write(reinterpret_cast<const char*> (&m_iFinalROMBank), sizeof(m_iFinalROMBank));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
}

void MultiMBC1MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iMode), sizeof(m_iMode));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_iFinalROMBank0), sizeof(m_iFinalROMBank0));
    stream.read(reinterpret_cast<char*> (&m_iFinalROMBank), sizeof(m_iFinalROMBank));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
}

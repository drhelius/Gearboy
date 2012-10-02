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

u8 MultiMBC1MemoryRule::PerformRead(u16 address)
{
    if (address < 0x4000)
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[address + (0x4000 * m_iFinalROMBank0)];
    }
    else if (address >= 0x4000 && address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x4000) + (0x4000 * m_iFinalROMBank)];
    }
    else if (address >= 0xA000 && address < 0xC000)
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
    else
        return m_pMemory->Retrieve(address);
}

void MultiMBC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x2000)
    {
        m_bRamEnabled = ((value & 0x0F) == 0x0A);
    }
    else if (address >= 0x2000 && address < 0x4000)
    {
        m_iCurrentROMBank = (m_iCurrentROMBank & 0x60) | (value & 0x1F);

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
    }
    else if (address >= 0x4000 && address < 0x6000)
    {
        m_iCurrentROMBank = ((value << 5) & 0x60) | (m_iCurrentROMBank & 0x1F);
        SetRomBank();
    }
    else if (address >= 0x6000 && address < 0x8000)
    {
        m_iMode = value & 0x01;
        SetRomBank();
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_bRamEnabled)
        {
            m_pMemory->Load(address, value);
        }
        else
        {
            Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
        }
    }
    else
        m_pMemory->Load(address, value);
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



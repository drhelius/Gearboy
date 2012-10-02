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

#include "MBC2MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MBC2MemoryRule::MBC2MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

MBC2MemoryRule::~MBC2MemoryRule()
{
}

u8 MBC2MemoryRule::PerformRead(u16 address)
{
    if (address >= 0x4000 && address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x4000) + (0x4000 * m_iCurrentROMBank)];
    }
    else if (address >= 0xA000 && address < 0xA200)
    {
        if (m_bRamEnabled)
            return m_pMemory->Retrieve(address);
        else
        {
            Log("--> ** Attempting to read from disabled ram %X", address);
            return 0xFF;
        }
    }
    else if (address >= 0xA200 && address < 0xC000)
    {
        Log("--> ** Attempting to read from non usable address %X", address);
        return 0x00;
    }
    else
        return m_pMemory->Retrieve(address);
}

void MBC2MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x1000)
    {
        m_bRamEnabled = (value & 0x0F) == 0x0A;
    }
    else if (address >= 0x1000 && address < 0x2100)
    {
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0x2100 && address < 0x2200)
    {
        m_iCurrentROMBank = value & 0x0F;
        if (m_iCurrentROMBank == 0)
            m_iCurrentROMBank = 1;
        m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
    }
    else if (address >= 0x2200 && address < 0x8000)
    {
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0xA000 && address < 0xA200)
    {
        if (m_bRamEnabled)
        {
            m_pMemory->Load(address, value & 0x0F);
        }
        else
        {
            Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
        }
    }
    else if (address >= 0xA200 && address < 0xC000)
    {
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else
        m_pMemory->Load(address, value);
}

void MBC2MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
}

void MBC2MemoryRule::SaveRam(std::ofstream &file)
{
    Log("MBC2MemoryRule save RAM...");
    
    for (int i = 0xA000; i < 0xA200; i++)
    {
        u8 ram_byte = 0;
        ram_byte = m_pMemory->Retrieve(i);
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }
    
    Log("MBC2MemoryRule save RAM done");
}

void MBC2MemoryRule::LoadRam(std::ifstream &file)
{
    Log("MBC2MemoryRule load RAM...");
    
    for (int i = 0xA000; i < 0xA200; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pMemory->Load(i, ram_byte);
    }
    
    Log("MBC2MemoryRule load RAM done");
}

int MBC2MemoryRule::GetRamBanksSize()
{
    return 0x200;
}




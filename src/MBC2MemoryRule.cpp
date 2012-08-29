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
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
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
    else if (m_bCGB && (address >= 0x8000 && address < 0xA000))
    {
        return m_pMemory->ReadCGBLCDRAM(address, false);
    }
    else if (address >= 0xA000 && address < 0xA200)
    {
        if (m_bRamEnabled)
            return m_pMemory->Retrieve(address);
        else
        {
            Log("--> ** Attempting to read from disabled ram %X", address);
            return 0x00;
        }
    }
    else if (address >= 0xA200 && address < 0xC000)
    {
        Log("--> ** Attempting to read from non usable address %X", address);
        return 0x00;
    }
    else if (m_bCGB && (address >= 0xD000 && address < 0xE000))
    {
        return m_pMemory->ReadCGBWRAM(address);
    }
    else if (address >= 0xFEA0 && address < 0xFF00)
    {
        // Empty area - GBC allows reading/writing to this area
        if (m_bCGB)
            return m_pMemory->Retrieve(address);
        else
            return ((((address + ((address >> 4) - 0x0FEA)) >> 2) & 1) ? 0x00 : 0xFF);
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
    }
    else if (address >= 0x2200 && address < 0x8000)
    {
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else if (m_bCGB && (address >= 0x8000 && address < 0xA000))
    {
        m_pMemory->WriteCGBLCDRAM(address, value);
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
    else if (address >= 0xC000 && address < 0xDE00)
    {
        if (m_bCGB && (address >= 0xD000))
        {
            m_pMemory->WriteCGBWRAM(address, value);
            m_pMemory->Load(address + 0x2000, value);
        }
        else
        {
            // Echo of 8K internal RAM
            m_pMemory->Load(address + 0x2000, value);
            m_pMemory->Load(address, value);
        }
    }
    else if (m_bCGB && (address >= 0xDE00 && address < 0xE000))
    {
        m_pMemory->WriteCGBWRAM(address, value);
    }
    else if (address >= 0xE000 && address < 0xFE00)
    {
        if (m_bCGB && (address >= 0xF000))
        {
            m_pMemory->WriteCGBWRAM(address - 0x2000, value);
            m_pMemory->Load(address, value);
        }
        else
        {
            // Echo of 8K internal RAM
            m_pMemory->Load(address - 0x2000, value);
            m_pMemory->Load(address, value);
        }
    }
    else if (address >= 0xFEA0 && address < 0xFF00)
    {
        // Empty area - GBC allows reading/writing to this area
        m_pMemory->Load(address, value);
    }
    else
    {
        m_pMemory->Load(address, value);
    }
}

void MBC2MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentROMBank = 0;
    m_bRamEnabled = false;
}




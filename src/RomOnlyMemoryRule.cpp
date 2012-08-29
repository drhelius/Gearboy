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

#include "RomOnlyMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

RomOnlyMemoryRule::RomOnlyMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
}

RomOnlyMemoryRule::~RomOnlyMemoryRule()
{
}

u8 RomOnlyMemoryRule::PerformRead(u16 address)
{
    if (m_bCGB && (address >= 0x8000 && address < 0xA000))
    {
        return m_pMemory->ReadCGBLCDRAM(address, false);
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
            return m_pMemory->Retrieve(address);
        else
        {
            Log("--> ** Attempting to read from non usable address %X", address);
            return 0x00;
        }
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
            return ((((address + ((address >> 4) - 0x0FEA)) >> 2) & 1) ? 0x00 : 0xFF );
    }
    else
        return m_pMemory->Retrieve(address);
}

void RomOnlyMemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        // ROM
        Log("--> ** Attempting to write on ROM address %X %X", address, value);
    }
    else if (m_bCGB && (address >= 0x8000 && address < 0xA000))
    {
        m_pMemory->WriteCGBLCDRAM(address, value);
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
        {
            m_pMemory->Load(address, value);
        }
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

void RomOnlyMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}


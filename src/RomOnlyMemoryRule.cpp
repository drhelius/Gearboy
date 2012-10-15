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
    Reset(false);
}

RomOnlyMemoryRule::~RomOnlyMemoryRule()
{
}

u8 RomOnlyMemoryRule::PerformRead(u16 address)
{
    if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
            return m_pMemory->Retrieve(address);
        else
        {
            Log("--> ** Attempting to read from RAM without ram in cart %X", address);
            return 0xFF;
        }
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
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
        {
            m_pMemory->Load(address, value);
        }
        else
        {
            Log("--> ** Attempting to write to RAM without ram in cart  %X %X", address, value);
        }
    }
    else
        m_pMemory->Load(address, value);
}

void RomOnlyMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
}

void RomOnlyMemoryRule::SaveRam(std::ofstream &file)
{
    Log("RomOnlyMemoryRule save RAM...");

    for (int i = 0xA000; i < 0xC000; i++)
    {
        u8 ram_byte = 0;
        ram_byte = m_pMemory->Retrieve(i);
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Log("RomOnlyMemoryRule save RAM done");
}

void RomOnlyMemoryRule::LoadRam(std::ifstream &file)
{
    Log("RomOnlyMemoryRule load RAM...");

    for (int i = 0xA000; i < 0xC000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pMemory->Load(i, ram_byte);
    }

    Log("RomOnlyMemoryRule load RAM done");
}

int RomOnlyMemoryRule::GetRamBanksSize()
{
    return 0x2000;
}


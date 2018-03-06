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

void RomOnlyMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
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

void RomOnlyMemoryRule::SaveRam(std::ostream &file)
{
    Log("RomOnlyMemoryRule save RAM...");

    for (int i = 0xA000; i < 0xC000; i++)
    {
        u8 ram_byte = m_pMemory->Retrieve(i);
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Log("RomOnlyMemoryRule save RAM done");
}

bool RomOnlyMemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Log("RomOnlyMemoryRule load RAM...");

    if ((fileSize > 0) && (fileSize != 0x2000))
    {
        Log("RomOnlyMemoryRule incorrect size. Expected: %d Found: %d", 0x2000, fileSize);
        return false;
    }

    for (int i = 0xA000; i < 0xC000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pMemory->Load(i, ram_byte);
    }

    Log("RomOnlyMemoryRule load RAM done");

    return true;
}

size_t RomOnlyMemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

u8* RomOnlyMemoryRule::GetRamBanks()
{
    return m_pMemory->GetMemoryMap() + 0xA000;
}

u8* RomOnlyMemoryRule::GetCurrentRamBank()
{
    if (m_pCartridge->GetRAMSize() > 0)
        return m_pMemory->GetMemoryMap() + 0xA000;
    else
        return NULL;
}

u8* RomOnlyMemoryRule::GetCurrentRomBank1()
{
    return m_pMemory->GetMemoryMap() + 0x4000;
}

u8* RomOnlyMemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

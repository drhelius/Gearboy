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

#include "MBC5MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MBC5MemoryRule::MBC5MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[0x20000];
    Reset(false);
}

MBC5MemoryRule::~MBC5MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

u8 MBC5MemoryRule::PerformRead(u16 address)
{
    if (address >= 0x4000 && address < 0x8000)
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return pROM[(address - 0x4000) + (0x4000 * m_iCurrentROMBank)];
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_bRamEnabled)
        {
            return m_pRAMBanks[(address - 0xA000) + (0x2000 * m_iCurrentRAMBank)];
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

void MBC5MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (address < 0x2000)
    {
        if (m_pCartridge->GetRAMSize() > 0)
            m_bRamEnabled = (value & 0x0F) == 0x0A;
    }
    else if (address >= 0x2000 && address < 0x3000)
    {
        m_iCurrentROMBank = value | (m_iCurrentROMBankHi << 8);
        m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
    }
    else if (address >= 0x3000 && address < 0x4000)
    {
        m_iCurrentROMBankHi = value & 0x01;
        m_iCurrentROMBank = (m_iCurrentROMBank & 0xFF) | (m_iCurrentROMBankHi << 8);
        m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
    }
    else if (address >= 0x4000 && address < 0x6000)
    {
        m_iCurrentRAMBank = value & 0x0F;
        m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
    }
    else if (address >= 0x6000 && address < 0x8000)
    {
        Log("--> ** Attempting to write on non usable address %X %X", address, value);
    }
    else if (address >= 0xA000 && address < 0xC000)
    {
        if (m_bRamEnabled)
        {
            m_pRAMBanks[(address - 0xA000) + (0x2000 * m_iCurrentRAMBank)] = value;
        }
        else
        {
            Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
        }
    }
    else
        m_pMemory->Load(address, value);
}

void MBC5MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_iCurrentROMBankHi = 0;
    m_bRamEnabled = false;
    for (int i = 0; i < 0x20000; i++)
        m_pRAMBanks[i] = 0xFF;
}


void MBC5MemoryRule::SaveRam(std::ofstream &file)
{
    Log("MBC5MemoryRule save RAM...");
    
    for (int i = 0; i < 0x20000; i++)
    {
        u8 ram_byte = 0;
        ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }
    
    Log("MBC5MemoryRule save RAM done");
}

void MBC5MemoryRule::LoadRam(std::ifstream &file)
{
    Log("MBC5MemoryRule load RAM...");
    
    for (int i = 0; i < 0x20000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }
    
    Log("MBC5MemoryRule load RAM done");
}

int MBC5MemoryRule::GetRamBanksSize()
{
    return 0x20000;
}


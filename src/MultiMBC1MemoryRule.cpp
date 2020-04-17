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
#include "MBC1MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MultiMBC1MemoryRule::MultiMBC1MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio, MBC1MemoryRule* pMBC1MemoryRule) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pMBC1MemoryRule = pMBC1MemoryRule;
    Reset(false);
}

MultiMBC1MemoryRule::~MultiMBC1MemoryRule()
{

}

void MultiMBC1MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iMulticartMode = 0;
    m_iROMBankHi = 0;
    m_iROMBankLo = 1;
}

u8 MultiMBC1MemoryRule::PerformRead(u16 address)
{
    if (m_iMulticartMode == 0)
        return m_pMBC1MemoryRule->PerformRead(address);

    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            u16 new_addr = ((m_iROMBankHi << 4) * 0x4000) + (address & 0x3fff);
            return pROM[new_addr];
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            u16 new_addr = ((m_iROMBankHi << 4 | m_iROMBankLo) * 0x4000) + (address & 0x3fff);
            return pROM[new_addr];
        }
        default:
        {
            return 0xFF;
        }
    }
}

void MultiMBC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    if (m_iMulticartMode == 0)
    {
        if ((address & 0xE000) == 0x6000)
        {
            m_iMulticartMode = value & 0x01;
            Log("--> ** Change Multicart mode %X %X", address, value);
        }
        else
            m_pMBC1MemoryRule->PerformWrite(address, value);
    }
    else
    {
        switch (address & 0xE000)
        {
            case 0x2000:
            {
                m_iROMBankLo = value & 0x15;
                Log("--> ** Set bank LO %X %X %X", address, value, m_iROMBankLo);
                break;
            }
            case 0x4000:
            {
                m_iROMBankHi = value & 0x03;
                Log("--> ** Set bank HI %X %X %X", address, value, m_iROMBankHi);
                break;
            }
            case 0x6000:
            {
                m_iMulticartMode = value & 0x01;
                Log("--> ** Change Multicart mode %X %X", address, value);
                break;
            }
            default:
            {
                Log("--> ** Attempting to write on invalid address %X %X", address, value);
                break;
            }
        }
    }
}

size_t MultiMBC1MemoryRule::GetRamSize()
{
    if (m_iMulticartMode != 0)
        return 0;
    else
        return m_pMBC1MemoryRule->GetRamSize();
}

u8* MultiMBC1MemoryRule::GetRamBanks()
{
    if (m_iMulticartMode != 0)
        return 0;
    else
        return m_pMBC1MemoryRule->GetRamBanks(); 
}

u8* MultiMBC1MemoryRule::GetCurrentRamBank()
{
    if (m_iMulticartMode != 0)
        return 0;
    else
        return m_pMBC1MemoryRule->GetCurrentRamBank(); 
}

u8* MultiMBC1MemoryRule::GetRomBank0()
{
    if (m_iMulticartMode != 0)
        return 0;
    else
        return m_pMBC1MemoryRule->GetRomBank0(); 
}

u8* MultiMBC1MemoryRule::GetCurrentRomBank1()
{
    if (m_iMulticartMode != 0)
        return 0;
    else
        return m_pMBC1MemoryRule->GetCurrentRomBank1();
}

void MultiMBC1MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iMulticartMode), sizeof(m_iMulticartMode));
    stream.write(reinterpret_cast<const char*> (&m_iROMBankHi), sizeof(m_iROMBankHi));
    stream.write(reinterpret_cast<const char*> (&m_iROMBankLo), sizeof(m_iROMBankLo));
    
    m_pMBC1MemoryRule->SaveState(stream);
}

void MultiMBC1MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iMulticartMode), sizeof(m_iMulticartMode));
    stream.read(reinterpret_cast<char*> (&m_iROMBankHi), sizeof(m_iROMBankHi));
    stream.read(reinterpret_cast<char*> (&m_iROMBankLo), sizeof(m_iROMBankLo));
    
    m_pMBC1MemoryRule->LoadState(stream);
}

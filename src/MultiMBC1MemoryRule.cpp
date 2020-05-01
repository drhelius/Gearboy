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
    m_iMulticartMode = 0;
    m_iROMBankHi = 0;
    m_iROMBankLo = 1;
    SetROMBanks();
}

u8 MultiMBC1MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            u8* pROM = m_pCartridge->GetTheROM();

            if (m_iMulticartMode == 0)
            {
                return pROM[address];
            }
            else
            {
                int bank_addr = (m_iMBC1MBank_0 * 0x4000) + address;
                return pROM[bank_addr];
            }
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            
            if (m_iMulticartMode == 0)
            {
                int bank_addr = (m_iMBC1Bank_1 * 0x4000) + (address & 0x3FFF);
                return pROM[bank_addr];
            }
            else
            {
                int bank_addr = (m_iMBC1MBank_1 * 0x4000) + (address & 0x3FFF);
                return pROM[bank_addr];
            }
        }
        default:
        {
            return 0xFF;
        }
    }
}

void MultiMBC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x2000:
        {
            m_iROMBankLo = value & 0x1F;
            SetROMBanks();
            break;
        }
        case 0x4000:
        {
            m_iROMBankHi = value & 0x03;
            SetROMBanks();
            break;
        }
        case 0x6000:
        {
            m_iMulticartMode = value & 0x01;
            break;
        }
        default:
        {
            Log("--> ** Attempting to write on invalid address %X %X", address, value);
            break;
        }
    }
}

void MultiMBC1MemoryRule::SetROMBanks()
{
    int full_bank = (m_iROMBankHi << 5) | m_iROMBankLo;

    m_iMBC1Bank_1 = full_bank;

    if (full_bank == 0x00 || full_bank == 0x20 || full_bank == 0x40 || full_bank == 0x60)
        m_iMBC1Bank_1 = full_bank + 1;
    
    m_iMBC1MBank_0 = ((full_bank >> 1) & 0x30);
    m_iMBC1MBank_1 = ((full_bank >> 1) & 0x30) | (full_bank & 0x0F);
}

size_t MultiMBC1MemoryRule::GetRamSize()
{
    return 0;
}

u8* MultiMBC1MemoryRule::GetRamBanks()
{
    return 0;
}

u8* MultiMBC1MemoryRule::GetCurrentRamBank()
{
    return m_pMemory->GetMemoryMap() + 0xA000;
}

int MultiMBC1MemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

u8* MultiMBC1MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();

    if (m_iMulticartMode == 0)
        return pROM;
    else
        return pROM + (m_iMBC1MBank_0 * 0x4000);
}

int MultiMBC1MemoryRule::GetCurrentRomBank0Index()
{
    return m_iMBC1MBank_0;
}

u8* MultiMBC1MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();

    if (m_iMulticartMode == 0)
        return &pROM[m_iMBC1Bank_1 * 0x4000];
    else
        return &pROM[m_iMBC1MBank_1 * 0x4000];
}

int MultiMBC1MemoryRule::GetCurrentRomBank1Index()
{
    return m_iMBC1Bank_1;
}

void MultiMBC1MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iMulticartMode), sizeof(m_iMulticartMode));
    stream.write(reinterpret_cast<const char*> (&m_iROMBankHi), sizeof(m_iROMBankHi));
    stream.write(reinterpret_cast<const char*> (&m_iROMBankLo), sizeof(m_iROMBankLo));
    stream.write(reinterpret_cast<const char*> (&m_iMBC1Bank_1), sizeof(m_iMBC1Bank_1));
    stream.write(reinterpret_cast<const char*> (&m_iMBC1MBank_0), sizeof(m_iMBC1MBank_0));
    stream.write(reinterpret_cast<const char*> (&m_iMBC1MBank_1), sizeof(m_iMBC1MBank_1));
}

void MultiMBC1MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iMulticartMode), sizeof(m_iMulticartMode));
    stream.read(reinterpret_cast<char*> (&m_iROMBankHi), sizeof(m_iROMBankHi));
    stream.read(reinterpret_cast<char*> (&m_iROMBankLo), sizeof(m_iROMBankLo));
    stream.read(reinterpret_cast<char*> (&m_iMBC1Bank_1), sizeof(m_iMBC1Bank_1));
    stream.read(reinterpret_cast<char*> (&m_iMBC1MBank_0), sizeof(m_iMBC1MBank_0));
    stream.read(reinterpret_cast<char*> (&m_iMBC1MBank_1), sizeof(m_iMBC1MBank_1));
}

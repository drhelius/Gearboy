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
    m_pRAMBanks = new u8[0x2000];
    Reset(false);
}

MultiMBC1MemoryRule::~MultiMBC1MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void MultiMBC1MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iMulticartMode = 0;
    m_iROMBankHi = 0;
    m_iROMBankLo = 1;
    m_bRamEnabled = false;
    for (int i = 0; i < 0x2000; i++)
        m_pRAMBanks[i] = 0xFF;
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
                // Mode 0: rom0 is fixed at bank 0
                return pROM[address];
            }
            else
            {
                // Mode 1: rom0 is switchable
                int bank_addr = (m_iMBC1MBank_0 * 0x4000) + address;
                return pROM[bank_addr];
            }
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            int bank_addr = (m_iMBC1MBank_1 * 0x4000) + (address & 0x3FFF);
            return pROM[bank_addr];
        }
        default:
        {
            if ((address & 0xE000) == 0xA000)
            {
                if (m_bRamEnabled && m_pCartridge->GetRAMBankCount() > 0)
                    return m_pRAMBanks[address - 0xA000];
                else
                    return 0xFF;
            }
            return 0xFF;
        }
    }
}

void MultiMBC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (m_pCartridge->GetRAMBankCount() > 0)
            {
                bool previous = m_bRamEnabled;
                m_bRamEnabled = ((value & 0x0F) == 0x0A);

                if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
                {
                    (*m_pRamChangedCallback)();
                }
            }
            break;
        }
        case 0x2000:
        {
            m_iROMBankLo = value & 0x1F;
            SetROMBanks();
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            m_iROMBankHi = value & 0x03;
            SetROMBanks();
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            m_iMulticartMode = value & 0x01;
            SetROMBanks();
            break;
        }
        default:
        {
            if ((address & 0xE000) == 0xA000)
            {
                if (m_bRamEnabled && m_pCartridge->GetRAMBankCount() > 0)
                    m_pRAMBanks[address - 0xA000] = value;
            }
            else
            {
                Debug("--> ** Attempting to write on invalid address %X %X", address, value);
            }
            break;
        }
    }
}

void MultiMBC1MemoryRule::SetROMBanks()
{
    int mask = m_pCartridge->GetROMBankCount() - 1;

    m_iMBC1MBank_0 = (m_iROMBankHi << 4) & mask;

    int rom_bank = (m_iROMBankLo & 0x0F) | (m_iROMBankHi << 4);
    if ((m_iROMBankLo & 0x1F) == 0)
        rom_bank++;
    m_iMBC1MBank_1 = rom_bank & mask;
}

size_t MultiMBC1MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() > 0 ? 0x2000 : 0;
}

u8* MultiMBC1MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* MultiMBC1MemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks;
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
    if (m_iMulticartMode == 0)
        return 0;
    return m_iMBC1MBank_0;
}

u8* MultiMBC1MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_iMBC1MBank_1 * 0x4000];
}

int MultiMBC1MemoryRule::GetCurrentRomBank1Index()
{
    return m_iMBC1MBank_1;
}

void MultiMBC1MemoryRule::SaveRam(std::ostream &file)
{
    Debug("MultiMBC1MemoryRule save RAM...");

    if (m_pCartridge->GetRAMBankCount() > 0)
    {
        for (int i = 0; i < 0x2000; i++)
        {
            u8 ram_byte = m_pRAMBanks[i];
            file.write(reinterpret_cast<const char*> (&ram_byte), 1);
        }
    }

    Debug("MultiMBC1MemoryRule save RAM done");
}

bool MultiMBC1MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("MultiMBC1MemoryRule load RAM...");

    if (m_pCartridge->GetRAMBankCount() > 0)
    {
        if ((fileSize > 0) && (fileSize != 0x2000))
        {
            Log("MultiMBC1MemoryRule incorrect size. Expected: %d Found: %d", 0x2000, fileSize);
            return false;
        }

        for (int i = 0; i < 0x2000; i++)
        {
            u8 ram_byte = 0;
            file.read(reinterpret_cast<char*> (&ram_byte), 1);
            m_pRAMBanks[i] = ram_byte;
        }
    }

    Debug("MultiMBC1MemoryRule load RAM done");

    return true;
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
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), 0x2000);
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
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), 0x2000);
}

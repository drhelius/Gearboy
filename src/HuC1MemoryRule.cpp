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

#include "HuC1MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

const int kHuC1RamBanksSize = 0x8000;

HuC1MemoryRule::HuC1MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[kHuC1RamBanksSize];
    Reset(false);
}

HuC1MemoryRule::~HuC1MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void HuC1MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bIRMode = false;
    for (int i = 0; i < kHuC1RamBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
}

u8 HuC1MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            if (m_bIRMode)
            {
                // IR stub: always return no light detected
                return 0xC0;
            }
            return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void HuC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            // $0E switches to IR mode, anything else switches to RAM mode
            m_bIRMode = ((value & 0x0F) == 0x0E);
            break;
        }
        case 0x2000:
        {
            m_iCurrentROMBank = value & 0x3F;
            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            m_iCurrentRAMBank = value & 0x03;
            m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
            m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            // HuC1 ignores writes
            break;
        }
        case 0xA000:
        {
            if (m_bIRMode)
            {
                // IR stub: ignore writes
            }
            else
            {
                m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
            }
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void HuC1MemoryRule::SaveRam(std::ostream &file)
{
    Debug("HuC1MemoryRule save RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("HuC1MemoryRule save RAM done");
}

bool HuC1MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("HuC1MemoryRule load RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    if ((fileSize > 0) && (fileSize != ramSize))
    {
        Log("HuC1MemoryRule incorrect size. Expected: %d Found: %d", ramSize, fileSize);
        return false;
    }

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Debug("HuC1MemoryRule load RAM done");

    return true;
}

size_t HuC1MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

u8* HuC1MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* HuC1MemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int HuC1MemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank;
}

u8* HuC1MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

int HuC1MemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* HuC1MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int HuC1MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void HuC1MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_bIRMode), sizeof(m_bIRMode));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), kHuC1RamBanksSize);
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

void HuC1MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_bIRMode), sizeof(m_bIRMode));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), kHuC1RamBanksSize);
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

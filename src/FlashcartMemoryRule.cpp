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

#include "FlashcartMemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

FlashcartMemoryRule::FlashcartMemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_iRAMBanksSize = 0x20000;
    m_pRAMBanks = new u8[m_iRAMBanksSize];
    Reset(false);
}

FlashcartMemoryRule::~FlashcartMemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void FlashcartMemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROM0Bank = 0;
    m_iCurrentROMBank = 1;
    m_RomBankLow = 1;
    m_RomBankHigh = 0;
    m_RomBankMask = 0;
    m_RomBankLatch = 1;
    m_bConfigMode = false;
    m_bRamEnabled = true;
    m_iRAMBytesSize = GetRAMBytesSize();
    for (int i = 0; i < m_iRAMBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    m_CurrentROM0Address = 0;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
    if (m_bCGB && !m_pMemory->IsBootromEnabled())
        m_pMemory->Load(0xFF81, 0x00);
    UpdateBanks();
}

int FlashcartMemoryRule::GetRAMBytesSize() const
{
    if (m_pCartridge->GetRAMBankCount() <= 0)
        return 0;

    if (m_pCartridge->GetRAMSize() == 0x01)
        return 0x800;

    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

int FlashcartMemoryRule::NormalizeROMBank(int bank) const
{
    int bankCount = m_pCartridge->GetROMBankCount();

    if (bankCount <= 0)
        return 0;

    bank %= bankCount;

    if (bank < 0)
        bank += bankCount;

    return bank;
}

void FlashcartMemoryRule::UpdateBanks()
{
    int romBank = m_RomBankLow | (m_RomBankHigh << 8);

    m_iCurrentROM0Bank = NormalizeROMBank(m_RomBankMask);
    m_iCurrentROMBank = NormalizeROMBank(romBank | m_RomBankMask);
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;

    if (m_pCartridge->GetRAMBankCount() > 0)
    {
        m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
        m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
    }
    else
    {
        m_iCurrentRAMBank = 0;
        m_CurrentRAMAddress = 0;
    }
}

u8 FlashcartMemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[address + m_CurrentROM0Address];
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            if (m_bRamEnabled && m_iRAMBytesSize > 0)
                return m_pRAMBanks[((address - 0xA000) + m_CurrentRAMAddress) & (m_iRAMBytesSize - 1)];

            Debug("--> ** Attempting to read from disabled ram %X", address);
            return 0xFF;
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void FlashcartMemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (address < 0x2000)
            {
                if (value == 0xA5)
                    m_bConfigMode = true;
                else if (value == 0x98)
                    m_bConfigMode = false;
                else if (m_iRAMBytesSize > 0)
                    m_bRamEnabled = ((value & 0x0F) == 0x0A);
            }
            break;
        }
        case 0x2000:
        {
            if (address < 0x3000)
            {
                m_RomBankLow = value;
                m_RomBankLatch = value;
            }
            else
            {
                m_RomBankHigh = value & 0x01;
            }
            UpdateBanks();
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            if (m_pCartridge->GetRAMBankCount() > 0)
            {
                m_iCurrentRAMBank = value & 0x0F;
                UpdateBanks();
            }
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            if ((address & 0x1000) && m_bConfigMode)
            {
                m_RomBankMask = m_RomBankLatch;
                UpdateBanks();
            }
            break;
        }
        case 0xA000:
        {
            if (m_bRamEnabled && m_iRAMBytesSize > 0)
                m_pRAMBanks[((address - 0xA000) + m_CurrentRAMAddress) & (m_iRAMBytesSize - 1)] = value;
            else
                Debug("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void FlashcartMemoryRule::SaveRam(std::ostream &file)
{
    Debug("FlashcartMemoryRule save RAM...");

    for (int i = 0; i < m_iRAMBytesSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("FlashcartMemoryRule save RAM done");
}

bool FlashcartMemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("FlashcartMemoryRule load RAM...");

    if ((fileSize > 0) && (fileSize != m_iRAMBytesSize))
    {
        Log("FlashcartMemoryRule incorrect size. Expected: %d Found: %d", m_iRAMBytesSize, fileSize);
        return false;
    }

    for (int i = 0; i < m_iRAMBytesSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Debug("FlashcartMemoryRule load RAM done");

    return true;
}

size_t FlashcartMemoryRule::GetRamSize()
{
    return m_iRAMBytesSize;
}

u8* FlashcartMemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* FlashcartMemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int FlashcartMemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank;
}

u8* FlashcartMemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return pROM + m_CurrentROM0Address;
}

int FlashcartMemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* FlashcartMemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return pROM + m_CurrentROMAddress;
}

int FlashcartMemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void FlashcartMemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_RomBankLow), sizeof(m_RomBankLow));
    stream.write(reinterpret_cast<const char*> (&m_RomBankHigh), sizeof(m_RomBankHigh));
    stream.write(reinterpret_cast<const char*> (&m_RomBankMask), sizeof(m_RomBankMask));
    stream.write(reinterpret_cast<const char*> (&m_RomBankLatch), sizeof(m_RomBankLatch));
    stream.write(reinterpret_cast<const char*> (&m_bConfigMode), sizeof(m_bConfigMode));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (&m_iRAMBytesSize), sizeof(m_iRAMBytesSize));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), m_iRAMBanksSize);
    stream.write(reinterpret_cast<const char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

void FlashcartMemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_RomBankLow), sizeof(m_RomBankLow));
    stream.read(reinterpret_cast<char*> (&m_RomBankHigh), sizeof(m_RomBankHigh));
    stream.read(reinterpret_cast<char*> (&m_RomBankMask), sizeof(m_RomBankMask));
    stream.read(reinterpret_cast<char*> (&m_RomBankLatch), sizeof(m_RomBankLatch));
    stream.read(reinterpret_cast<char*> (&m_bConfigMode), sizeof(m_bConfigMode));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (&m_iRAMBytesSize), sizeof(m_iRAMBytesSize));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), m_iRAMBanksSize);
    stream.read(reinterpret_cast<char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
    UpdateBanks();
}
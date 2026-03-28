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

#include "MMM01MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

const int kMMM01RamBanksSize = 0x20000;

MMM01MemoryRule::MMM01MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[kMMM01RamBanksSize];
    Reset(false);
}

MMM01MemoryRule::~MMM01MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void MMM01MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_bLocked = false;
    m_bRamEnabled = false;
    m_bMBC1Mode = false;
    m_bMBC1ModeDisable = false;
    m_bMultiplexMode = false;
    m_RomBankLow = 0;
    m_RomBankMid = 0;
    m_RomBankHigh = 0;
    m_RomBankMask = 0;
    m_RamBankLow = 0;
    m_RamBankHigh = 0;
    m_RamBankMask = 0x03;
    m_iCurrentROMBank = 0;
    m_iCurrentROM0Bank = 0;
    m_iCurrentRAMBank = 0;
    for (int i = 0; i < kMMM01RamBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    m_CurrentROMAddress = 0;
    m_CurrentROM0Address = 0;
    m_CurrentRAMAddress = 0;
    UpdateBanks();
}

void MMM01MemoryRule::UpdateBanks()
{
    if (m_bLocked)
    {
        if (m_bMultiplexMode)
        {
            m_iCurrentROM0Bank = (m_RomBankLow & (m_RomBankMask << 1)) |
                                 ((m_bMBC1Mode ? 0 : m_RamBankLow) << 5) |
                                 (m_RomBankHigh << 7);
            m_iCurrentROMBank = m_RomBankLow |
                                (m_RamBankLow << 5) |
                                (m_RomBankHigh << 7);
            m_iCurrentRAMBank = m_RomBankMid | (m_RamBankHigh << 2);
        }
        else
        {
            m_iCurrentROM0Bank = (m_RomBankLow & (m_RomBankMask << 1)) |
                                 (m_RomBankMid << 5) |
                                 (m_RomBankHigh << 7);
            m_iCurrentROMBank = m_RomBankLow |
                                (m_RomBankMid << 5) |
                                (m_RomBankHigh << 7);
            m_iCurrentRAMBank = m_RamBankLow | (m_RamBankHigh << 2);
        }

        if (m_iCurrentROMBank == m_iCurrentROM0Bank)
            m_iCurrentROMBank++;

        int romBankCount = m_pCartridge->GetROMBankCount();
        if (romBankCount > 0)
        {
            m_iCurrentROMBank &= (romBankCount - 1);
            m_iCurrentROM0Bank &= (romBankCount - 1);
        }

        int ramBankCount = m_pCartridge->GetRAMBankCount();
        if (ramBankCount > 0)
            m_iCurrentRAMBank &= (ramBankCount - 1);
    }
    else
    {
        // Unmapped mode: map the last 32 KiB of the ROM
        int romBankCount = m_pCartridge->GetROMBankCount();
        if (romBankCount >= 2)
        {
            m_iCurrentROM0Bank = romBankCount - 2;
            m_iCurrentROMBank = romBankCount - 1;
        }
        else
        {
            m_iCurrentROM0Bank = 0;
            m_iCurrentROMBank = 0;
        }
        m_iCurrentRAMBank = 0;
    }

    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
    m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
}

u8 MMM01MemoryRule::PerformRead(u16 address)
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
            if (m_bRamEnabled)
            {
                return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
            }
            else
            {
                Debug("--> ** Attempting to read from disabled ram %X", address);
                return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void MMM01MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            // RAM Enable + RAM Bank Mask + Mapping Enable
            m_bRamEnabled = ((value & 0x0F) == 0x0A);

            if (!m_bLocked)
            {
                m_RamBankMask = (value >> 4) & 0x03;
                if (value & 0x40)
                {
                    m_bLocked = true;
                    UpdateBanks();
                }
            }
            break;
        }
        case 0x2000:
        {
            // ROM Bank Low + ROM Bank Mid (unmapped only)
            u8 masked_low = value & 0x1F;

            if (!m_bLocked)
            {
                m_RomBankMid = (value >> 5) & 0x03;
            }

            // Apply ROM Bank Mask: locked bits cannot be changed
            m_RomBankLow &= (m_RomBankMask << 1);
            m_RomBankLow |= masked_low & ~(m_RomBankMask << 1);

            UpdateBanks();
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            // RAM Bank Low + RAM Bank High + ROM Bank High + MBC1 Mode Disable
            m_RamBankLow = (value | (~m_RamBankMask)) & 0x03;

            if (!m_bLocked)
            {
                m_RamBankHigh = (value >> 2) & 0x03;
                m_RomBankHigh = (value >> 4) & 0x03;
                m_bMBC1ModeDisable = (value & 0x40) != 0;
            }

            UpdateBanks();
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            // MBC1 Mode Select + ROM Bank Mask + Multiplex Enable
            if (!m_bMBC1ModeDisable)
            {
                m_bMBC1Mode = (value & 0x01) != 0;
            }

            if (!m_bLocked)
            {
                m_RomBankMask = (value >> 2) & 0x0F;
                m_bMultiplexMode = (value & 0x40) != 0;
            }

            UpdateBanks();
            break;
        }
        case 0xA000:
        {
            if (m_bRamEnabled)
            {
                m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
            }
            else
            {
                Debug("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
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

void MMM01MemoryRule::SaveRam(std::ostream &file)
{
    Debug("MMM01MemoryRule save RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("MMM01MemoryRule save RAM done");
}

bool MMM01MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("MMM01MemoryRule load RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    if ((fileSize > 0) && (fileSize != ramSize))
    {
        Log("MMM01MemoryRule incorrect size. Expected: %d Found: %d", ramSize, fileSize);
        return false;
    }

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Debug("MMM01MemoryRule load RAM done");

    return true;
}

size_t MMM01MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

u8* MMM01MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* MMM01MemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int MMM01MemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank;
}

u8* MMM01MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return pROM + m_CurrentROM0Address;
}

int MMM01MemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* MMM01MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int MMM01MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void MMM01MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_bLocked), sizeof(m_bLocked));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (&m_bMBC1Mode), sizeof(m_bMBC1Mode));
    stream.write(reinterpret_cast<const char*> (&m_bMBC1ModeDisable), sizeof(m_bMBC1ModeDisable));
    stream.write(reinterpret_cast<const char*> (&m_bMultiplexMode), sizeof(m_bMultiplexMode));
    stream.write(reinterpret_cast<const char*> (&m_RomBankLow), sizeof(m_RomBankLow));
    stream.write(reinterpret_cast<const char*> (&m_RomBankMid), sizeof(m_RomBankMid));
    stream.write(reinterpret_cast<const char*> (&m_RomBankHigh), sizeof(m_RomBankHigh));
    stream.write(reinterpret_cast<const char*> (&m_RomBankMask), sizeof(m_RomBankMask));
    stream.write(reinterpret_cast<const char*> (&m_RamBankLow), sizeof(m_RamBankLow));
    stream.write(reinterpret_cast<const char*> (&m_RamBankHigh), sizeof(m_RamBankHigh));
    stream.write(reinterpret_cast<const char*> (&m_RamBankMask), sizeof(m_RamBankMask));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), kMMM01RamBanksSize);
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

void MMM01MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_bLocked), sizeof(m_bLocked));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (&m_bMBC1Mode), sizeof(m_bMBC1Mode));
    stream.read(reinterpret_cast<char*> (&m_bMBC1ModeDisable), sizeof(m_bMBC1ModeDisable));
    stream.read(reinterpret_cast<char*> (&m_bMultiplexMode), sizeof(m_bMultiplexMode));
    stream.read(reinterpret_cast<char*> (&m_RomBankLow), sizeof(m_RomBankLow));
    stream.read(reinterpret_cast<char*> (&m_RomBankMid), sizeof(m_RomBankMid));
    stream.read(reinterpret_cast<char*> (&m_RomBankHigh), sizeof(m_RomBankHigh));
    stream.read(reinterpret_cast<char*> (&m_RomBankMask), sizeof(m_RomBankMask));
    stream.read(reinterpret_cast<char*> (&m_RamBankLow), sizeof(m_RamBankLow));
    stream.read(reinterpret_cast<char*> (&m_RamBankHigh), sizeof(m_RamBankHigh));
    stream.read(reinterpret_cast<char*> (&m_RamBankMask), sizeof(m_RamBankMask));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), kMMM01RamBanksSize);
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
}

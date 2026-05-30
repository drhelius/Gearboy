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

#include "SachenMMC2MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

SachenMMC2MemoryRule::SachenMMC2MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_iRAMBanksSize = 0;
    m_iRAMBytesSize = 0;
    m_OuterBankOffset = 0;
    m_bAlternateWiring = false;
    m_pRAMBanks = NULL;
    Reset(false);
}

SachenMMC2MemoryRule::~SachenMMC2MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void SachenMMC2MemoryRule::Reset(bool bCGB)
{
    ResizeRAMBanks();

    m_bCGB = bCGB;
    m_LockMode = m_pMemory->IsBootromEnabled() ? LockModeDMG : LockModeUnlocked;
    m_iTransition = 0;
    m_Mask = 0;
    m_UnmaskedBank = 0;
    m_BaseBank = 0;
    m_OuterBankOffset = 0;
    m_bAlternateWiring = DetectAlternateWiring();
    u8* pROM = m_pCartridge->GetTheROM();
    m_bScrambledHeader = IsValidPointer(pROM) && (m_pCartridge->GetTotalSize() >= 0x1C5) &&
            (pROM[0x184] == 0xCE) && (pROM[0x194] == 0x66) &&
            (pROM[0x1C4] == 0xED);
    for (int i = 0; i < m_iRAMBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    SwitchROMBank0(0);
    SwitchROMBank1(1);
}

u16 SachenMMC2MemoryRule::UnscrambleAddress(u16 address) const
{
    u16 unscrambled = address & 0xFFAC;
    unscrambled |= (address & 0x0040) >> 6;
    unscrambled |= (address & 0x0010) >> 3;
    unscrambled |= (address & 0x0002) << 3;
    unscrambled |= (address & 0x0001) << 6;
    return unscrambled;
}

bool SachenMMC2MemoryRule::DetectAlternateWiring() const
{
    u8* pROM = m_pCartridge->GetTheROM();

    if (!IsValidPointer(pROM) || (m_pCartridge->GetTotalSize() < 0x40144))
        return false;

    static const u8 ok_topus_studios[15] = {
        'O', 'K', 'T', 'O', 'P', 'U', 'S', ' ', 'S', 'T', 'U', 'D', 'I', 'O', 'S'
    };

    return memcmp(pROM + 0x40134, ok_topus_studios, sizeof(ok_topus_studios)) == 0;
}

int SachenMMC2MemoryRule::NormalizeROMBank(int bank) const
{
    int bankCount = m_pCartridge->GetROMBankCount();

    if (bankCount <= 0)
        return 0;

    bank %= bankCount;

    if (bank < 0)
        bank += bankCount;

    return bank;
}

int SachenMMC2MemoryRule::GetRAMBytesSize() const
{
    if (m_pCartridge->GetRAMBankCount() <= 0)
        return 0;

    if (m_pCartridge->GetRAMSize() == 0x01)
        return 0x800;

    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

void SachenMMC2MemoryRule::ResizeRAMBanks()
{
    m_iRAMBytesSize = GetRAMBytesSize();

    int ramBanksSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    if (m_iRAMBanksSize != ramBanksSize)
    {
        SafeDeleteArray(m_pRAMBanks);
        m_iRAMBanksSize = ramBanksSize;

        if (m_iRAMBanksSize > 0)
            m_pRAMBanks = new u8[m_iRAMBanksSize];
    }
}

void SachenMMC2MemoryRule::SwitchROMBank0(int bank)
{
    m_iCurrentROM0Bank = NormalizeROMBank(m_OuterBankOffset + bank);
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
}

void SachenMMC2MemoryRule::SwitchROMBank1(int bank)
{
    m_iCurrentROMBank = NormalizeROMBank(m_OuterBankOffset + bank);
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

void SachenMMC2MemoryRule::UpdateLockOnRead(u16 address)
{
    if ((m_LockMode != LockModeUnlocked) && !m_pMemory->IsBootromRegistryEnabled())
    {
        m_LockMode = LockModeUnlocked;
        m_iTransition = 0;
        return;
    }

    if ((m_LockMode != LockModeUnlocked) && ((address & 0x8700) == 0x0100))
    {
        m_iTransition++;
        if (m_iTransition == 0x31)
        {
            m_LockMode = (m_LockMode == LockModeDMG) ? LockModeCGB : LockModeUnlocked;
            m_iTransition = 0;
        }
    }
}

void SachenMMC2MemoryRule::SwitchToCGBLock()
{
    if (m_LockMode == LockModeDMG)
    {
        m_LockMode = LockModeCGB;
        m_iTransition = 0;
    }
}

u8 SachenMMC2MemoryRule::PerformRead(u16 address)
{
    UpdateLockOnRead(address);

    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            if (((m_LockMode != LockModeUnlocked) || m_bScrambledHeader) &&
                    ((address & 0xFF00) == 0x0100))
            {
                if (m_LockMode == LockModeCGB)
                    address |= 0x0080;

                address = UnscrambleAddress(address);
            }

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
            if ((m_iRAMBytesSize > 0) && IsValidPointer(m_pRAMBanks))
                return m_pRAMBanks[(address - 0xA000) & (m_iRAMBytesSize - 1)];

            Debug("--> ** Attempting to read from RAM without ram in cart %X", address);
            return 0xFF;
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void SachenMMC2MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if ((m_UnmaskedBank & 0x30) == 0x30)
            {
                m_BaseBank = value;
                SwitchROMBank0(m_BaseBank & m_Mask);
                TraceBankSwitch(address, value);
            }
            break;
        }
        case 0x2000:
        {
            if (m_bAlternateWiring && ((address & 0x00C0) == 0x00C0))
            {
                m_OuterBankOffset = (value & 0x01) << 4;
                SwitchROMBank0(0);
                SwitchROMBank1(m_UnmaskedBank == 0 ? 1 : m_UnmaskedBank);
                TraceBankSwitch(address, value);
                break;
            }

            u8 bank = value;
            if (bank == 0)
                bank = 1;

            m_UnmaskedBank = bank;
            bank = (bank & ~m_Mask) | (m_BaseBank & m_Mask);
            SwitchROMBank1(bank);
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            if ((m_UnmaskedBank & 0x30) == 0x30)
            {
                m_Mask = value;
                u8 bank = (m_UnmaskedBank & ~m_Mask) | (m_BaseBank & m_Mask);
                SwitchROMBank1(bank);
                SwitchROMBank0(m_BaseBank & m_Mask);
                TraceBankSwitch(address, value);
            }
            break;
        }
        case 0x6000:
        {
            break;
        }
        case 0xA000:
        {
            if ((m_iRAMBytesSize > 0) && IsValidPointer(m_pRAMBanks))
                m_pRAMBanks[(address - 0xA000) & (m_iRAMBytesSize - 1)] = value;
            else
                Debug("--> ** Attempting to write to RAM without ram in cart  %X %X", address, value);
            break;
        }
        default:
        {
            m_pMemory->Load(address, value);
            break;
        }
    }
}

void SachenMMC2MemoryRule::SaveRam(std::ostream &file)
{
    Debug("SachenMMC2MemoryRule save RAM...");

    for (int i = 0; i < m_iRAMBanksSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    Debug("SachenMMC2MemoryRule save RAM done");
}

bool SachenMMC2MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("SachenMMC2MemoryRule load RAM...");

    if ((fileSize > 0) && (fileSize != m_iRAMBanksSize))
    {
        Log("SachenMMC2MemoryRule incorrect size. Expected: %d Found: %d", m_iRAMBanksSize, fileSize);
        return false;
    }

    for (int i = 0; i < m_iRAMBanksSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    Debug("SachenMMC2MemoryRule load RAM done");

    return true;
}

size_t SachenMMC2MemoryRule::GetRamSize()
{
    return m_iRAMBanksSize;
}

u8* SachenMMC2MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* SachenMMC2MemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks;
}

int SachenMMC2MemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

bool SachenMMC2MemoryRule::NeedsHighMemoryAccessNotifications()
{
    return true;
}

void SachenMMC2MemoryRule::NotifyHighMemoryRead(u16 address)
{
    if (address >= 0xC000)
        SwitchToCGBLock();
}

void SachenMMC2MemoryRule::NotifyHighMemoryWrite(u16 address, u8 value)
{
    UNUSED(value);

    if ((address & 0xE000) == 0xC000)
        SwitchToCGBLock();
}

u8* SachenMMC2MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROM0Address];
}

int SachenMMC2MemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* SachenMMC2MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int SachenMMC2MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void SachenMMC2MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_LockMode), sizeof(m_LockMode));
    stream.write(reinterpret_cast<const char*> (&m_iTransition), sizeof(m_iTransition));
    stream.write(reinterpret_cast<const char*> (&m_Mask), sizeof(m_Mask));
    stream.write(reinterpret_cast<const char*> (&m_UnmaskedBank), sizeof(m_UnmaskedBank));
    stream.write(reinterpret_cast<const char*> (&m_BaseBank), sizeof(m_BaseBank));
    stream.write(reinterpret_cast<const char*> (&m_bScrambledHeader), sizeof(m_bScrambledHeader));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
}

void SachenMMC2MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_LockMode), sizeof(m_LockMode));
    stream.read(reinterpret_cast<char*> (&m_iTransition), sizeof(m_iTransition));
    stream.read(reinterpret_cast<char*> (&m_Mask), sizeof(m_Mask));
    stream.read(reinterpret_cast<char*> (&m_UnmaskedBank), sizeof(m_UnmaskedBank));
    stream.read(reinterpret_cast<char*> (&m_BaseBank), sizeof(m_BaseBank));
    stream.read(reinterpret_cast<char*> (&m_bScrambledHeader), sizeof(m_bScrambledHeader));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    m_bAlternateWiring = DetectAlternateWiring();
    m_OuterBankOffset = m_bAlternateWiring ? (m_iCurrentROM0Bank & 0x10) : 0;
    ResizeRAMBanks();
    m_iCurrentROM0Bank = NormalizeROMBank(m_iCurrentROM0Bank);
    m_iCurrentROMBank = NormalizeROMBank(m_iCurrentROMBank);
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}
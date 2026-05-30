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

#include "SachenMMC1MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

static void RollLogoRegister(u8& c, u8& a, bool& carry)
{
    u8 old_c = c;
    c = static_cast<u8>((c << 1) | (carry ? 1 : 0));
    carry = (old_c & 0x80) != 0;

    u8 old_a = a;
    a = static_cast<u8>((a << 1) | (carry ? 1 : 0));
    carry = (old_a & 0x80) != 0;
}

static u8 ExpandLogoPass(u8& c, u8 a, bool& carry)
{
    for (int i = 0; i < 4; i++)
    {
        u8 saved_c = c;
        RollLogoRegister(c, a, carry);
        c = saved_c;
        RollLogoRegister(c, a, carry);
    }

    return a;
}

SachenMMC1MemoryRule::SachenMMC1MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

SachenMMC1MemoryRule::~SachenMMC1MemoryRule()
{
}

void SachenMMC1MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_LockMode = m_pMemory->IsBootromEnabled() ? LockModeDMG : LockModeUnlocked;
    m_iTransition = 0;
    m_Mask = 0;
    m_UnmaskedBank = 0;
    m_BaseBank = 0;
    SwitchROMBank0(0);
    SwitchROMBank1(1);

    if (!m_pMemory->IsBootromEnabled())
        LoadBootromLogoState();
}

u16 SachenMMC1MemoryRule::UnscrambleAddress(u16 address) const
{
    u16 unscrambled = address & 0xFFAC;
    unscrambled |= (address & 0x0040) >> 6;
    unscrambled |= (address & 0x0010) >> 3;
    unscrambled |= (address & 0x0002) << 3;
    unscrambled |= (address & 0x0001) << 6;
    return unscrambled;
}

u8 SachenMMC1MemoryRule::ReadBootromLogoByte(u16 address) const
{
    u8* pROM = m_pCartridge->GetTheROM();

    if (!IsValidPointer(pROM))
        return 0xFF;

    int physical_address = UnscrambleAddress(address | 0x0080) + m_CurrentROM0Address;

    if (physical_address >= m_pCartridge->GetTotalSize())
        return 0xFF;

    return pROM[physical_address];
}

void SachenMMC1MemoryRule::LoadBootromLogoState()
{
    if (m_pCartridge->GetTotalSize() < 0x150)
        return;

    bool carry = false;
    u16 destination = 0x8010;

    for (u16 address = 0x0104; address <= 0x0133; address++)
    {
        u8 a = ReadBootromLogoByte(address);
        u8 c = a;

        a = ExpandLogoPass(c, a, carry);
        m_pMemory->Load(destination, a);
        destination += 2;
        m_pMemory->Load(destination, a);
        destination += 2;

        a = ExpandLogoPass(c, a, carry);
        m_pMemory->Load(destination, a);
        destination += 2;
        m_pMemory->Load(destination, a);
        destination += 2;

        carry = ((address + 1) & 0xFF) < 0x34;
    }
}

int SachenMMC1MemoryRule::NormalizeROMBank(int bank) const
{
    int bankCount = m_pCartridge->GetROMBankCount();

    if (bankCount <= 0)
        return 0;

    bank %= bankCount;

    if (bank < 0)
        bank += bankCount;

    return bank;
}

void SachenMMC1MemoryRule::SwitchROMBank0(int bank)
{
    m_iCurrentROM0Bank = NormalizeROMBank(bank);
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
}

void SachenMMC1MemoryRule::SwitchROMBank1(int bank)
{
    m_iCurrentROMBank = NormalizeROMBank(bank);
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

u8 SachenMMC1MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            if (m_LockMode != LockModeUnlocked)
            {
                m_iTransition++;
                if (m_iTransition == 0x31)
                    m_LockMode = LockModeUnlocked;
                else
                    address |= 0x0080;
            }

            if ((address & 0xFF00) == 0x0100)
                address = UnscrambleAddress(address);

            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[address + m_CurrentROM0Address];
        }
        case 0x4000:
        case 0x6000:
        {
            if (m_LockMode != LockModeUnlocked)
            {
                m_iTransition++;
                if (m_iTransition == 0x31)
                    m_LockMode = LockModeUnlocked;
                else
                    address |= 0x0080;
            }

            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            Debug("--> ** Attempting to read from RAM without ram in cart %X", address);
            return 0xFF;
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void SachenMMC1MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch ((address & 0xFFD3) & 0xE000)
    {
        case 0x0000:
        {
            if ((m_UnmaskedBank & 0x30) == 0x30)
            {
                m_BaseBank = value;
                u8 bank = (m_UnmaskedBank & ~m_Mask) | (m_BaseBank & m_Mask);
                SwitchROMBank0(m_BaseBank & m_Mask);
                SwitchROMBank1(bank);
                TraceBankSwitch(address, value);
            }
            break;
        }
        case 0x2000:
        {
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
        case 0xA000:
        {
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

u8* SachenMMC1MemoryRule::GetRomBank0()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROM0Address];
}

int SachenMMC1MemoryRule::GetCurrentRomBank0Index()
{
    return m_iCurrentROM0Bank;
}

u8* SachenMMC1MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int SachenMMC1MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

void SachenMMC1MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_LockMode), sizeof(m_LockMode));
    stream.write(reinterpret_cast<const char*> (&m_iTransition), sizeof(m_iTransition));
    stream.write(reinterpret_cast<const char*> (&m_Mask), sizeof(m_Mask));
    stream.write(reinterpret_cast<const char*> (&m_UnmaskedBank), sizeof(m_UnmaskedBank));
    stream.write(reinterpret_cast<const char*> (&m_BaseBank), sizeof(m_BaseBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
}

void SachenMMC1MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_LockMode), sizeof(m_LockMode));
    stream.read(reinterpret_cast<char*> (&m_iTransition), sizeof(m_iTransition));
    stream.read(reinterpret_cast<char*> (&m_Mask), sizeof(m_Mask));
    stream.read(reinterpret_cast<char*> (&m_UnmaskedBank), sizeof(m_UnmaskedBank));
    stream.read(reinterpret_cast<char*> (&m_BaseBank), sizeof(m_BaseBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    SwitchROMBank0(m_iCurrentROM0Bank);
    SwitchROMBank1(m_iCurrentROMBank);
}
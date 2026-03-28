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

#include "HuC3MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

const int kHuC3RamBanksSize = 0x8000;

HuC3MemoryRule::HuC3MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[kHuC3RamBanksSize];
    Reset(false);
}

HuC3MemoryRule::~HuC3MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void HuC3MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
    m_iMode = 0;
    for (int i = 0; i < kHuC3RamBanksSize; i++)
        m_pRAMBanks[i] = 0xFF;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
    m_RTC.minutes = 0;
    m_RTC.days = 0;
    m_RTC.alarm_minutes = 0;
    m_RTC.alarm_days = 0;
    m_RTC.alarm_enabled = 0;
    m_RTC.last_time = static_cast<s32>(m_pCartridge->GetCurrentRTC());
    memset(m_RTC.padding, 0, sizeof(m_RTC.padding));
    m_RTCAccessIndex = 0;
    m_RTCAccessFlags = 0;
    m_RTCReadValue = 0;
}

void HuC3MemoryRule::UpdateRTC()
{
    s32 now = static_cast<s32>(m_pCartridge->GetCurrentRTC());

    if (now > m_RTC.last_time)
    {
        s32 diff = now - m_RTC.last_time;
        m_RTC.last_time = now;

        s32 minutes_elapsed = diff / 60;
        m_RTC.minutes += static_cast<u16>(minutes_elapsed);

        while (m_RTC.minutes >= 1440)
        {
            m_RTC.minutes -= 1440;
            m_RTC.days++;
        }
    }
}

bool HuC3MemoryRule::HandleRTCWrite(u8 value)
{
    switch (m_iMode)
    {
        case 0x0B:
        {
            // RTC command/argument write
            u8 command = (value >> 4) & 0x0F;
            u8 argument = value & 0x0F;

            switch (command)
            {
                case 1:
                {
                    // Read value and increment access address
                    UpdateRTC();
                    if (m_RTCAccessIndex < 3)
                    {
                        m_RTCReadValue = (m_RTC.minutes >> (m_RTCAccessIndex * 4)) & 0x0F;
                    }
                    else if (m_RTCAccessIndex < 7)
                    {
                        m_RTCReadValue = (m_RTC.days >> ((m_RTCAccessIndex - 3) * 4)) & 0x0F;
                    }
                    else
                    {
                        m_RTCReadValue = 0;
                    }
                    m_RTCAccessIndex++;
                    break;
                }
                case 2:
                case 3:
                {
                    // Write value and (optionally) increment access address
                    if (m_RTCAccessIndex < 3)
                    {
                        m_RTC.minutes &= ~(0x0F << (m_RTCAccessIndex * 4));
                        m_RTC.minutes |= (argument << (m_RTCAccessIndex * 4));
                    }
                    else if (m_RTCAccessIndex < 7)
                    {
                        m_RTC.days &= ~(0x0F << ((m_RTCAccessIndex - 3) * 4));
                        m_RTC.days |= (argument << ((m_RTCAccessIndex - 3) * 4));
                    }
                    else if (m_RTCAccessIndex >= 0x58 && m_RTCAccessIndex <= 0x5A)
                    {
                        m_RTC.alarm_minutes &= ~(0x0F << ((m_RTCAccessIndex - 0x58) * 4));
                        m_RTC.alarm_minutes |= (argument << ((m_RTCAccessIndex - 0x58) * 4));
                    }
                    else if (m_RTCAccessIndex >= 0x5B && m_RTCAccessIndex <= 0x5E)
                    {
                        m_RTC.alarm_days &= ~(0x0F << ((m_RTCAccessIndex - 0x5B) * 4));
                        m_RTC.alarm_days |= (argument << ((m_RTCAccessIndex - 0x5B) * 4));
                    }
                    else if (m_RTCAccessIndex == 0x5F)
                    {
                        m_RTC.alarm_enabled = argument & 1;
                    }
                    if (command == 3)
                        m_RTCAccessIndex++;
                    break;
                }
                case 4:
                {
                    // Set access address low nibble
                    m_RTCAccessIndex &= 0xF0;
                    m_RTCAccessIndex |= argument;
                    break;
                }
                case 5:
                {
                    // Set access address high nibble
                    m_RTCAccessIndex &= 0x0F;
                    m_RTCAccessIndex |= (argument << 4);
                    break;
                }
                case 6:
                {
                    // Extended command
                    m_RTCAccessFlags = argument;
                    break;
                }
                default:
                    break;
            }
            return true;
        }
        case 0x0C:
        {
            // RTC read register: read only
            return true;
        }
        case 0x0D:
        {
            // RTC semaphore: ignore writes
            return true;
        }
        case 0x0E:
        {
            // IR mode stub: ignore writes
            return true;
        }
        default:
            return false;
    }
}

u8 HuC3MemoryRule::PerformRead(u16 address)
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
            switch (m_iMode)
            {
                case 0x0C:
                    // RTC command/response read
                    if (m_RTCAccessFlags == 0x02)
                        return 0x01;
                    return m_RTCReadValue;
                case 0x0D:
                    // RTC semaphore: always ready
                    return 0x01;
                case 0x0E:
                    // IR stub: no light detected
                    return 0xC0;
                case 0x00:
                case 0x0A:
                {
                    if (m_pCartridge->GetRAMBankCount() > 0)
                        return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
                    return 0xFF;
                }
                default:
                    return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void HuC3MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            // Mode select
            bool previous = m_bRamEnabled;
            m_iMode = value & 0x0F;
            m_bRamEnabled = (m_iMode == 0x0A);

            if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            {
                (*m_pRamChangedCallback)();
            }
            break;
        }
        case 0x2000:
        {
            m_iCurrentROMBank = value & 0x7F;
            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            m_iCurrentRAMBank = value & 0x0F;
            m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
            m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
            TraceBankSwitch(address, value);
            break;
        }
        case 0x6000:
        {
            // Games write $01 here on startup
            break;
        }
        case 0xA000:
        {
            if (HandleRTCWrite(value))
                break;

            if (m_bRamEnabled && m_pCartridge->GetRAMBankCount() > 0)
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

void HuC3MemoryRule::SaveRam(std::ostream &file)
{
    Debug("HuC3MemoryRule save RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    file.write(reinterpret_cast<const char*> (&m_RTC), sizeof(m_RTC));

    Debug("HuC3MemoryRule save RAM done");
}

bool HuC3MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("HuC3MemoryRule load RAM...");

    s32 ramSize = m_pCartridge->GetRAMBankCount() * 0x2000;
    bool loadRTC = true;

    if (fileSize > 0)
    {
        if (fileSize < ramSize)
        {
            Log("HuC3MemoryRule incorrect size. Expected: %d Found: %d", ramSize, fileSize);
            return false;
        }

        s32 expectedWithRTC = ramSize + static_cast<s32>(sizeof(m_RTC));
        if (fileSize < expectedWithRTC)
        {
            Log("HuC3MemoryRule no RTC data in save file");
            loadRTC = false;
        }
    }

    for (s32 i = 0; i < ramSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    if (loadRTC)
    {
        file.read(reinterpret_cast<char*> (&m_RTC), sizeof(m_RTC));
    }

    Debug("HuC3MemoryRule load RAM done");

    return true;
}

size_t HuC3MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
}

size_t HuC3MemoryRule::GetRTCSize()
{
    return sizeof(m_RTC);
}

u8* HuC3MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* HuC3MemoryRule::GetCurrentRamBank()
{
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int HuC3MemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank;
}

u8* HuC3MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

int HuC3MemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* HuC3MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int HuC3MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

u8* HuC3MemoryRule::GetRTCMemory()
{
    return reinterpret_cast<u8*>(&m_RTC);
}

void HuC3MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (&m_iMode), sizeof(m_iMode));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), kHuC3RamBanksSize);
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
    stream.write(reinterpret_cast<const char*> (&m_RTC), sizeof(m_RTC));
    stream.write(reinterpret_cast<const char*> (&m_RTCAccessIndex), sizeof(m_RTCAccessIndex));
    stream.write(reinterpret_cast<const char*> (&m_RTCAccessFlags), sizeof(m_RTCAccessFlags));
    stream.write(reinterpret_cast<const char*> (&m_RTCReadValue), sizeof(m_RTCReadValue));
}

void HuC3MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (&m_iMode), sizeof(m_iMode));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), kHuC3RamBanksSize);
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
    stream.read(reinterpret_cast<char*> (&m_RTC), sizeof(m_RTC));
    stream.read(reinterpret_cast<char*> (&m_RTCAccessIndex), sizeof(m_RTCAccessIndex));
    stream.read(reinterpret_cast<char*> (&m_RTCAccessFlags), sizeof(m_RTCAccessFlags));
    stream.read(reinterpret_cast<char*> (&m_RTCReadValue), sizeof(m_RTCReadValue));
}

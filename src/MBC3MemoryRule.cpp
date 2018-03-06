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

#include "MBC3MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

MBC3MemoryRule::MBC3MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    m_pRAMBanks = new u8[0x8000];
    Reset(false);
}

MBC3MemoryRule::~MBC3MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void MBC3MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
    m_bRTCEnabled = false;
    for (int i = 0; i < 0x8000; i++)
        m_pRAMBanks[i] = 0xFF;
    m_RTC.Seconds = 0;
    m_RTC.Minutes = 0;
    m_RTC.Hours = 0;
    m_RTC.Days = 0;
    m_RTC.Control = 0;
    m_RTC.LatchedSeconds = 0;
    m_RTC.LatchedMinutes = 0;
    m_RTC.LatchedHours = 0;
    m_RTC.LatchedDays = 0;
    m_RTC.LatchedControl = 0;
    m_RTC.LastTime = static_cast<s32>(m_pCartridge->GetCurrentRTC());
    m_RTC.padding = 0;
    m_iRTCLatch = 0;
    m_RTCRegister = 0;
    m_RTCLastTimeCache = m_RTC.LastTime;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
}

u8 MBC3MemoryRule::PerformRead(u16 address)
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
            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
                }
                else
                {
                    Log("--> ** Attempting to read from disabled ram %X", address);
                    return 0xFF;
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                switch (m_RTCRegister)
                {
                    case 0x08:
                        return m_RTC.LatchedSeconds;
                        break;
                    case 0x09:
                        return m_RTC.LatchedMinutes;
                        break;
                    case 0x0A:
                        return m_RTC.LatchedHours;
                        break;
                    case 0x0B:
                        return m_RTC.LatchedDays;
                        break;
                    case 0x0C:
                        return m_RTC.LatchedControl;
                        break;
                    default:
                        return 0xFF;
                }
            }
            else
            {
                Log("--> ** Attempting to read from disabled RTC %X", address);
                return 0xFF;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void MBC3MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        {
            if (m_pCartridge->GetRAMSize() > 0)
            {
                bool previous = m_bRamEnabled;
                m_bRamEnabled = ((value & 0x0F) == 0x0A);

                if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
                {
                    (*m_pRamChangedCallback)();
                }
            }
            m_bRTCEnabled = ((value & 0x0F) == 0x0A);
            break;
        }
        case 0x2000:
        {
            m_iCurrentROMBank = value & 0x7F;
            if (m_iCurrentROMBank == 0)
                m_iCurrentROMBank = 1;
            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            break;
        }
        case 0x4000:
        {
            if ((value >= 0x08) && (value <= 0x0C))
            {
                // RTC
                if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
                {
                    m_RTCRegister = value;
                    m_iCurrentRAMBank = -1;
                }
                else
                {
                    Log("--> ** Attempting to select RTC register when RTC is disabled or not present %X %X", address, value);
                }
            }
            else if (value <= 0x03)
            {
                m_iCurrentRAMBank = value;
                m_iCurrentRAMBank &= (m_pCartridge->GetRAMBankCount() - 1);
                m_CurrentRAMAddress = m_iCurrentRAMBank * 0x2000;
            }
            else
            {
                Log("--> ** Attempting to select unkwon register %X %X", address, value);
            }
            break;
        }
        case 0x6000:
        {
            if (m_pCartridge->IsRTCPresent())
            {
                // RTC Latch
                if ((m_iRTCLatch == 0x00) && (value == 0x01))
                {
                    UpdateRTC();
                    m_RTC.LatchedSeconds = m_RTC.Seconds;
                    m_RTC.LatchedMinutes = m_RTC.Minutes;
                    m_RTC.LatchedHours = m_RTC.Hours;
                    m_RTC.LatchedDays = m_RTC.Days;
                    m_RTC.LatchedControl = m_RTC.Control;
                }

                m_iRTCLatch = value;
            }
            break;
        }
        case 0xA000:
        {
            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
                }
                else
                {
                    Log("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                switch (m_RTCRegister)
                {
                    case 0x08:
                        m_RTC.Seconds = value;
                        break;
                    case 0x09:
                        m_RTC.Minutes = value;
                        break;
                    case 0x0A:
                        m_RTC.Hours = value;
                        break;
                    case 0x0B:
                        m_RTC.Days = value;
                        break;
                    case 0x0C:
                        m_RTC.Control = (m_RTC.Control & 0x80) | (value & 0xC1);
                        break;
                }
            }
            else
            {
                Log("--> ** Attempting to write on RTC when RTC is disabled or not present %X %X", address, value);
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

void MBC3MemoryRule::UpdateRTC()
{
    s32 now = static_cast<s32>(m_pCartridge->GetCurrentRTC());

    if (!IsSetBit(m_RTC.Control, 6) && (m_RTCLastTimeCache != now))
    {
        m_RTCLastTimeCache = now;
        s32 difference = now - m_RTC.LastTime;
        m_RTC.LastTime = now;

        if (difference > 0)
        {
            m_RTC.Seconds += (s32) (difference % 60);

            if (m_RTC.Seconds > 59)
            {
                m_RTC.Seconds -= 60;
                m_RTC.Minutes++;
            }

            difference /= 60;
            m_RTC.Minutes += (s32) (difference % 60);

            if (m_RTC.Minutes > 59)
            {
                m_RTC.Minutes -= 60;
                m_RTC.Hours++;
            }

            difference /= 60;
            m_RTC.Hours += (s32) (difference % 24);

            if (m_RTC.Hours > 23)
            {
                m_RTC.Hours -= 24;
                m_RTC.Days++;
            }

            difference /= 24;
            m_RTC.Days += (s32) (difference & 0xffffffff);

            if (m_RTC.Days > 0xFF)
            {
                m_RTC.Control = (m_RTC.Control & 0xC1) | 0x01;

                if (m_RTC.Days > 511)
                {
                    m_RTC.Days %= 512;
                    m_RTC.Control |= 0x80;
                    m_RTC.Control &= 0xC0;
                }
            }
        }
    }
}

void MBC3MemoryRule::SaveRam(std::ostream & file)
{
    Log("MBC3MemoryRule save RAM...");

    for (int i = 0; i < 0x8000; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    if (m_pCartridge->IsRTCPresent())
    {
        file.write(reinterpret_cast<const char*> (&m_RTC), sizeof(m_RTC));
    }

    Log("MBC3MemoryRule save RAM done");
}

bool MBC3MemoryRule::LoadRam(std::istream & file, s32 fileSize)
{
    Log("MBC3MemoryRule load RAM...");

    bool loadRTC = m_pCartridge->IsRTCPresent();

    if (fileSize > 0)
    {
        if (fileSize < 0x8000)
        {
            Log("MBC3MemoryRule incorrect RAM size. Expected: %d Found: %d", 0x8000, fileSize);
            return false;
        }

        if (loadRTC)
        {
            s32 minExpectedSize = 0x8000 + 44;
            s32 maxExpectedSize = 0x8000 + 48;

            if ((fileSize != minExpectedSize) && (fileSize != maxExpectedSize))
            {
                Log("MBC3MemoryRule incorrect RTC size. MinExpected: %d MaxExpected: %d Found: %d", minExpectedSize, maxExpectedSize, fileSize);
            }

            if (fileSize < minExpectedSize)
            {
                Log("MBC3MemoryRule ignoring RTC data");
                loadRTC = false;
            }
        }
    }

    for (int i = 0; i < 0x8000; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    if (loadRTC)
    {
        file.read(reinterpret_cast<char*> (&m_RTC), 44);
    }

    Log("MBC3MemoryRule load RAM done");

    return true;
}

size_t MBC3MemoryRule::GetRamSize()
{
    return 0x8000;
}

size_t MBC3MemoryRule::GetRTCSize()
{
    return m_pCartridge->IsRTCPresent() ? sizeof(m_RTC) : 0;
}

u8* MBC3MemoryRule::GetRamBanks()
{
    return m_pRAMBanks;
}

u8* MBC3MemoryRule::GetCurrentRamBank()
{
    if (m_iCurrentRAMBank >= 0)
        return &m_pRAMBanks[m_CurrentRAMAddress];
    else
        return NULL;
}

u8* MBC3MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

u8* MBC3MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

u8* MBC3MemoryRule::GetRTCMemory()
{
    return m_pCartridge->IsRTCPresent() ? reinterpret_cast<u8*>(&m_RTC) : NULL;
}

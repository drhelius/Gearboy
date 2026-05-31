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
    m_iRAMBanksSize = 0;
    m_pRAMBanks = NULL;
    Reset(false);
}

MBC3MemoryRule::~MBC3MemoryRule()
{
    SafeDeleteArray(m_pRAMBanks);
}

void MBC3MemoryRule::Reset(bool bCGB)
{
    ResizeRAMBanks();

    m_bCGB = bCGB;
    m_iCurrentRAMBank = 0;
    m_iCurrentROMBank = 1;
    m_bRamEnabled = false;
    m_bRTCEnabled = false;
    for (int i = 0; i < m_iRAMBanksSize; i++)
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
    m_iCurrentROM0Bank = 0;
    m_CurrentROM0Address = 0;
    m_CurrentROMAddress = 0x4000;
    m_CurrentRAMAddress = 0;
    m_iRTCCycles = 0;
    m_bPKJDRAMSelected = true;
    for (int i = 0; i < 7; i++)
        m_PKJDRegisters[i] = 0;
    m_iPoke2in1BaseBank = 0;
    m_bPoke2in1Bank0Change = false;
    m_bPoke2in1Locked = false;
}

void MBC3MemoryRule::ResizeRAMBanks()
{
    int ramBanksSize = m_pCartridge->IsMBC30() ? 0x10000 : 0x8000;

    if (m_iRAMBanksSize != ramBanksSize)
    {
        SafeDeleteArray(m_pRAMBanks);
        m_iRAMBanksSize = ramBanksSize;
        m_pRAMBanks = new u8[m_iRAMBanksSize];
    }
}

int MBC3MemoryRule::GetSafeRAMBankMask() const
{
    int ramBankCount = m_pCartridge->GetRAMBankCount();

    if (ramBankCount <= 0)
        ramBankCount = 4;

    return ramBankCount - 1;
}

int MBC3MemoryRule::NormalizeROMBank(int bank) const
{
    int bankCount = m_pCartridge->GetROMBankCount();

    if (bankCount <= 0)
        return 0;

    bank %= bankCount;

    if (bank < 0)
        bank += bankCount;

    return bank;
}

void MBC3MemoryRule::SetPoke2in1ROMBank(u8 value)
{
    int bank = value & 0x7F;

    if (bank == 0)
        bank = 1;

    m_iCurrentROMBank = NormalizeROMBank(bank + m_iPoke2in1BaseBank);
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

void MBC3MemoryRule::SetPoke2in1BaseBank(int bank)
{
    m_iPoke2in1BaseBank = NormalizeROMBank(bank);
    m_iCurrentROM0Bank = m_iPoke2in1BaseBank;
    m_iCurrentROMBank = NormalizeROMBank(m_iPoke2in1BaseBank + 1);
    m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
    m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
}

u8 MBC3MemoryRule::ReadPKJD(u16 address)
{
    if (!m_bRamEnabled)
    {
        Debug("--> ** Attempting to read from disabled ram %X", address);
        return 0xFF;
    }

    if (m_bPKJDRAMSelected)
        return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];

    if (m_RTCRegister < 7)
        return m_PKJDRegisters[m_RTCRegister];

    return 0;
}

void MBC3MemoryRule::WritePKJD(u16 address, u8 value)
{
    if (!m_bRamEnabled)
    {
        Debug("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
        return;
    }

    if (m_bPKJDRAMSelected)
    {
        m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
        return;
    }

    switch (m_RTCRegister)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            m_PKJDRegisters[m_RTCRegister] = value;
            break;
        case 7:
            switch (value)
            {
                case 0x11:
                    m_PKJDRegisters[5]--;
                    break;
                case 0x12:
                    m_PKJDRegisters[6]--;
                    break;
                case 0x41:
                    m_PKJDRegisters[5] += m_PKJDRegisters[6];
                    break;
                case 0x42:
                    m_PKJDRegisters[6] += m_PKJDRegisters[5];
                    break;
                case 0x51:
                    m_PKJDRegisters[5]++;
                    break;
                case 0x52:
                    m_PKJDRegisters[6]--;
                    break;
            }
            break;
    }
}

u8 MBC3MemoryRule::ReadPoke2in1RAM(u16 address)
{
    return m_pRAMBanks[((address - 0xA000) + m_CurrentRAMAddress) & (m_iRAMBanksSize - 1)];
}

void MBC3MemoryRule::WritePoke2in1RAM(u16 address, u8 value)
{
    if (m_bPoke2in1Bank0Change && (address == 0xA100) && !m_bPoke2in1Locked)
    {
        if (value == 0x01)
            SetPoke2in1BaseBank(2);
        else if (value != 0xC0)
            SetPoke2in1BaseBank(66);
        else
        {
            m_bPoke2in1Locked = true;
            SetPoke2in1BaseBank(m_iPoke2in1BaseBank);
        }
        return;
    }

    m_pRAMBanks[((address - 0xA000) + m_CurrentRAMAddress) & (m_iRAMBanksSize - 1)] = value;
}

u8 MBC3MemoryRule::PerformRead(u16 address)
{
    switch (address & 0xE000)
    {
        case 0x0000:
        case 0x2000:
        {
            if (IsPoke2in1())
            {
                u8* pROM = m_pCartridge->GetTheROM();
                return pROM[address + m_CurrentROM0Address];
            }

            return m_pMemory->Retrieve(address);
        }
        case 0x4000:
        case 0x6000:
        {
            u8* pROM = m_pCartridge->GetTheROM();
            return pROM[(address - 0x4000) + m_CurrentROMAddress];
        }
        case 0xA000:
        {
            if (IsPoke2in1())
                return ReadPoke2in1RAM(address);

            if (IsPKJD())
                return ReadPKJD(address);

            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    if (!m_pCartridge->IsMBC30() && m_pCartridge->IsRTCPresent() && (m_iCurrentRAMBank & 0x07) > 3)
                        return 0xFF;
                    return m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress];
                }
                else
                {
                    Debug("--> ** Attempting to read from disabled ram %X", address);
                    return 0xFF;
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                switch (m_RTCRegister & 0x07)
                {
                    case 0x00:
                        return m_RTC.LatchedSeconds & 0x3F;
                        break;
                    case 0x01:
                        return m_RTC.LatchedMinutes & 0x3F;
                        break;
                    case 0x02:
                        return m_RTC.LatchedHours & 0x1F;
                        break;
                    case 0x03:
                        return m_RTC.LatchedDays;
                        break;
                    case 0x04:
                        return m_RTC.LatchedControl & 0xC1;
                        break;
                    default:
                        return 0xFF;
                }
            }
            else
            {
                Debug("--> ** Attempting to read from disabled RTC %X", address);
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
            if (IsPoke2in1())
            {
                bool previous = m_bRamEnabled;
                m_bRamEnabled = ((value & 0x0A) == 0x0A);
                m_bPoke2in1Bank0Change = ((value & 0xC0) == 0xC0);

                if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
                {
                    (*m_pRamChangedCallback)();
                }
                m_bRTCEnabled = false;
                break;
            }

            bool previous = m_bRamEnabled;
            m_bRamEnabled = ((value & 0x0F) == 0x0A);

            if (IsValidPointer(m_pRamChangedCallback) && previous && !m_bRamEnabled)
            {
                (*m_pRamChangedCallback)();
            }
            m_bRTCEnabled = ((value & 0x0F) == 0x0A);
            break;
        }
        case 0x2000:
        {
            if (IsPoke2in1())
                SetPoke2in1ROMBank(value);
            else if (m_pCartridge->IsMBC30())
                m_iCurrentROMBank = value;
            else
                m_iCurrentROMBank = value & 0x7F;
            if (!IsPoke2in1())
            {
                if (m_iCurrentROMBank == 0)
                    m_iCurrentROMBank = 1;
                m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
                m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
            }
            TraceBankSwitch(address, value);
            break;
        }
        case 0x4000:
        {
            if (IsPKJD())
            {
                u8 bank = value & 0x0F;

                if (bank < 8)
                {
                    m_iCurrentRAMBank = value;
                    m_CurrentRAMAddress = (m_iCurrentRAMBank & GetSafeRAMBankMask()) * 0x2000;
                    TraceBankSwitch(address, value);

                    if (value < 8)
                    {
                        m_bPKJDRAMSelected = true;
                        m_RTCRegister = 0;
                    }
                }
                else if (bank <= 0x0C)
                {
                    m_bPKJDRAMSelected = false;
                    m_RTCRegister = bank - 8;
                    m_iCurrentRAMBank = -1;
                }
                else if (value <= 0x0F)
                {
                    m_bPKJDRAMSelected = false;
                    m_RTCRegister = value - 8;
                    m_iCurrentRAMBank = -1;
                }
                break;
            }

            if (m_pCartridge->IsRTCPresent() && (value & 0x08))
            {
                // RTC register select (bit 3 set)
                m_RTCRegister = value;
                m_iCurrentRAMBank = -1;
            }
            else
            {
                m_iCurrentRAMBank = value;
                m_CurrentRAMAddress = (m_iCurrentRAMBank & (m_pCartridge->GetRAMBankCount() - 1)) * 0x2000;
                TraceBankSwitch(address, value);
            }
            break;
        }
        case 0x6000:
        {
            if (m_pCartridge->IsRTCPresent())
            {
                // RTC Latch
                m_RTC.LatchedSeconds = m_RTC.Seconds;
                m_RTC.LatchedMinutes = m_RTC.Minutes;
                m_RTC.LatchedHours = m_RTC.Hours;
                m_RTC.LatchedDays = m_RTC.Days & 0xFF;
                m_RTC.LatchedControl = (m_RTC.Control & 0xC0) | ((m_RTC.Days >> 8) & 0x01);
            }
            break;
        }
        case 0xA000:
        {
            if (IsPoke2in1())
            {
                WritePoke2in1RAM(address, value);
                break;
            }

            if (IsPKJD())
            {
                WritePKJD(address, value);
                break;
            }

            if (m_iCurrentRAMBank >= 0)
            {
                if (m_bRamEnabled)
                {
                    if (!m_pCartridge->IsMBC30() && m_pCartridge->IsRTCPresent() && (m_iCurrentRAMBank & 0x07) > 3)
                        break;
                    m_pRAMBanks[(address - 0xA000) + m_CurrentRAMAddress] = value;
                }
                else
                {
                    Debug("--> ** Attempting to write on RAM when ram is disabled %X %X", address, value);
                }
            }
            else if (m_pCartridge->IsRTCPresent() && m_bRTCEnabled)
            {
                switch (m_RTCRegister & 0x07)
                {
                    case 0x00:
                        m_RTC.Seconds = value;
                        m_iRTCCycles = 0;
                        break;
                    case 0x01:
                        m_RTC.Minutes = value;
                        break;
                    case 0x02:
                        m_RTC.Hours = value;
                        break;
                    case 0x03:
                        m_RTC.Days = (m_RTC.Days & 0x100) | value;
                        break;
                    case 0x04:
                        m_RTC.Days = (m_RTC.Days & 0xFF) | ((value & 0x01) << 8);
                        m_RTC.Control = value & 0xC1;
                        break;
                }
            }
            else
            {
                Debug("--> ** Attempting to write on RTC when RTC is disabled or not present %X %X", address, value);
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

void MBC3MemoryRule::Tick(unsigned int clockCycles)
{
    if (!m_pCartridge->IsRTCPresent())
        return;
    if (IsSetBit(m_RTC.Control, 6))
        return;

    m_iRTCCycles += clockCycles;

    while (m_iRTCCycles >= GEARBOY_MASTER_CLOCK_RATE)
    {
        m_iRTCCycles -= GEARBOY_MASTER_CLOCK_RATE;

        if (++m_RTC.Seconds == 60)
        {
            m_RTC.Seconds = 0;
            if (++m_RTC.Minutes == 60)
            {
                m_RTC.Minutes = 0;
                if (++m_RTC.Hours == 24)
                {
                    m_RTC.Hours = 0;
                    if (++m_RTC.Days > 0x1FF)
                    {
                        m_RTC.Days = 0;
                        m_RTC.Control |= 0x80;
                    }
                }
            }
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

            if (m_RTC.Days > 0x1FF)
            {
                m_RTC.Days &= 0x1FF;
                m_RTC.Control |= 0x80;
            }
        }
    }
}

void MBC3MemoryRule::SaveRam(std::ostream & file)
{
    Debug("MBC3MemoryRule save RAM...");

    for (int i = 0; i < m_iRAMBanksSize; i++)
    {
        u8 ram_byte = m_pRAMBanks[i];
        file.write(reinterpret_cast<const char*> (&ram_byte), 1);
    }

    if (m_pCartridge->IsRTCPresent())
    {
        m_pCartridge->UpdateCurrentRTC();
        RTC_Registers rtcOut = m_RTC;
        rtcOut.LastTime = static_cast<s32>(m_pCartridge->GetCurrentRTC());
        rtcOut.Days = m_RTC.Days & 0xFF;
        rtcOut.Control = (m_RTC.Control & 0xC0) | ((m_RTC.Days >> 8) & 0x01);
        file.write(reinterpret_cast<const char*> (&rtcOut), sizeof(rtcOut));
    }

    Debug("MBC3MemoryRule save RAM done");
}

bool MBC3MemoryRule::LoadRam(std::istream & file, s32 fileSize)
{
    Debug("MBC3MemoryRule load RAM...");

    bool loadRTC = m_pCartridge->IsRTCPresent();

    if (fileSize > 0)
    {
        if (fileSize < m_iRAMBanksSize)
        {
            Log("MBC3MemoryRule incorrect RAM size. Expected: %d Found: %d", m_iRAMBanksSize, fileSize);
            return false;
        }

        if (loadRTC)
        {
            s32 minExpectedSize = m_iRAMBanksSize + 44;
            s32 maxExpectedSize = m_iRAMBanksSize + 48;

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

    for (int i = 0; i < m_iRAMBanksSize; i++)
    {
        u8 ram_byte = 0;
        file.read(reinterpret_cast<char*> (&ram_byte), 1);
        m_pRAMBanks[i] = ram_byte;
    }

    if (loadRTC)
    {
        file.read(reinterpret_cast<char*> (&m_RTC), 44);
        m_RTC.Days = (m_RTC.Days & 0xFF) | ((m_RTC.Control & 0x01) << 8);
        m_pCartridge->UpdateCurrentRTC();
        m_RTCLastTimeCache = 0;
        UpdateRTC();
    }

    Debug("MBC3MemoryRule load RAM done");

    return true;
}

size_t MBC3MemoryRule::GetRamSize()
{
    return m_pCartridge->GetRAMBankCount() * 0x2000;
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
    return m_pRAMBanks + m_CurrentRAMAddress;
}

int MBC3MemoryRule::GetCurrentRamBankIndex()
{
    return m_iCurrentRAMBank > 0 ? m_iCurrentRAMBank : 0;
}

u8* MBC3MemoryRule::GetRomBank0()
{
    if (IsPoke2in1())
    {
        u8* pROM = m_pCartridge->GetTheROM();
        return &pROM[m_CurrentROM0Address];
    }

    return m_pMemory->GetMemoryMap() + 0x0000;
}

int MBC3MemoryRule::GetCurrentRomBank0Index()
{
    if (IsPoke2in1())
        return m_iCurrentROM0Bank;

    return 0;
}

u8* MBC3MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int MBC3MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

u8* MBC3MemoryRule::GetRTCMemory()
{
    return m_pCartridge->IsRTCPresent() ? reinterpret_cast<u8*>(&m_RTC) : NULL;
}

void MBC3MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.write(reinterpret_cast<const char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.write(reinterpret_cast<const char*> (&m_bRTCEnabled), sizeof(m_bRTCEnabled));
    stream.write(reinterpret_cast<const char*> (m_pRAMBanks), m_iRAMBanksSize);
    stream.write(reinterpret_cast<const char*> (&m_iRTCLatch), sizeof(m_iRTCLatch));
    stream.write(reinterpret_cast<const char*> (&m_RTCRegister), sizeof(m_RTCRegister));
    stream.write(reinterpret_cast<const char*> (&m_RTCLastTimeCache), sizeof(m_RTCLastTimeCache));
    stream.write(reinterpret_cast<const char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
    stream.write(reinterpret_cast<const char*> (&m_RTC), sizeof(m_RTC));

    if (IsPKJD())
    {
        stream.write(reinterpret_cast<const char*> (&m_bPKJDRAMSelected), sizeof(m_bPKJDRAMSelected));
        stream.write(reinterpret_cast<const char*> (m_PKJDRegisters), sizeof(m_PKJDRegisters));
    }

    if (IsPoke2in1())
    {
        stream.write(reinterpret_cast<const char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
        stream.write(reinterpret_cast<const char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
        stream.write(reinterpret_cast<const char*> (&m_iPoke2in1BaseBank), sizeof(m_iPoke2in1BaseBank));
        stream.write(reinterpret_cast<const char*> (&m_bPoke2in1Bank0Change), sizeof(m_bPoke2in1Bank0Change));
        stream.write(reinterpret_cast<const char*> (&m_bPoke2in1Locked), sizeof(m_bPoke2in1Locked));
    }
}

void MBC3MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*> (&m_iCurrentRAMBank), sizeof(m_iCurrentRAMBank));
    stream.read(reinterpret_cast<char*> (&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*> (&m_bRamEnabled), sizeof(m_bRamEnabled));
    stream.read(reinterpret_cast<char*> (&m_bRTCEnabled), sizeof(m_bRTCEnabled));
    stream.read(reinterpret_cast<char*> (m_pRAMBanks), m_iRAMBanksSize);
    stream.read(reinterpret_cast<char*> (&m_iRTCLatch), sizeof(m_iRTCLatch));
    stream.read(reinterpret_cast<char*> (&m_RTCRegister), sizeof(m_RTCRegister));
    stream.read(reinterpret_cast<char*> (&m_RTCLastTimeCache), sizeof(m_RTCLastTimeCache));
    stream.read(reinterpret_cast<char*> (&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*> (&m_CurrentRAMAddress), sizeof(m_CurrentRAMAddress));
    stream.read(reinterpret_cast<char*> (&m_RTC), sizeof(m_RTC));
    m_RTC.Days = (m_RTC.Days & 0xFF) | ((m_RTC.Control & 0x01) << 8);

    if (IsPKJD())
    {
        stream.read(reinterpret_cast<char*> (&m_bPKJDRAMSelected), sizeof(m_bPKJDRAMSelected));
        stream.read(reinterpret_cast<char*> (m_PKJDRegisters), sizeof(m_PKJDRegisters));
    }

    if (IsPoke2in1())
    {
        stream.read(reinterpret_cast<char*> (&m_iCurrentROM0Bank), sizeof(m_iCurrentROM0Bank));
        stream.read(reinterpret_cast<char*> (&m_CurrentROM0Address), sizeof(m_CurrentROM0Address));
        stream.read(reinterpret_cast<char*> (&m_iPoke2in1BaseBank), sizeof(m_iPoke2in1BaseBank));
        stream.read(reinterpret_cast<char*> (&m_bPoke2in1Bank0Change), sizeof(m_bPoke2in1Bank0Change));
        stream.read(reinterpret_cast<char*> (&m_bPoke2in1Locked), sizeof(m_bPoke2in1Locked));
        m_iCurrentROM0Bank = NormalizeROMBank(m_iCurrentROM0Bank);
        m_CurrentROM0Address = m_iCurrentROM0Bank * 0x4000;
        m_iPoke2in1BaseBank = NormalizeROMBank(m_iPoke2in1BaseBank);
    }
}

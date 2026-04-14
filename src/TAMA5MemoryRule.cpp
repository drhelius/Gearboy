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

#include "TAMA5MemoryRule.h"
#include "Video.h"
#include "Memory.h"
#include "Processor.h"
#include "Input.h"
#include "Cartridge.h"

enum TAMA5_Register
{
    TAMA5_BANK_LO  = 0x0,
    TAMA5_BANK_HI  = 0x1,
    TAMA5_WRITE_LO = 0x4,
    TAMA5_WRITE_HI = 0x5,
    TAMA5_ADDR_HI  = 0x6,
    TAMA5_ADDR_LO  = 0x7,
};

enum TAMA5_ReadRegister
{
    TAMA5_ACTIVE  = 0xA,
    TAMA5_READ_LO = 0xC,
    TAMA5_READ_HI = 0xD,
};

enum TAMA6_RTCIndex
{
    TAMA6_RTC_SECOND_1  = 0x0,
    TAMA6_RTC_SECOND_10 = 0x1,
    TAMA6_RTC_MINUTE_1  = 0x2,
    TAMA6_RTC_MINUTE_10 = 0x3,
    TAMA6_RTC_HOUR_1    = 0x4,
    TAMA6_RTC_HOUR_10   = 0x5,
    TAMA6_RTC_WEEK      = 0x6,
    TAMA6_RTC_DAY_1     = 0x7,
    TAMA6_RTC_DAY_10    = 0x8,
    TAMA6_RTC_MONTH_1   = 0x9,
    TAMA6_RTC_MONTH_10  = 0xA,
    TAMA6_RTC_YEAR_1    = 0xB,
    TAMA6_RTC_YEAR_10   = 0xC,
    TAMA6_RTC_PAGE      = 0xD,
};

enum TAMA6_AlarmIndex
{
    TAMA6_ALARM_24_HOUR   = 0xA,
    TAMA6_ALARM_LEAP_YEAR = 0xB,
};

enum TAMA6_Command
{
    TAMA6_DISABLE_TIMER = 0x00,
    TAMA6_ENABLE_TIMER  = 0x01,
    TAMA6_MINUTE_WRITE  = 0x04,
    TAMA6_HOUR_WRITE    = 0x05,
    TAMA6_MINUTE_READ   = 0x06,
    TAMA6_HOUR_READ     = 0x07,
    TAMA6_DISABLE_ALARM = 0x10,
    TAMA6_ENABLE_ALARM  = 0x11,
};

static const u8 k_TAMA6RTCMask[32] = {
    // Timer page (0x00-0x0F)
    0xF, 0x7, 0xF, 0x7, 0xF, 0x3, 0x7, 0xF, 0x3, 0xF, 0x1, 0xF, 0xF, 0x0, 0x0, 0x0,
    // Alarm page (0x10-0x1F)
    0x0, 0x0, 0xF, 0x7, 0xF, 0x3, 0x7, 0xF, 0x3, 0x0, 0x1, 0x3, 0x0, 0x0, 0x0, 0x0,
};

static const int k_DaysToMonth[] = {
    0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

static int DMYToDayOfYear(int day, int month, int leapYear)
{
    if (month < 1 || month > 12)
        return 1;

    int d = day + k_DaysToMonth[month];

    if (month > 2 && (leapYear & 3) == 0)
        d++;

    return d;
}

static int DayOfYearToMonth(int dayOfYear, int leapYear)
{
    for (int m = 1; m < 12; m++)
    {
        int limit = k_DaysToMonth[m + 1];

        if (m >= 2 && (leapYear & 3) == 0)
            limit++;

        if (dayOfYear <= limit)
            return m;
    }

    return 12;
}

static int DayOfYearToDayOfMonth(int dayOfYear, int leapYear)
{
    int month = DayOfYearToMonth(dayOfYear, leapYear);
    int offset = k_DaysToMonth[month];

    if (month > 2 && (leapYear & 3) == 0)
        offset++;

    return dayOfYear - offset;
}

TAMA5MemoryRule::TAMA5MemoryRule(Processor* pProcessor,
        Memory* pMemory, Video* pVideo, Input* pInput,
        Cartridge* pCartridge, Audio* pAudio) : MemoryRule(pProcessor,
pMemory, pVideo, pInput, pCartridge, pAudio)
{
    Reset(false);
}

TAMA5MemoryRule::~TAMA5MemoryRule()
{
}

void TAMA5MemoryRule::Reset(bool bCGB)
{
    m_bCGB = bCGB;
    m_iCurrentROMBank = 1;
    m_CurrentROMAddress = 0x4000;
    m_Reg = 0;
    memset(m_Registers, 0, sizeof(m_Registers));
    memset(m_SRAM, 0xFF, sizeof(m_SRAM));
    memset(&m_RTC, 0, sizeof(m_RTC));
    m_RTC.lastTime = static_cast<s32>(m_pCartridge->GetCurrentRTC());
}

void TAMA5MemoryRule::UpdateRTC()
{
    s32 now = static_cast<s32>(m_pCartridge->GetCurrentRTC());
    s32 elapsed = now - m_RTC.lastTime;
    m_RTC.lastTime = now;

    if (elapsed <= 0 || m_RTC.disabled)
        return;

    u8* t = m_RTC.rtcTimerPage;
    bool is24hour = m_RTC.rtcAlarmPage[TAMA6_ALARM_24_HOUR] != 0;

    // Seconds
    s32 diff = t[TAMA6_RTC_SECOND_1] + t[TAMA6_RTC_SECOND_10] * 10 + (elapsed % 60);

    if (diff < 0)
    {
        diff += 60;
        elapsed -= 60;
    }

    t[TAMA6_RTC_SECOND_1] = diff % 10;
    t[TAMA6_RTC_SECOND_10] = (diff % 60) / 10;
    elapsed = elapsed / 60 + diff / 60;

    // Minutes
    diff = t[TAMA6_RTC_MINUTE_1] + t[TAMA6_RTC_MINUTE_10] * 10 + (elapsed % 60);

    if (diff < 0)
    {
        diff += 60;
        elapsed -= 60;
    }

    t[TAMA6_RTC_MINUTE_1] = diff % 10;
    t[TAMA6_RTC_MINUTE_10] = (diff % 60) / 10;
    elapsed = elapsed / 60 + diff / 60;

    // Hours
    int hour = t[TAMA6_RTC_HOUR_1];

    if (is24hour)
    {
        hour += t[TAMA6_RTC_HOUR_10] * 10;
    }
    else
    {
        int h10 = t[TAMA6_RTC_HOUR_10];
        hour += (h10 & 1) * 10;
        hour += ((h10 & 2) >> 1) * 12;
    }

    diff = hour + (elapsed % 24);

    if (diff < 0)
    {
        diff += 24;
        elapsed -= 24;
    }

    if (is24hour)
    {
        t[TAMA6_RTC_HOUR_1] = (diff % 24) % 10;
        t[TAMA6_RTC_HOUR_10] = (diff % 24) / 10;
    }
    else
    {
        t[TAMA6_RTC_HOUR_1] = (diff % 12) % 10;
        t[TAMA6_RTC_HOUR_10] = (u8)((diff % 12) / 10 + (diff / 12) * 2);
    }

    elapsed = elapsed / 24 + diff / 24;

    // Days
    int day = t[TAMA6_RTC_DAY_1] + t[TAMA6_RTC_DAY_10] * 10;
    int month = t[TAMA6_RTC_MONTH_1] + t[TAMA6_RTC_MONTH_10] * 10;
    int year = t[TAMA6_RTC_YEAR_1] + t[TAMA6_RTC_YEAR_10] * 10;
    int leapYear = m_RTC.rtcAlarmPage[TAMA6_ALARM_LEAP_YEAR];
    int week = t[TAMA6_RTC_WEEK];

    int dayOfYear = DMYToDayOfYear(day, month, leapYear);
    diff = dayOfYear + elapsed;

    while (diff <= 0)
    {
        diff += ((leapYear & 3) ? 365 : 366);
        year--;
        leapYear--;
    }
    while (diff > ((leapYear & 3) ? 365 : 366))
    {
        diff -= ((leapYear & 3) ? 365 : 366);
        year++;
        leapYear++;
    }

    week = (int)((week + elapsed) % 7);
    if (week < 0)
        week += 7;

    year %= 100;
    leapYear &= 3;
    day = DayOfYearToDayOfMonth(diff, leapYear);
    month = DayOfYearToMonth(diff, leapYear);

    t[TAMA6_RTC_WEEK] = (u8)week;
    t[TAMA6_RTC_DAY_1] = day % 10;
    t[TAMA6_RTC_DAY_10] = day / 10;
    t[TAMA6_RTC_MONTH_1] = month % 10;
    t[TAMA6_RTC_MONTH_10] = month / 10;
    t[TAMA6_RTC_YEAR_1] = year % 10;
    t[TAMA6_RTC_YEAR_10] = year / 10;
    m_RTC.rtcAlarmPage[TAMA6_ALARM_LEAP_YEAR] = (u8)leapYear;
}

void TAMA5MemoryRule::ExecuteCommand()
{
    u8 address = ((m_Registers[TAMA5_ADDR_HI] << 4) & 0x10) | m_Registers[TAMA5_ADDR_LO];
    u8 data = (m_Registers[TAMA5_WRITE_HI] << 4) | m_Registers[TAMA5_WRITE_LO];

    switch (m_Registers[TAMA5_ADDR_HI] >> 1)
    {
        case 0x0:
        {
            // RAM write
            if (address < 32)
                m_SRAM[address] = data;
            break;
        }
        case 0x1:
        {
            // RAM read (handled in PerformRead)
            break;
        }
        case 0x2:
        {
            // RTC commands
            switch (address)
            {
                case TAMA6_DISABLE_TIMER:
                    m_RTC.disabled = 1;
                    m_RTC.rtcTimerPage[TAMA6_RTC_PAGE] &= 0x7;
                    m_RTC.rtcAlarmPage[TAMA6_RTC_PAGE] &= 0x7;
                    m_RTC.rtcFreePage0[TAMA6_RTC_PAGE] &= 0x7;
                    m_RTC.rtcFreePage1[TAMA6_RTC_PAGE] &= 0x7;
                    break;
                case TAMA6_ENABLE_TIMER:
                    m_RTC.disabled = 0;
                    m_RTC.rtcTimerPage[TAMA6_RTC_SECOND_1] = 0;
                    m_RTC.rtcTimerPage[TAMA6_RTC_SECOND_10] = 0;
                    m_RTC.rtcTimerPage[TAMA6_RTC_PAGE] |= 0x8;
                    m_RTC.rtcAlarmPage[TAMA6_RTC_PAGE] |= 0x8;
                    m_RTC.rtcFreePage0[TAMA6_RTC_PAGE] |= 0x8;
                    m_RTC.rtcFreePage1[TAMA6_RTC_PAGE] |= 0x8;
                    break;
                case TAMA6_MINUTE_WRITE:
                    m_RTC.rtcTimerPage[TAMA6_RTC_MINUTE_1] = data & 0x0F;
                    m_RTC.rtcTimerPage[TAMA6_RTC_MINUTE_10] = data >> 4;
                    break;
                case TAMA6_HOUR_WRITE:
                    m_RTC.rtcTimerPage[TAMA6_RTC_HOUR_1] = data & 0x0F;
                    m_RTC.rtcTimerPage[TAMA6_RTC_HOUR_10] = data >> 4;
                    break;
                case TAMA6_DISABLE_ALARM:
                    m_RTC.rtcTimerPage[TAMA6_RTC_PAGE] &= 0xB;
                    m_RTC.rtcAlarmPage[TAMA6_RTC_PAGE] &= 0xB;
                    m_RTC.rtcFreePage0[TAMA6_RTC_PAGE] &= 0xB;
                    m_RTC.rtcFreePage1[TAMA6_RTC_PAGE] &= 0xB;
                    break;
                case TAMA6_ENABLE_ALARM:
                    m_RTC.rtcTimerPage[TAMA6_RTC_PAGE] |= 0x4;
                    m_RTC.rtcAlarmPage[TAMA6_RTC_PAGE] |= 0x4;
                    m_RTC.rtcFreePage0[TAMA6_RTC_PAGE] |= 0x4;
                    m_RTC.rtcFreePage1[TAMA6_RTC_PAGE] |= 0x4;
                    break;
                default:
                    break;
            }
            break;
        }
        case 0x4:
        {
            // RTC page direct access
            u8 rtcAddr = m_Registers[TAMA5_WRITE_LO];
            if (rtcAddr >= TAMA6_RTC_PAGE)
                break;
            u8 rtcData = m_Registers[TAMA5_WRITE_HI];
            switch (m_Registers[TAMA5_ADDR_LO])
            {
                case 0:
                    rtcData &= k_TAMA6RTCMask[rtcAddr];
                    m_RTC.rtcTimerPage[rtcAddr] = rtcData;
                    break;
                case 2:
                    rtcData &= k_TAMA6RTCMask[rtcAddr | 0x10];
                    m_RTC.rtcAlarmPage[rtcAddr] = rtcData;
                    break;
                case 4:
                    m_RTC.rtcFreePage0[rtcAddr] = rtcData;
                    break;
                case 6:
                    m_RTC.rtcFreePage1[rtcAddr] = rtcData;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

u8 TAMA5MemoryRule::PerformRead(u16 address)
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
            if ((address & 0x1FFF) > 1)
                return 0xFF;

            if (address & 1)
                return 0xFF;

            u8 regAddr = ((m_Registers[TAMA5_ADDR_HI] << 4) & 0x10) | m_Registers[TAMA5_ADDR_LO];
            u8 value = 0xF0;

            switch (m_Reg)
            {
                case TAMA5_ACTIVE:
                    return 0xF1;

                case TAMA5_READ_LO:
                case TAMA5_READ_HI:
                {
                    switch (m_Registers[TAMA5_ADDR_HI] >> 1)
                    {
                        case 0x1:
                        {
                            // RAM read
                            if (regAddr < 32)
                                value = m_SRAM[regAddr];
                            break;
                        }
                        case 0x2:
                        {
                            // RTC commands read
                            UpdateRTC();
                            switch (regAddr)
                            {
                                case TAMA6_MINUTE_READ:
                                    value = (m_RTC.rtcTimerPage[TAMA6_RTC_MINUTE_10] << 4) |
                                             m_RTC.rtcTimerPage[TAMA6_RTC_MINUTE_1];
                                    break;
                                case TAMA6_HOUR_READ:
                                    value = (m_RTC.rtcTimerPage[TAMA6_RTC_HOUR_10] << 4) |
                                             m_RTC.rtcTimerPage[TAMA6_RTC_HOUR_1];
                                    break;
                                default:
                                    value = regAddr;
                                    break;
                            }
                            break;
                        }
                        case 0x4:
                        {
                            // RTC page direct read
                            if (m_Reg == TAMA5_READ_HI)
                                break;
                            UpdateRTC();
                            u8 rtcAddr = m_Registers[TAMA5_WRITE_LO];
                            if (rtcAddr > TAMA6_RTC_PAGE)
                            {
                                value = 0;
                                break;
                            }
                            switch (m_Registers[TAMA5_ADDR_LO])
                            {
                                case 1:
                                    value = m_RTC.rtcTimerPage[rtcAddr];
                                    break;
                                case 3: 
                                    value = m_RTC.rtcAlarmPage[rtcAddr];
                                    break;
                                case 5: 
                                    value = m_RTC.rtcFreePage0[rtcAddr];
                                    break;
                                case 7: 
                                    value = m_RTC.rtcFreePage1[rtcAddr];
                                    break;
                                default: break;
                            }
                            break;
                        }
                        default:
                            break;
                    }

                    if (m_Reg == TAMA5_READ_HI)
                        value >>= 4;

                    return value | 0xF0;
                }

                default:
                    return 0xF1;
            }
        }
        default:
        {
            return m_pMemory->Retrieve(address);
        }
    }
}

void TAMA5MemoryRule::PerformWrite(u16 address, u8 value)
{
    switch (address & 0xE000)
    {
        case 0xA000:
        {
            if ((address & 0x1FFF) > 1)
                break;

            if (address & 1)
            {
                // Register select
                m_Reg = value;
            }
            else
            {
                // Data write (4-bit)
                value &= 0x0F;

                if (m_Reg < 8)
                {
                    m_Registers[m_Reg] = value;

                    switch (m_Reg)
                    {
                        case TAMA5_BANK_LO:
                        case TAMA5_BANK_HI:
                        {
                            m_iCurrentROMBank = m_Registers[TAMA5_BANK_LO] | (m_Registers[TAMA5_BANK_HI] << 4);
                            m_iCurrentROMBank &= (m_pCartridge->GetROMBankCount() - 1);
                            m_CurrentROMAddress = m_iCurrentROMBank * 0x4000;
                            TraceBankSwitch(address, value);
                            break;
                        }
                        case TAMA5_ADDR_LO:
                        {
                            ExecuteCommand();
                            break;
                        }
                        default:
                            break;
                    }
                }
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

void TAMA5MemoryRule::SaveRam(std::ostream &file)
{
    Debug("TAMA5MemoryRule save RAM...");

    file.write(reinterpret_cast<const char*>(m_SRAM), sizeof(m_SRAM));
    file.write(reinterpret_cast<const char*>(&m_RTC), sizeof(m_RTC));

    Debug("TAMA5MemoryRule save RAM done");
}

bool TAMA5MemoryRule::LoadRam(std::istream &file, s32 fileSize)
{
    Debug("TAMA5MemoryRule load RAM...");

    s32 sramSize = static_cast<s32>(sizeof(m_SRAM));
    s32 rtcSize = static_cast<s32>(sizeof(m_RTC));
    bool loadRTC = true;

    if (fileSize > 0)
    {
        if (fileSize < sramSize)
        {
            Log("TAMA5MemoryRule incorrect size. Expected: %d Found: %d", sramSize, fileSize);
            return false;
        }
        if (fileSize < sramSize + rtcSize)
        {
            Log("TAMA5MemoryRule no RTC data in save file");
            loadRTC = false;
        }
    }

    file.read(reinterpret_cast<char*>(m_SRAM), sizeof(m_SRAM));

    if (loadRTC)
    {
        file.read(reinterpret_cast<char*>(&m_RTC), sizeof(m_RTC));
    }

    Debug("TAMA5MemoryRule load RAM done");

    return true;
}

size_t TAMA5MemoryRule::GetRamSize()
{
    return sizeof(m_SRAM);
}

size_t TAMA5MemoryRule::GetRTCSize()
{
    return sizeof(m_RTC);
}

u8* TAMA5MemoryRule::GetRamBanks()
{
    return m_SRAM;
}

u8* TAMA5MemoryRule::GetCurrentRamBank()
{
    return m_SRAM;
}

int TAMA5MemoryRule::GetCurrentRamBankIndex()
{
    return 0;
}

u8* TAMA5MemoryRule::GetRomBank0()
{
    return m_pMemory->GetMemoryMap() + 0x0000;
}

int TAMA5MemoryRule::GetCurrentRomBank0Index()
{
    return 0;
}

u8* TAMA5MemoryRule::GetCurrentRomBank1()
{
    u8* pROM = m_pCartridge->GetTheROM();
    return &pROM[m_CurrentROMAddress];
}

int TAMA5MemoryRule::GetCurrentRomBank1Index()
{
    return m_iCurrentROMBank;
}

u8* TAMA5MemoryRule::GetRTCMemory()
{
    return reinterpret_cast<u8*>(&m_RTC);
}

void TAMA5MemoryRule::SaveState(std::ostream& stream)
{
    using namespace std;

    stream.write(reinterpret_cast<const char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.write(reinterpret_cast<const char*>(&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.write(reinterpret_cast<const char*>(&m_Reg), sizeof(m_Reg));
    stream.write(reinterpret_cast<const char*>(m_Registers), sizeof(m_Registers));
    stream.write(reinterpret_cast<const char*>(m_SRAM), sizeof(m_SRAM));
    stream.write(reinterpret_cast<const char*>(&m_RTC), sizeof(m_RTC));
}

void TAMA5MemoryRule::LoadState(std::istream& stream)
{
    using namespace std;

    stream.read(reinterpret_cast<char*>(&m_iCurrentROMBank), sizeof(m_iCurrentROMBank));
    stream.read(reinterpret_cast<char*>(&m_CurrentROMAddress), sizeof(m_CurrentROMAddress));
    stream.read(reinterpret_cast<char*>(&m_Reg), sizeof(m_Reg));
    stream.read(reinterpret_cast<char*>(m_Registers), sizeof(m_Registers));
    stream.read(reinterpret_cast<char*>(m_SRAM), sizeof(m_SRAM));
    stream.read(reinterpret_cast<char*>(&m_RTC), sizeof(m_RTC));
}

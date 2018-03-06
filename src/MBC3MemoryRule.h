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

#ifndef MBC3MEMORYRULE_H
#define	MBC3MEMORYRULE_H

#include "MemoryRule.h"

struct RTC_Registers
{
    s32 Seconds;
    s32 Minutes;
    s32 Hours;
    s32 Days;
    s32 Control;
    s32 LatchedSeconds;
    s32 LatchedMinutes;
    s32 LatchedHours;
    s32 LatchedDays;
    s32 LatchedControl;
    s32 LastTime;
    s32 padding;
};

class MBC3MemoryRule : public MemoryRule
{
public:
    MBC3MemoryRule(Processor* pProcessor, Memory* pMemory,
            Video* pVideo, Input* pInput, Cartridge* pCartridge, Audio* pAudio);
    virtual ~MBC3MemoryRule();
    virtual u8 PerformRead(u16 address);
    virtual void PerformWrite(u16 address, u8 value);
    virtual void Reset(bool bCGB);
    virtual void SaveRam(std::ofstream &file);
    virtual bool LoadRam(std::ifstream &file, s32 fileSize);
    virtual size_t GetRamSize();
    virtual size_t GetRTCSize();
    virtual u8* GetRamBanks();
    virtual u8* GetCurrentRamBank();
    virtual u8* GetRomBank0();
    virtual u8* GetCurrentRomBank1();
    virtual u8* GetRTCMemory();

private:
    void UpdateRTC();

private:
    int m_iCurrentRAMBank;
    int m_iCurrentROMBank;
    bool m_bRamEnabled;
    bool m_bRTCEnabled;
    u8* m_pRAMBanks;
    s32 m_iRTCLatch;
    u8 m_RTCRegister;
    s32 m_RTCLastTimeCache;
    int m_CurrentROMAddress;
    int m_CurrentRAMAddress;
    RTC_Registers m_RTC;
};

#endif	/* MBC3MEMORYRULE_H */
